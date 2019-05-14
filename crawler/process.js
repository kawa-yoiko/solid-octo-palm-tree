const fs = require('fs');

const dataset = require('./' + process.argv[2] + '-raw.json');
const processResult = {};

processResult.pages = []; // Collection of page IDs
const pageIdOfTitle = {}; // Mapping from page titles to page IDs
const titleOfPageId = {}; // Mapping from page IDs to page titles

for (let i = 0; i < dataset.pages.length; i++) {
  processResult.pages.push(dataset.pages[i].pageid);
  pageIdOfTitle[dataset.pages[i].title] = dataset.pages[i].pageid;
  titleOfPageId[dataset.pages[i].pageid] = dataset.pages[i].title;
}
for (let i = 0; i < dataset.files.length; i++) {
  processResult.pages.push(dataset.files[i].pageid);
  pageIdOfTitle[dataset.files[i].title] = dataset.files[i].pageid;
  titleOfPageId[dataset.files[i].pageid] = dataset.files[i].title;
}

processResult.edges = [];

const parseLink = (title) => {
  let p;
  p = title.indexOf('#');
  if (p === 0) return -1;
  if (p !== -1) title = title.substr(0, p);
  p = title.indexOf('|');
  if (p !== -1) title = title.substr(0, p);
  let id = pageIdOfTitle[title];
  if (id !== undefined) return id;
  title = title[0].toUpperCase() + title.substr(1);
  id = pageIdOfTitle[title];
  if (id !== undefined) return id;
  title = title.replace(/_/g, ' ');
  id = pageIdOfTitle[title];
  if (id !== undefined) return id;
  return -1;
};

for (let i = 0; i < dataset.pages.length; i++) {
  const curPageId = dataset.pages[i].pageid;
  const content = dataset.pageContents[curPageId];
  let curly = 0;
  let bracket = -1;
  for (let j = 0; j < content.length; j++) {
    const t = content.substr(j, 2);
    if (t === '{{') {
      curly += 1;
    } else if (t === '}}') {
      curly -= 1;
    } else if (t === '[[') {
      bracket = j + 2;
    } else if (t === ']]') {
      if (bracket != -1) {
        const title = content.substring(bracket, j);
        const pageId = parseLink(title);
        if (pageId === -1) {
          if (!title.startsWith('Category:'))
            console.log(pageId, title);
        } else {
          processResult.edges.push([curPageId, pageId]);
        }
      }
      bracket = -1;
    }
  }
}

// processResult = { pages: [id], edges: [[id, id]] }

// processResultText =
// <N -- number of pages> <M -- number of edges>
// <title of page 0>
// <title of page 1>
// ...
// <title of page N>
// <index of first node> <index of second node>
// <index of first node> <index of second node>
// ...
// <index of first node> <index of second node>

let processResultText = '';

processResultText += processResult.pages.length.toString() + ' ' +
  processResult.edges.length.toString() + '\n';

const indexOfPageId = {};

for (let i = 0; i < processResult.pages.length; i++) {
  const pageId = processResult.pages[i];
  indexOfPageId[pageId] = i;
  processResultText += titleOfPageId[pageId] + '\n';
}

for (let i = 0; i < processResult.edges.length; i++) {
  const u = indexOfPageId[processResult.edges[i][0]];
  const v = indexOfPageId[processResult.edges[i][1]];
  processResultText += u.toString() + ' ' + v.toString() + '\n';
}

// Write to file

const filePath = process.argv[2] + '-processed.txt';

fs.writeFile(filePath, processResultText, (err) => {
  if (err) {
    console.log(`Error writing to file ${filePath}: ${err.message}`);
    console.log('Dumping dataset in the console. See below');
    console.log(processResultText);
  } else {
    console.log(`Dataset written to file ${filePath}`);
  }
});
