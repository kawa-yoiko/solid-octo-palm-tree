//const d3 = require('d3-force');
const d3 = require('./node_modules/d3-force/dist/d3-force.js');

const n = 10;
const nodes = [...Array(n).keys()].map((id) => ({
  x: Math.cos(Math.PI * 2 * id / n) * 50,
  y: Math.sin(Math.PI * 2 * id / n) * 50,
  id: id
}));

const edges = [
  [0, 1], [1, 2], [1, 3], [2, 4],
  [3, 5], [4, 1], [4, 7], [5, 2],
  [6, 2], [7, 5], [8, 5], [8, 0],
  [9, 3], [9, 2], [9, 8], [9, 4]
];
const edgesObj = edges.map((e, i) => ({source: e[0], target: e[1], index: i}));

const sim = d3.forceSimulation(nodes)
  .force('link', d3.forceLink(edgesObj))
  .stop();

for (let i = 0; i < 10; i++) {
  console.log(`\n== ${i} ==`);
  sim.tick();
  nodes.map((node) => {
    console.log(`${node.x.toFixed(4)} ${node.y.toFixed(4)} ${node.vx.toFixed(4)} ${node.vy.toFixed(4)}`);
  });
};
