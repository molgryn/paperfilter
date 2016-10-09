import {Layer, StreamChunk, Item, Value, LargeBuffer} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet');
    layer.summary = '' + Math.random();
    let item = new Item();
    item.name = "aaa";
    item.value = new Value(new LargeBuffer());
    let item2 = new Item();
    item2.name = "aaabbb";
    item2.value = new Value(false);
    item.addChild(item2);
    layer.addItem(item);
    layer.setAttr("src", item2);
    return [layer];
  }
};
