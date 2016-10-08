const {Layer, StreamChunk, VirtualPacket} = require('dripcap');

export default class Dissector {
  constructor() {
    this.basePackets = [];
  }
  analyze(packet, parentLayer, chunk) {
    this.basePackets.push(packet.seq);
    if (this.basePackets.length > 3) {
      let vp = new VirtualPacket('::Ethernet::TCP::HTTP');
      vp.payload = packet.payload;
      let pkt = new StreamChunk('::Ethernet::TCP::XXX', 'tgrtdd' + Math.floor(Math.random() * 5));
      return [vp, pkt];
    }
  }
};
