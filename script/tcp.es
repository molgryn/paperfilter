import {Layer, Item, Value, StreamChunk} from 'dripcap';
import {Flags, Enum} from 'dripcap/utils';
import {IPv4Host} from 'dripcap/ipv4';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer(parentLayer.namespace.replace('<TCP>', 'TCP'));
    layer.name = 'TCP';
    layer.alias = 'tcp';

    let source = parentLayer.payload.readUInt16BE(0);
    layer.addItem({
      name: 'Source port',
      value: source,
      range: '0:2'
    });

    let destination = parentLayer.payload.readUInt16BE(2);
    layer.addItem({
      name: 'Destination port',
      value: destination,
      range: '2:4'
    });

    let srcAddr = parentLayer.attr('src');
    let dstAddr = parentLayer.attr('dst');
    if (srcAddr.type === 'dripcap/ipv4/addr') {
      layer.setAttr('src', IPv4Host(srcAddr.data, source));
      layer.setAttr('dst', IPv4Host(dstAddr.data, destination));
    }

    let seq = new Value(parentLayer.payload.readUInt32BE(4));
    layer.addItem({
      name: 'Sequence number',
      value: seq,
      range: '4:8'
    });
    layer.setAttr('seq', seq);

    let ack = new Value(parentLayer.payload.readUInt32BE(8));
    layer.addItem({
      name: 'Acknowledgment number',
      value: ack,
      range: '8:12'
    });
    layer.setAttr('ack', ack);

    let dataOffset = new Value(parentLayer.payload.readUInt8(12) >> 4);
    layer.addItem({
      name: 'Data offset',
      value: dataOffset,
      range: '12:13'
    });
    layer.setAttr('dataOffset', dataOffset);

    let table = {
      'NS': 0x1 << 8,
      'CWR': 0x1 << 7,
      'ECE': 0x1 << 6,
      'URG': 0x1 << 5,
      'ACK': 0x1 << 4,
      'PSH': 0x1 << 3,
      'RST': 0x1 << 2,
      'SYN': 0x1 << 1,
      'FIN': 0x1 << 0,
    };

    let flags = Flags(table, parentLayer.payload.readUInt8(13) |
      ((parentLayer.payload.readUInt8(12) & 0x1) << 8));

    layer.addItem({
      name: 'Flags',
      value: flags,
      data: '12:14',
      children: [
        {
          name: 'NS',
          value: new Value(flags.data['NS']),
          range: '12:13'
        },
        {
          name: 'CWR',
          value: new Value(flags.data['CWR']),
          range: '13:14'
        },
        {
          name: 'ECE',
          value: new Value(flags.data['ECE']),
          range: '13:14'
        },
        {
          name: 'URG',
          value: new Value(flags.data['URG']),
          range: '13:14'
        },
        {
          name: 'ACK',
          value: new Value(flags.data['ACK']),
          range: '13:14'
        },
        {
          name: 'PSH',
          value: new Value(flags.data['PSH']),
          range: '13:14'
        },
        {
          name: 'RST',
          value: new Value(flags.data['RST']),
          range: '13:14'
        },
        {
          name: 'SYN',
          value: new Value(flags.data['SYN']),
          range: '13:14'
        },
        {
          name: 'FIN',
          value: new Value(flags.data['FIN']),
          range: '13:14'
        }
      ]
    });

    let window = new Value(parentLayer.payload.readUInt16BE(14));
    layer.addItem({
      name: 'Window size',
      value: window,
      range: '14:16'
    });
    layer.setAttr('window', window);

    let checksum = new Value(parentLayer.payload.readUInt16BE(16));
    layer.addItem({
      name: 'Checksum',
      value: checksum,
      range: '16:18'
    });
    layer.setAttr('checksum', checksum);

    let urgent = new Value(parentLayer.payload.readUInt16BE(18));
    layer.addItem({
      name: 'Urgent pointer',
      value: urgent,
      range: '18:20'
    })
    layer.setAttr('urgent', urgent);

    let optionDataOffset = dataOffset.data * 4;
    let optionItems = [];
    let option = {
      name: 'Options',
      range: '20:' + optionDataOffset,
      children: []
    };

    let optionOffset = 20;

    while (optionDataOffset > optionOffset) {
      switch (parentLayer.payload[optionOffset]) {
        case 0:
          optionOffset = optionDataOffset;
          break;

        case 1:
          option.children.push({
            name: 'NOP',
            range: `${optionOffset}:${optionOffset + 1}`
          });
          optionOffset++;
          break;

        case 2:
          optionItems.push('Maximum segment size');
          option.children.push({
            name: 'Maximum segment size',
            value: new Value(parentLayer.payload.readUInt16BE(optionOffset + 2)),
            range: `${optionOffset}:${optionOffset + 4}`
          });
          optionOffset += 4;
          break;

        case 3:
          optionItems.push('Window scale');
          option.children.push({
            name: 'Window scale',
            value: new Value(parentLayer.payload.readUInt8(optionOffset + 2)),
            range: `${optionOffset}:${optionOffset + 3}`
          });
          optionOffset += 3;
          break;

        case 4:
          optionItems.push('Selective ACK permitted');
          option.children.push({
            name: 'Selective ACK permitted',
            range: `${optionOffset}:${optionOffset + 2}`
          });
          optionOffset += 2;
          break;

        // TODO: https://tools.ietf.org/html/rfc2018
        case 5:
          let length = parentLayer.payload.readUInt8(optionOffset + 1);
          optionItems.push('Selective ACK');
          option.children.push({
            name: 'Selective ACK',
            value: new Value(parentLayer.payload.slice(optionOffset + 2, optionOffset + length)),
            data: `${optionOffset}:${optionOffset + length}`
          });

          optionOffset += length;
          break;

        case 8:
          let mt = parentLayer.payload.readUInt32BE(optionOffset + 2);
          let et = parentLayer.payload.readUInt32BE(optionOffset + 2);
          optionItems.push('Timestamps');
          option.children.push({
            name: 'Timestamps',
            value: new Value(`${mt} - ${et}`),
            range: `${optionOffset}:${optionOffset + 10}`,
            children: [{
              name: 'My timestamp',
              value: mt,
              range: `${optionOffset + 2}:${optionOffset + 6}`
            }, {
              name: 'Echo reply timestamp',
              value: et,
              range: `${optionOffset + 6}:${optionOffset + 10}`
            }]
          });
          optionOffset += 10;
          break;

        default:
          throw new Error('unknown option');
      }
    }

    option.value = new Value(optionItems.join(','));
    layer.addItem(option);

    layer.payload = parentLayer.payload.slice(optionDataOffset);
    layer.addItem({
      name: 'Payload',
      value: new Value(layer.payload),
      range: optionDataOffset + ':'
    });

    layer.summary = `${layer.attr('src').data} -> ${layer.attr('dst').data} seq:${seq.data} ack:${ack.data}`;

    let id = layer.attr('src').data + '/' + layer.attr('dst').data;
    let chunk = new StreamChunk(layer.namespace, id);
    if (flags.data['FIN'] && flags.data['ACK']) {
      stream.end();
    }

    return [layer, chunk];
  }
};

