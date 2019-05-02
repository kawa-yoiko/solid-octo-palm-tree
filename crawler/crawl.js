const https = require('https')

const asyncGet = (url) => new Promise((resolve, reject) => { 
  https.get(url, (res) => {
    const { statusCode } = res;
    const contentType = res.headers['content-type'];

    let isJSON = false;
    if (statusCode !== 200) {
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

(async () => {
  console.log(await asyncTryGet('https://cavestory.fandom.com/index.php?curid=2029&action=raw'));
  console.log(await asyncTryGet('https://cavestory.fandom.com/api.php?format=json&action=query&list=allpages&aplimit=10'));
  console.log(await asyncTryGet('https://cavestory.fandom.com/404'));
})();
