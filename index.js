const nylonfilter = require('bindings')('nylonfilter');
const EventEmitter = require('events');
const tmp = require('tmp');

class Session extends EventEmitter {
  constructor(option) {
    super();
    option.tmp = tmp.dirSync().name;
    this.sess = new nylonfilter.Session(option);
    this.sess.errorCallback = (err) => {
      this.emit('error', err);
    };
    this.sess.packetCallback = (stat) => {
      this.emit('packets', stat);
    };
    this.sess.filterCallback = (name, packets) => {
      this.emit('filter', name, packets);
    };
  }

  analyze(pkt) {
    return this.sess.analyze(pkt);
  }

  filter(name, filter) {
    return this.sess.filter(name, filter);
  }

  get(seq) {
    return this.sess.get(seq);
  }

  get namespace() {
    return this.sess.namespace;
  }
}

module.exports = {
  Session: Session
}
