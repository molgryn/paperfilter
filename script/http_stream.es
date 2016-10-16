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
      layer.setAttr('method', new Value(m[1]));
      layer.setAttr('path', new Value(m[2]));
      layer.setAttr('version', new Value(m[3]));
      layer.setAttr('src', parentLayer.attr('src'));
      layer.setAttr('dst', parentLayer.attr('dst'));
      return [layer];
    }
  }
};
