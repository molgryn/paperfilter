import {Layer, Item, Value} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    /*
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
    layer.setAttr("src", new Value(false));
    return [layer];
    */
    let layer = new Layer('::Ethernet');
    layer.name = 'Ethernet';

    let destination = parentLayer.payload.slice(0, 6);
    layer.addItem(new Item({
      name: 'Destination',
      attr: 'dst',
      range: '0:6'
    }));
    layer.setAttr('dst', new Value(destination, 'dripcap/mac'));

    let source = parentLayer.payload.slice(6, 12);
    layer.addItem(new Item({
      name: 'Source',
      attr: 'src',
      range: '6:12'
    }));
    layer.setAttr('src', new Value(source, 'dripcap/mac'));

    layer.payload = parentLayer.payload.slice(14);
    layer.addItem(new Item({
      name: 'Payload',
      value: new Value(layer.payload),
      range: '14:'
    }));

    return [layer];
  }
};
