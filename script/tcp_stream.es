const {Layer, StreamChunk} = require('dripcap');

export default class Dissector {
  constructor() {
    this.seq = -1;
    this.length = 0;
  }
  analyze(packet, parentLayer, chunk) {

    if (parentLayer.payload.length > 0) {
      let ns = chunk.namespace.replace('<TCP>', 'TCP');
      let stream = new StreamChunk(ns, chunk.id, parentLayer);

      /*
      if (this.seq < 0) {
        this.length += layer.payload.length;
        stream.data = layer.payload;
      } else {
        let start = this.seq + this.length;
        let length = layer.payload.length;
        if (start > layer.attrs.seq) {
          length -= (start - layer.attrs.seq);
        }
        this.length += length;
        stream.data = layer.payload;
      }
      this.seq = layer.attrs.seq;
      output.push(stream);
      */
    }

  }
};
