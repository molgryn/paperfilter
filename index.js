const nylonfilter = require('bindings')('nylonfilter');
const EventEmitter = require('events');
const tmp = require('tmp');
const rollup = require('rollup').rollup;
const babel = require('rollup-plugin-babel');

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

    rollup({
      entry: __dirname + '/script/eth.es',
      external: ['dripcap'],
      plugins: [babel()],
      onwarn: (e) => {
        console.log(e)
      }
    }).then((bundle) => {
      const result = bundle.generate({
        format: 'cjs'
      });
      console.log(result);
    });
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
