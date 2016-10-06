var Application = require('spectron').Application
var assert = require('assert')

describe('application launch', function () {
  this.timeout(10000)

  beforeEach(function () {
    this.app = new Application({
      path: require('electron'),
      args: ['--enable-logging', __dirname + '/../app.js']
    })
    return this.app.start()
  })

  afterEach(function () {
    if (this.app && this.app.isRunning()) {
      if (this.currentTest.state === 'failed') {
        this.app.client.getMainProcessLogs().then((logs) => {
          logs.forEach((log) => {
            console.log(log)
          });
        })
        this.app.client.getRenderProcessLogs().then((logs) => {
          logs.forEach((log) => {
            console.log(log.message);
            console.log(log.source);
          });
        });
      }
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
