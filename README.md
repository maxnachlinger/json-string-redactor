## json-string-redactor

> Redacts JSON string values for specified keys 

[![travis][travis-image]][travis-url]

[travis-image]: https://travis-ci.org/maxnachlinger/json-string-redactor.svg?branch=master
[travis-url]: https://travis-ci.org/maxnachlinger/json-string-redactor

```
npm i json-string-redactor
```

### Example
```javascript
const jsonRedactor = require('json-string-redactor');

const input = JSON.stringify({
  name: 'Test0',
  prop0: 'I should not be redacted',
  child: {
    name: 'Test1',
    prop1: 'I should not be redacted either',
    child: {
      name: 'Test2',
      child: {
        name: 'Test3',
        secret: 'This is a secret',
      },
    },
  },
});

const redacted = jsonRedactor(['name', 'secret'], input);
/*
redacted is now: 
{
  "name": "*****",
  "prop0": "I should not be redacted",
  "child": {
    "name": "*****",
    "prop1": "I should not be redacted either",
    "child": {
      "name": "*****",
      "child": {
        "name": "*****",
        "secret": "****************"
      }
    }
  }
}
*/
```
Note: You get 1 `*` per character redacted :)
