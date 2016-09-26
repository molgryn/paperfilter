var Application = require('spectron').Application
var assert = require('assert')

describe('application launch', function () {
  this.timeout(10000)

  beforeEach(function () {
    this.app = new Application({
      path: require('electron'),
      args: [__dirname + '/../app.js']
    })
    return this.app.start()
  })

  afterEach(function () {
    if (this.app && this.app.isRunning()) {
      return this.app.stop()
    }
  })

  it('shows an initial window', function () {
    return Promise.all([
      this.app.client.waitUntilTextExists('#aa', '::<Ethernet>', 10000),
      this.app.client.waitUntilTextExists('#bb', '::<Ethernet>', 10000),
      this.app.client.waitUntilTextExists('#cc', '::<Ethernet>', 10000)
    ]);
  });
})
