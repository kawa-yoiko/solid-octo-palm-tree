const https = require('https')

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
  return ret;
};

(async () => {
  const pages = await fetchList('allpages', 'aplimit');
  console.log(pages);
  console.log(pages.length);
  const categories = await fetchList('allcategories', 'aclimit');
  console.log(categories);
  console.log(categories.length);
  const images = await fetchList('allimages', 'ailimit');
  console.log(images);
  console.log(images.length);
})();
