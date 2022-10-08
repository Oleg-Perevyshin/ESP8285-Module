const fetchJSON = (...arg) => fetch(...arg).then(response => response.ok ? response.json() : Promise.reject(response));
const fetchText = (...arg) => fetch(...arg).then(response => response.ok ? response.text() : Promise.reject(response));

const translate = (dictionary, key) => {
  let val = null;
  try {
    val = key.split(".").reduce((a, b) => a[isNaN(b) ? b : +b], dictionary);
  } catch (e) {
  }
  if (typeof val !== "string")
    val = key;
  return val;
};

export {fetchJSON, fetchText, translate};
