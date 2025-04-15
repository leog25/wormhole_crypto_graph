import { UnrealBloomPass } from 'https://esm.sh/three/examples/jsm/postprocessing/UnrealBloomPass.js';
import * as d3 from 'https://cdn.skypack.dev/d3';

const WS_URL = window.__ENV__.WS_URL || 'ws://localhost:9001';
const socket = new WebSocket(WS_URL);

socket.onopen = () => {
  console.log("WebSocket connected.");
};

function getRandomColor() {
  const letters = '0123456789ABCDEF';
  let color = '#';
  for (let i = 0; i < 6; i++) {
    color += letters[Math.floor(Math.random() * 16)];
  }
  return color;
}

fetch('data.json')
  .then(response => response.json())
  .then(data => {
    data.nodes.forEach(node => {
      if (node.x !== undefined && node.y !== undefined && node.z !== undefined) {
        const scale = 100; 
        
        node.fx = node.x * scale;
        node.fy = node.y * scale;
        node.fz = node.z * scale;
      }
    });
    
    const gData = {
      nodes: data.nodes,
      links: data.links
    };

    function emitParticleBetweenNodes(startNodeId, endNodeId) {
      const link = gData.links.find(l =>
        (l.source.id === startNodeId && l.target.id === endNodeId)
      );
      
      if (link) {
        console.log('Found link:', link);
        Graph.emitParticle(link);
      } else {
        console.warn(`No link found between nodes ${startNodeId} and ${endNodeId}`);
      }
    }

    const Graph = new ForceGraph3D(document.getElementById('3d-graph'))
      .backgroundColor('#000003')
      .linkDirectionalParticleColor(() => 'yellow')
      .linkDirectionalParticleWidth(4)
      .linkHoverPrecision(10)
      .nodeLabel('id')
      .nodeColor(node => getRandomColor())
      .linkDirectionalParticleSpeed(0.001) 
      .d3Force('link', d3.forceLink()
        .strength(0.1)
      )
      .graphData(gData);

    Graph.onLinkClick(Graph.emitParticle); 

    const bloomPass = new UnrealBloomPass();
    bloomPass.strength = 4;
    bloomPass.radius = 1;
    bloomPass.threshold = 0;
    Graph.postProcessingComposer().addPass(bloomPass);

    Graph.cameraPosition(
      { x: 1000, y: 0, z: -100 }, 
      { x: 0, y: 0, z: -45 },       
      2000                         
    );
    
    const graphScene = Graph.scene();
    graphScene.rotation.x = -1 *Math.PI / 2;
    graphScene.rotation.y = 0;
    graphScene.rotation.z = 0; 

    socket.onmessage = (event) => {
      try {
        const msg = JSON.parse(event.data);
        if (msg.type === 'emitParticle') {
          emitParticleBetweenNodes(msg.startNodeId, msg.endNodeId);
        }
      } catch (err) {
        console.error("Failed to parse WebSocket message", err);
      }
    };
    
    socket.onclose = () => {
      console.log("WebSocket disconnected.");
    };
  })
  .catch(error => console.error('Error loading the JSON data:', error)); 
  