import {Layer, Item, Value, StreamChunk} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet::IPv4::TCP');
    layer.name = 'TCP';
    layer.alias = 'tcp';
    let chunk = new StreamChunk('::Ethernet::TCP', 'tcp-' + Math.floor(Math.random() * 5));
    return [layer, chunk];
  }
};
