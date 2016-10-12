import {Layer, Item, Value} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet::IPv4');
    layer.name = 'IPv4';
    layer.alias = 'ipv4';
    layer.namespace = '::Ethernet::IPv4::<TCP>';
    return [layer];
  }
};
