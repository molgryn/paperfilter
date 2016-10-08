import {Layer, StreamChunk, Item} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet');
    layer.summary = '' + Math.random();
    let item = new Item();
    item.name = "aaa";
    layer.addItem(item);
    return [layer];
  }
};