/*

export default class TCPDissector {
  constructor(options) {}

  analyze(packet, parentLayer) {
    function assertLength(len) {
      if (parentLayer.payload.length < len) {
        throw new Error('too short frame');
      }
    }

    let layer = new Layer();
    layer.name = 'TCP';
    layer.namespace = parentLayer.namespace.replace('<TCP>', 'TCP');

    try {

      assertLength(2);
      let source = parentLayer.payload.readUInt16BE(0, true)
      layer.fields.push({
        name: 'Source port',
        value: source,
        data: parentLayer.payload.slice(0, 2)
      });

      let srcAddr = parentLayer.attrs.src;

      if (srcAddr.constructor.name === 'IPv4Address') {
        layer.attrs.src = new IPv4Host(srcAddr, source);
      } else {
        layer.attrs.src = new IPv6Host(srcAddr, source);
      }

      assertLength(4);
      let destination = parentLayer.payload.readUInt16BE(2, true);
      layer.fields.push({
        name: 'Destination port',
        value: destination,
        data: parentLayer.payload.slice(2, 4)
      });

      let dstAddr = parentLayer.attrs.dst;
      if (dstAddr.constructor.name === 'IPv4Address') {
        layer.attrs.dst = new IPv4Host(dstAddr, destination);
      } else {
        layer.attrs.dst = new IPv6Host(dstAddr, destination);
      }

      assertLength(8);
      let seq = parentLayer.payload.readUInt32BE(4, true);
      layer.fields.push({
        name: 'Sequence number',
        attr: 'seq',
        data: parentLayer.payload.slice(4, 8)
      });
      layer.attrs.seq = seq;

      assertLength(12);
      let ack = parentLayer.payload.readUInt32BE(8, true);
      layer.fields.push({
        name: 'Acknowledgment number',
        attr: 'ack',
        data: parentLayer.payload.slice(8, 12)
      });
      layer.attrs.ack = ack;

      assertLength(13);
      let dataOffset = (parentLayer.payload.readUInt8(12, true) >> 4);
      layer.fields.push({
        name: 'Data offset',
        attr: 'dataOffset',
        data: parentLayer.payload.slice(12, 13)
      });
      layer.attrs.dataOffset = dataOffset;

      assertLength(14);
      let flags = new TCPFlags(parentLayer.payload.readUInt8(13, true) |
        ((parentLayer.payload.readUInt8(12, true) & 0x1) << 8));

      layer.fields.push({
        name: 'Flags',
        attr: 'flags',
        data: parentLayer.payload.slice(12, 14),
        fields: [{
          name: 'NS',
          value: flags.get('NS'),
          data: parentLayer.payload.slice(12, 13)
        }, {
          name: 'CWR',
          value: flags.get('CWR'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'ECE',
          value: flags.get('ECE'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'URG',
          value: flags.get('URG'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'ACK',
          value: flags.get('ACK'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'PSH',
          value: flags.get('PSH'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'RST',
          value: flags.get('RST'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'SYN',
          value: flags.get('SYN'),
          data: parentLayer.payload.slice(13, 14)
        }, {
          name: 'FIN',
          value: flags.get('FIN'),
          data: parentLayer.payload.slice(13, 14)
        }]
      });
      layer.attrs.flags = flags;

      assertLength(16);
      layer.fields.push({
        name: 'Window size',
        attr: 'window',
        data: parentLayer.payload.slice(14, 16)
      });
      layer.attrs.window = parentLayer.payload.readUInt16BE(14, true);

      assertLength(18);
      layer.fields.push({
        name: 'Checksum',
        attr: 'checksum',
        data: parentLayer.payload.slice(16, 18)
      });
      layer.attrs.checksum = parentLayer.payload.readUInt16BE(16, true);

      assertLength(20);
      layer.fields.push({
        name: 'Urgent pointer',
        attr: 'urgent',
        data: parentLayer.payload.slice(18, 20)
      })
      layer.attrs.urgent = parentLayer.payload.readUInt16BE(18, true);

      assertLength(dataOffset * 4);
      let optionItems = [];
      let option = {
        name: 'Options',
        data: parentLayer.payload.slice(20, dataOffset * 4),
        fields: []
      };

      let optionData = parentLayer.payload.slice(0, dataOffset * 4);
      let optionOffset = 20;

      function checkLength(payload, offset, len) {
        if (!(payload.length >= offset + len && payload[offset + 1] === len)) {
          throw new Error('invalid option');
        }
      }

      while (optionData.length > optionOffset) {
        switch (optionData[optionOffset]) {
          case 0:
            optionOffset = optionData.length;
            break;

          case 1:
            option.fields.push({
              name: 'NOP',
              value: '',
              data: parentLayer.payload.slice(optionOffset, optionOffset + 1)
            });
            optionOffset++;
            break;

          case 2:
            checkLength(optionData, optionOffset, 4);
            optionItems.push('Maximum segment size');
            option.fields.push({
              name: 'Maximum segment size',
              value: parentLayer.payload.readUInt16BE(optionOffset + 2, true),
              data: parentLayer.payload.slice(optionOffset, optionOffset + 4)
            });
            optionOffset += 4;
            break;

          case 3:
            checkLength(optionData, optionOffset, 3);
            optionItems.push('Window scale');
            option.fields.push({
              name: 'Window scale',
              value: parentLayer.payload.readUInt8(optionOffset + 2, true),
              data: parentLayer.payload.slice(optionOffset, optionOffset + 3)
            });
            optionOffset += 3;
            break;

          case 4:
            checkLength(optionData, optionOffset, 2);
            optionItems.push('Selective ACK permitted');
            option.fields.push({
              name: 'Selective ACK permitted',
              value: '',
              data: parentLayer.payload.slice(optionOffset, optionOffset + 2)
            });
            optionOffset += 2;
            break;

            // TODO: https://tools.ietf.org/html/rfc2018
          case 5:
            checkLength(optionData, optionOffset, 2)
            let length = parentLayer.payload.readUInt8(optionOffset + 1, true);
            checkLength(optionData.length, optionOffset, length);
            optionItems.push('Selective ACK');
            option.fields.push({
              name: 'Selective ACK',
              value: parentLayer.payload.slice(optionOffset + 2, optionOffset + length),
              data: parentLayer.payload.slice(optionOffset, optionOffset + length)
            });

            optionOffset += length;
            break;

          case 8:
            checkLength(optionData, optionOffset, 10);
            let mt = parentLayer.payload.readUInt32BE(optionOffset + 2, true);
            let et = parentLayer.payload.readUInt32BE(optionOffset + 2, true);
            optionItems.push('Timestamps');
            option.fields.push({
              name: 'Timestamps',
              value: `${mt} - ${et}`,
              data: parentLayer.payload.slice(optionOffset, optionOffset + 10),
              fields: [{
                name: 'My timestamp',
                value: mt,
                data: parentLayer.payload.slice(optionOffset + 2, optionOffset + 6)
              }, {
                name: 'Echo reply timestamp',
                value: et,
                data: parentLayer.payload.slice(optionOffset + 6, optionOffset + 10)
              }]
            });
            optionOffset += 10;
            break;

          default:
            throw new Error('unknown option');
        }
      }

      option.value = optionItems.join(',');
      layer.fields.push(option);

      layer.payload = parentLayer.payload.slice(dataOffset * 4);

      layer.fields.push({
        name: 'Payload',
        value: layer.payload,
        data: layer.payload
      });

      let stream = new PacketStream('TCP Stream', parentLayer.namespace, layer.attrs.src + '/' + layer.attrs.dst);
      if (flags.get('FIN') && flags.get('ACK')) {
        stream.end();
      }
      stream.data = layer.payload;
      layer.streams.push(stream);

      layer.summary = `${layer.attrs.src} -> ${layer.attrs.dst} seq:${seq} ack:${ack}`;

    } catch (err) {
      layer.error = err.message;
    }

    parentLayer.layers[layer.namespace] = layer;
  }
}

let table = {
  'NS': 0x1 << 8,
  'CWR': 0x1 << 7,
  'ECE': 0x1 << 6,
  'URG': 0x1 << 5,
  'ACK': 0x1 << 4,
  'PSH': 0x1 << 3,
  'RST': 0x1 << 2,
  'SYN': 0x1 << 1,
  'FIN': 0x1 << 0,
};

*/
