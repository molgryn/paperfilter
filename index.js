const paperfilter = require('bindings')('paperfilter');
const EventEmitter = require('events');
const rollup = require('rollup').rollup;
const babel = require('rollup-plugin-babel');
const esprima = require('esprima');

class Session extends EventEmitter {
  constructor(option) {
    super();

    let module = {};
    (new Function('module', option.filterScript))(module);
    this._filter = module.exports;

    this._sess = new paperfilter.Session(option);
    this._sess.errorCallback = (err) => {
      this.emit('error', new Error(err));
    };
    this._sess.packetCallback = (stat) => {
      this.emit('packets', stat);
    };
    this._sess.filterCallback = (name, packets) => {
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
    return this._sess.analyze(pkt);
  }

  filter(name, filter) {
    let body = '';
    const ast = esprima.parse(filter);
    switch (ast.body.length) {
      case 0:
        break;
      case 1:
        const root = ast.body[0];
        if (root.type !== "ExpressionStatement")
          throw new SyntaxError();
        this._filter(root.expression);
        body = JSON.stringify(root.expression);
        break;
      default:
        throw new SyntaxError();
    }
    return this._sess.filter(name, body);
  }

  get(seq) {
    return this._sess.get(seq);
  }

  get namespace() {
    return this._sess.namespace;
  }

  get permission() {
    return this._sess.permission;
  }

  get devices() {
    return this._sess.devices;
  }

  get interface() {
    return this._sess.interface;
  }

  set interface(ifs) {
    this._sess.interface = ifs;
  }

  get promiscuous() {
    return this._sess.promiscuous;
  }

  set promiscuous(promisc) {
    this._sess.promiscuous = promisc;
  }

  get snaplen() {
    return this._sess.snaplen;
  }

  set snaplen(len) {
    this._sess.snaplen = len;
  }

  setBPF(bpf) {
    this._sess.setBPF(bpf);
  }

  start() {
    this._sess.start();
  }

  stop() {
    this._sess.stop();
  }
}

module.exports = {
  Session: Session
}
