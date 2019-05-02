const https = require('https');
const fs = require('fs');

const asyncGet = (url) => new Promise((resolve, reject) => { 
  https.get(url, (res) => {
    const { statusCode } = res;
    const contentType = res.headers['content-type'];

    let isJSON = false;
    if (statusCode === 302) {
      const redirect = asyncGet(res.headers['location']);
      return redirect.then(resolve).catch(reject);
    } else if (statusCode !== 200) {
      return reject(new Error(`Unexpected status code: ${statusCode}`));
    } else if (/^application\/json/.test(contentType)) {
      isJSON = true;
    }

    res.setEncoding('utf8');
    let rawData = '';
    res.on('data', chunk => rawData += chunk);
    res.on('end', () => {
      try {
        resolve(isJSON ? JSON.parse(rawData) : rawData);
      } catch (err) {
        reject(err);
      }
    });
  });
});

const asyncTryGet = async (url, tries) => {
  tries = tries || 3;
  let result = null;
  for (let i = 0; i < tries; i++) {
    try {
      result = await asyncGet(url);
    } catch (err) {
      console.log(`${err.message} (${url})`);
    }
    if (result) return result;
  }
  return null;
};

const fetchList = async (list, limitArg) => {
  console.log(`Fetching list ${list}`);
  const limit = 500;
  const ret = [];
  let continueParam = '';
  while (1) {
    const response = await asyncTryGet(`https://cavestory.fandom.com/api.php?format=json&action=query&list=${list}&${limitArg}=${limit}${continueParam}`);

    const queryList = response['query'][list];
    for (let i = 0; i < queryList.length; i++) ret.push(queryList[i]);

    const queryContinue = response['query-continue'];
    const continueObj = queryContinue ? queryContinue[list] : null;
    if (!continueObj) break;
    continueParam = '&';
    for (let i in continueObj) continueParam += `${i}=${continueObj[i]}`;
  }
  console.log(`Fetched list ${list} of ${ret.length} element(s)`);
  return ret;
};

let pagesFetchedCount = 0;
let pagesToFetchTotal = 0;

const fetchPage = async (pageId) => {
  const ret = await asyncTryGet(`https://cavestory.fandom.com/index.php?curid=${pageId}&action=raw`);
  console.log(`Fetched page ${pageId} (${++pagesFetchedCount} / ${pagesToFetchTotal})`);
  return ret;
};

(async () => {
  const pages = await fetchList('allpages', 'aplimit');
  const categories = await fetchList('allcategories', 'aclimit');
  const images = await fetchList('allimages', 'ailimit');

  const pageContents = {};
  const promises = [];
  pagesToFetchTotal = pages.length;
  for (let i = 0; i < pages.length; ++i)
    promises.push(fetchPage(pages[i].pageid));
  const promiseOfAll = Promise.all(promises);
  const pageContentsByIndex = await promiseOfAll;
  for (let i = 0; i < pageContentsByIndex.length; ++i)
    pageContents[pages[i].pageid] = pageContentsByIndex[i];

  const pageContentsJSON = JSON.stringify({
    pages: pages,
    categories: categories,
    images: images,
    pageContents: pageContents
  });
  const filePath = 'dataset.json';
  fs.writeFile(filePath, pageContentsJSON, (err) => {
    if (err) {
      console.log(`Error writing to file ${filePath}: ${err.message}`);
      console.log('Dumping dataset in the console. See below');
      console.log(pageContentsJSON);
    } else {
      console.log(`Dataset written to file ${filePath}`);
    }
  });
})();
