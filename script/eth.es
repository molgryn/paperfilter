import {Layer, Item, Value} from 'dripcap';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet');
    layer.name = 'Ethernet';
    layer.alias = 'eth';

    let destination = new Value(parentLayer.payload.slice(0, 6), 'dripcap/mac');
    layer.addItem(new Item({
      name: 'Destination',
      value: destination,
      range: '0:6'
    }));
    layer.setAttr('dst', destination);

    let source = new Value(parentLayer.payload.slice(6, 12), 'dripcap/mac');
    layer.addItem(new Item({
      name: 'Source',
      value: source,
      range: '6:12'
    }));
    layer.setAttr('src', source);

    let type = parentLayer.payload.readUInt16BE(12);
    if (type <= 1500) {
      layer.addItem(new Item({
        name: 'Length',
        value: new Value(type),
        range: '12:14'
      }));
    } else {
      let table = {
        0x0800: 'IPv4',
        0x0806: 'ARP',
        0x0842: 'WoL',
        0x809B: 'AppleTalk',
        0x80F3: 'AARP',
        0x86DD: 'IPv6'
      };

      let name = table[type] || 'Unknown';
      let val = {};
      val[name] = type;
      let etherType = new Value(val, 'dripcap/enum');
      layer.addItem(new Item({
        name: 'EtherType',
        value: etherType,
        range: '12:14'
      }));
      layer.setAttr('etherType', etherType);

      if (table[type] != null) {
        layer.namespace = `::Ethernet::<${table[type]}>`;
      }
    }

    layer.payload = parentLayer.payload.slice(14);
    layer.addItem(new Item({
      name: 'Payload',
      value: new Value(layer.payload),
      range: '14:'
    }));

    return [layer];
  }
};
