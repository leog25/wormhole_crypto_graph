#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <mutex>
#include <uwebsockets/App.h>


using json = nlohmann::json;

const std::unordered_map<int, std::string> chainIDToName = {
    {2, "ethereum"}, 
    {1, "solana"},
    {12, "acala"},
    {8, "algorand"},
    {22, "aptos"},
    {23, "arbitrum"},
    {6, "avalanche"},
    {30, "base"},
    {39, "berachain"},
    {4, "bnb smart chain"},
    {4004, "celestia"},
    {14, "celo"},
    {10, "fantom"},
    {11, "karura"},
    {16, "moonbeam"},
    {15, "near"},
    {7, "oasis"},
    {24, "optimism"},
    {5, "polygon"},
    {34, "scroll"},
    {32, "sei"},
    {21, "sui"},
    {3, "terra"},
};

std::string getChainName(int id) {
    auto it = chainIDToName.find(id);
    if (it != chainIDToName.end()) {
        return it->second;
    } else {
        return "Chain-" + std::to_string(id);
    }
}

struct ChainPair {
    std::string sourceChain;
    std::string destChain;
    std::string timestamp;
    
    std::string toString() const {
        return "Source: " + sourceChain + ", Destination: " + destChain + ", Time: " + timestamp;
    }
};

struct WebSocketData {

};

std::set<uWS::WebSocket<false, true, WebSocketData>*> clients;
std::mutex clientsMutex;


int main() {
    std::unordered_map<std::string, ChainPair> transactions;
    
    std::cout << "Fetching Wormhole transaction data every second...\n";

    std::thread([&]() {

    std::vector<std::string> prev_txs;
    while (true) {
        try {
            cpr::Response r = cpr::Get(cpr::Url{"https://api.wormholescan.io/api/v1/transactions?&pageSize=5"});
            
            if (r.status_code == 200) {
                json response = json::parse(r.text);
                
                for (const auto& tx : response["transactions"]) {
                    std::string txId = tx["id"];
                    int sourceChainId = -1;
                    int destChainId = -1;
                    
                    if (tx.contains("emitterChain")) {
                        sourceChainId = tx["emitterChain"];
                    } else if (tx.contains("standardizedProperties") && 
                               tx["standardizedProperties"].contains("fromChain")) {
                        sourceChainId = tx["standardizedProperties"]["fromChain"];
                    }
                    
                    if (tx.contains("standardizedProperties") && 
                        tx["standardizedProperties"].contains("toChain") && 
                        tx["standardizedProperties"]["toChain"] != 0) {
                        destChainId = tx["standardizedProperties"]["toChain"];
                    } else if (tx.contains("globalTx") && 
                               tx["globalTx"].contains("destinationTx") && 
                               tx["globalTx"]["destinationTx"] != nullptr && 
                               tx["globalTx"]["destinationTx"].contains("chainId")) {
                        destChainId = tx["globalTx"]["destinationTx"]["chainId"];
                    } else if (tx.contains("payload") && 
                               tx["payload"].contains("targetChainId")) {
                        destChainId = tx["payload"]["targetChainId"];
                    }
                    
                    if (sourceChainId != -1 && destChainId != -1 && 
                        chainIDToName.find(sourceChainId) != chainIDToName.end() && 
                        chainIDToName.find(destChainId) != chainIDToName.end() && 
                        std::find(prev_txs.begin(), prev_txs.end(), txId) == prev_txs.end()) {
                        
                        std::string timestamp = "";
                        if (tx.contains("timestamp")) {
                            timestamp = tx["timestamp"];
                        }
                        
                        ChainPair chains{
                            getChainName(sourceChainId),
                            getChainName(destChainId),
                            timestamp
                        };
                        
                        transactions[txId] = chains;

                        std::cout << chains.sourceChain << " -> " << chains.destChain << " at " << chains.timestamp << "\n";

                        std::string message = R"({"type":"emitParticle","startNodeId":")" +
                                            chains.sourceChain + R"(","endNodeId":")" +
                                            chains.destChain + R"("})";

                        {
                            std::lock_guard<std::mutex> lock(clientsMutex);
                            for (auto* ws : clients) {
                                ws->send(message, uWS::OpCode::TEXT);
                            }
                        }

                        prev_txs.push_back(txId);

                    }
                }
                

                if (transactions.empty()) {
                    std::cout << "No valid transactions found yet.\n";
                } else {
                    std::vector<std::pair<std::string, std::string>> sortedTxs;
                    for (const auto& [id, chains] : transactions) {
                        sortedTxs.push_back({chains.timestamp, id});
                    }
                    
                    std::sort(sortedTxs.begin(), sortedTxs.end());

                }
            } else {
                std::cerr << "Error: API request failed with status code " << r.status_code << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    }).detach();

    uWS::App().ws<WebSocketData>("/*", {
        .open = [&](auto* ws) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.insert(ws);
            std::cout << "Client connected.\n";
        },
        .message = [](auto* ws, std::string_view msg, uWS::OpCode) {
            std::cout << "Received: " << msg << "\n";
        },
        .close = [&](auto* ws, int, std::string_view) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.erase(ws);
            std::cout << "Client disconnected.\n";
        }
    }).listen(9001, [](auto* token) {
        if (token) {
            std::cout << "WebSocket server listening on port 9001\n";
        } else {
            std::cerr << "Failed to listen on port 9001\n";
        }
    }).run();

    
    return 0;
}


