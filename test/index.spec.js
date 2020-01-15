'use strict';

const jsonRedactor = require('../');

describe("index.js tests", () => {

  it('rejects on bad input', () => {
    expect(() => jsonRedactor())
      .toThrow('Expected an array of property names to redact and a JSON string.');
  });

  it('redacts a single key', () => {
    const json = JSON.stringify({name: 'Test'});
    const redacted = jsonRedactor(['name'], json);
    expect(redacted).toBe('{"name":"****"}');
  });

  it('leaves non-redacted keys alone', () => {
    const json = JSON.stringify({name: 'Test'});
    const redacted = jsonRedactor(['test'], json);
    expect(redacted).toBe('{"name":"Test"}');
  });

  it('redacts nested keys', () => {
    const input = {
      name: 'Test0',
      child: {
        name: 'Test1',
        child: {
          name: 'Test2',
          child: {
            name: 'Test3',
          },
        },
      },
    };
    const json = JSON.stringify(input);
    const redacted = jsonRedactor(['name'], json);
    expect(redacted).toBe('{"name":"*****","child":{"name":"*****","child":{"name":"*****","child":{"name":"*****"}}}}');
  });

  it('redacts multiple nested keys', () => {
    const input = {
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
    };
    const json = JSON.stringify(input);
    const redacted = jsonRedactor(['name', 'secret'], json);
    expect(redacted).toBe('{"name":"*****","child":{"name":"*****","child":{"name":"*****","child":{"name":"*****","secret":"****************"}}}}');
  });
});
