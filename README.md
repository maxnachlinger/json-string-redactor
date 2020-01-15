## json-string-redactor

> Redacts JSON string values for specified keys 

```
npm i json-string-redactor
```

### Example
```javascript
const jsonRedactor = require('json-string-redactor');

const input = JSON.stringify({
  name: 'Test0',
  child: {
    name: 'Test1',
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
  "child": {
    "name": "*****",
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
