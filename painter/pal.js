// https://github.com/ant-design/ant-design/tree/master/components/style/color
const fs = require('fs');

const s = fs.readFileSync('pal.css').toString();
const r = /palette-(.+?)\{background:#(.+?)\}/g;

console.log(
`#ifndef _PAL_H_
#define _PAL_H_

#define OuO(_r, _g, _b) ((Color){0x##_r, 0x##_g, 0x##_b, 255})
`);

for (let m = r.exec(s); m !== null; m = r.exec(s)) {
  const name = m[1].toUpperCase().replace('-', '_');
  let val = m[2];
  if (val.length === 3) {
    val = [0, 1, 2].map((i) => val[i].repeat(2)).join('');
  }
  console.log('#define ' + name + ' '.repeat(12 - name.length) +
    `OuO(${val.substr(0, 2)}, ${val.substr(2, 2)}, ${val.substr(4, 2)})`);
}

console.log(`
#endif`);
