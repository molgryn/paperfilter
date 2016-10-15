import {Layer, Item, Value} from 'dripcap';
import {Flags} from 'dripcap/utils';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet::IPv4');
    layer.name = 'IPv4';
    layer.alias = 'ipv4';

    let version = new Value(parentLayer.payload.readUInt8(0, true) >> 4);
    layer.addItem(new Item({
      name: 'Version',
      value: version,
      range: '0:1'
    }));
    layer.setAttr('version', version);

    let headerLength = new Value(parentLayer.payload.readUInt8(0, true) & 0b00001111);
    layer.addItem(new Item({
      name: 'Internet Header Length',
      value: headerLength,
      range: '0:1'
    }));
    layer.setAttr('headerLength', headerLength);

    let type = new Value(parentLayer.payload.readUInt8(1, true));
    layer.addItem(new Item({
      name: 'Type of service',
      value: type,
      range: '1:2'
    }));
    layer.setAttr('type', type);

    let totalLength = new Value(parentLayer.payload.readUInt16BE(2, true));
    layer.addItem(new Item({
      name: 'Total Length',
      value: totalLength,
      range: '2:4'
    }));
    layer.setAttr('totalLength', totalLength);

    let id = new Value(parentLayer.payload.readUInt16BE(4, true));
    layer.addItem(new Item({
      name: 'Identification',
      value: id,
      range: '4:6'
    }));
    layer.setAttr('id', id);

    let table = {
      'Reserved': 0x1,
      'Don\'t Fragment': 0x2,
      'More Fragments': 0x4,
    };

    let flags = Flags(table, (parentLayer.payload.readUInt8(6, true) >> 5) & 0x7);

    layer.addItem(new Item({
      name: 'Flags',
      value: flags,
      range: '6:7',
      children: [
        new Item({
          name: 'Reserved',
          value: new Value(flags.data['Reserved']),
          range: '6:7'
        }),
        new Item({
          name: 'Don\'t Fragment',
          value: new Value(flags.data['Don\'t Fragment']),
          range: '6:7'
        }),
        new Item({
          name: 'More Fragments',
          value: new Value(flags.data['More Fragments']),
          range: '6:7'
        })
      ]
    }));

    layer.namespace = '::Ethernet::IPv4::<TCP>';
    return [layer];
  }
};

/*
import {
  Layer,
  Buffer
} from 'dripcap';
import MACAddress from 'dripcap/mac';
import ProtocolEnum from 'dripcap/ipv4/protocol';
import IPv4Address from 'dripcap/ipv4/addr';
import FieldFlags from 'dripcap/ipv4/fields';

export default class IPv4Dissector {
  constructor(options) {}

  analyze(packet, parentLayer) {
    function assertLength(len) {
      if (parentLayer.payload.length < len) {
        throw new Error('too short frame');
      }
    }

    let layer = new Layer();
    layer.name = 'IPv4';
    layer.namespace = '::Ethernet::IPv4';

    try {

      assertLength(7);
      let flags = new FieldFlags((parentLayer.payload.readUInt8(6, true) >> 5) & 0x7);
      layer.fields.push({
        name: 'Flags',
        attr: 'flags',
        data: parentLayer.payload.slice(6, 7),
        fields: [{
          name: 'Reserved',
          value: flags.get('Reserved'),
          data: parentLayer.payload.slice(6, 7),
        }, {
          name: 'Don\'t Fragment',
          value: flags.get('Don\'t Fragment'),
          data: parentLayer.payload.slice(6, 7),
        }, {
          name: 'More Fragments',
          value: flags.get('More Fragments'),
          data: parentLayer.payload.slice(6, 7),
        }]
      });
      layer.attrs.flags = flags;

      layer.fields.push({
        name: 'Fragment Offset',
        attr: 'fragmentOffset',
        data: parentLayer.payload.slice(6, 8),
      });
      layer.attrs.fragmentOffset = parentLayer.payload.readUInt8(6, true) & 0b0001111111111111;

      assertLength(9);
      layer.fields.push({
        name: 'TTL',
        attr: 'ttl',
        data: parentLayer.payload.slice(8, 9),
      });
      layer.attrs.ttl = parentLayer.payload.readUInt8(8, true);

      assertLength(10);
      let protocol = new ProtocolEnum(parentLayer.payload.readUInt8(9, true));

      layer.fields.push({
        name: 'Protocol',
        attr: 'protocol',
        data: parentLayer.payload.slice(9, 10),
      });
      layer.attrs.protocol = protocol;

      if (protocol.known) {
        layer.namespace = `::Ethernet::IPv4::<${protocol.name}>`;
      }

      assertLength(12);
      layer.fields.push({
        name: 'Header Checksum',
        attr: 'checksum',
        data: parentLayer.payload.slice(10, 12),
      });
      layer.attrs.checksum = parentLayer.payload.readUInt16BE(10, true);

      assertLength(16);
      let source = new IPv4Address(parentLayer.payload.slice(12, 16));
      layer.fields.push({
        name: 'Source IP Address',
        attr: 'src',
        data: parentLayer.payload.slice(12, 16),
      });
      layer.attrs.src = source;

      assertLength(20);
      let destination = new IPv4Address(parentLayer.payload.slice(16, 20));
      layer.fields.push({
        name: 'Destination IP Address',
        attr: 'dst',
        data: parentLayer.payload.slice(16, 20),
      });
      layer.attrs.dst = destination;

      assertLength(totalLength);
      layer.payload = parentLayer.payload.slice(20, totalLength);

      layer.fields.push({
        name: 'Payload',
        value: layer.payload,
        data: layer.payload
      });

      layer.summary = `${source} -> ${destination}`;
      if (protocol.known) {
        layer.summary = `[${protocol.name}] ` + layer.summary;
      }

    } catch (err) {
      layer.error = err.message;
    }

    parentLayer.layers[layer.namespace] = layer;
  }
}

*/
