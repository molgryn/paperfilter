import {Layer, StreamChunk} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet');
    layer.summary = '' + Math.random();
    let pkt = new StreamChunk('::Ethernet::TCP', 'tgrtdd' + Math.floor(Math.random() * 5));
    return [layer, pkt];
  }
};
