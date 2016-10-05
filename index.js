const nylonfilter = require('bindings')('nylonfilter');
const EventEmitter = require('events');
const tmp = require('tmp');
const rollup = require('rollup').rollup;
const babel = require('rollup-plugin-babel');

class Session extends EventEmitter {
  constructor(option) {
    super();
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

  static create(option) {
    let sessOption = {
      tmp: tmp.dirSync().name,
      namespace: option.namespace,
      dissectors: [],
      stream_dissectors: []
    };

    let tasks = [];
    for (let diss of option.dissectors) {
      tasks.push(rollup({
        entry: diss.script,
        external: ['dripcap'],
        plugins: [babel()],
        onwarn: (e) => {
          console.log(e)
        }
      }).then((bundle) => {
        const result = bundle.generate({
          format: 'cjs'
        });
        return result.code;
      }).then((code) => {
        sessOption.dissectors.push({
          namespaces: diss.namespaces,
          script: code
        });
      }));
    }
    for (let diss of option.stream_dissectors) {
      tasks.push(rollup({
        entry: diss.script,
        external: ['dripcap'],
        plugins: [babel()],
        onwarn: (e) => {
          console.log(e)
        }
      }).then((bundle) => {
        const result = bundle.generate({
          format: 'cjs'
        });
        return result.code;
      }).then((code) => {
        sessOption.stream_dissectors.push({
          namespaces: diss.namespaces,
          script: code
        });
      }));
    }
    return Promise.all(tasks).then(() => {
      return new Session(sessOption);
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
