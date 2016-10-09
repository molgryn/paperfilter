const paperfilter = require('bindings')('paperfilter');
const EventEmitter = require('events');
const rollup = require('rollup').rollup;
const babel = require('rollup-plugin-babel');
const esprima = require('esprima');

class Session extends EventEmitter {
  constructor(option) {
    super();
    this.sess = new paperfilter.Session(option);
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
    tasks.push(rollup({
      entry: __dirname + '/filter.es',
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
      sessOption.filterScript = code;
    }));
    return Promise.all(tasks).then(() => {
      return new Session(sessOption);
    });
  }

  analyze(pkt) {
    return this.sess.analyze(pkt);
  }

  filter(name, filter) {
    console.log(esprima.parse(filter).body)
    return this.sess.filter(name, filter);
  }

  get(seq) {
    return this.sess.get(seq);
  }

  get namespace() {
    return this.sess.namespace;
  }

  get permission() {
    return this.sess.permission;
  }

  get devices() {
    return this.sess.devices;
  }

  get interface() {
    return this.sess.interface;
  }

  set interface(ifs) {
    this.sess.interface = ifs;
  }

  get promiscuous() {
    return this.sess.promiscuous;
  }

  set promiscuous(promisc) {
    this.sess.promiscuous = promisc;
  }

  get snaplen() {
    return this.sess.snaplen;
  }

  set snaplen(len) {
    this.sess.snaplen = len;
  }

  setBPF(bpf) {
    this.sess.setBPF(bpf);
  }

  start() {
    this.sess.start();
  }

  stop() {
    this.sess.stop();
  }
}

module.exports = {
  Session: Session
}
