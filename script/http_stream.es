const {Layer, Value, StreamChunk} = require('dripcap');

export default class Dissector {
  constructor() {

  }
  analyze(packet, parentLayer, chunk) {
    let payload = chunk.attr('payload').data;
    let body = payload.toString('utf8');
    let re = /(GET|POST) (\S+) (HTTP\/(0\.9|1\.0|1\.1))\r\n/;
    let m = body.match(re);
    if (m != null) {
      let layer = new Layer(chunk.namespace + '::HTTP');
      layer.name = 'HTTP';
      layer.alias = 'http';
      layer.payload = payload;

      let method = new Value(m[1]);
      let cursor = method.data.length;

      layer.addItem({
        name: 'Method',
        value: method,
        range: '0:' + cursor
      });
      layer.setAttr('method', method);

      let path = new Value(m[2]);
      cursor++;
      layer.addItem({
        name: 'Path',
        value: path,
        range: cursor + ':' + (cursor + path.data.length)
      });
      layer.setAttr('path', path);

      let version = new Value(m[3]);
      cursor += path.data.length + 1;
      layer.addItem({
        name: 'Version',
        value: version,
        range: cursor + ':' + (cursor + version.data.length)
      });

      layer.setAttr('version', version);
      layer.setAttr('src', parentLayer.attr('src'));
      layer.setAttr('dst', parentLayer.attr('dst'));
      return [layer];
    }
  }
};
