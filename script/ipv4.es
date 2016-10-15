import {Layer, Item, Value} from 'dripcap';
import {Flags, Enum} from 'dripcap/utils';
import {IPv4Address} from 'dripcap/ipv4';

export default class Dissector {
  analyze(packet, parentLayer) {
    let layer = new Layer('::Ethernet::IPv4');
    layer.name = 'IPv4';
    layer.alias = 'ipv4';

    let version = new Value(parentLayer.payload.readUInt8(0) >> 4);
    layer.addItem({
      name: 'Version',
      value: version,
      range: '0:1'
    });
    layer.setAttr('version', version);

    let headerLength = new Value(parentLayer.payload.readUInt8(0) & 0b00001111);
    layer.addItem({
      name: 'Internet Header Length',
      value: headerLength,
      range: '0:1'
    });
    layer.setAttr('headerLength', headerLength);

    let type = new Value(parentLayer.payload.readUInt8(1));
    layer.addItem({
      name: 'Type of service',
      value: type,
      range: '1:2'
    });
    layer.setAttr('type', type);

    let totalLength = new Value(parentLayer.payload.readUInt16BE(2));
    layer.addItem({
      name: 'Total Length',
      value: totalLength,
      range: '2:4'
    });
    layer.setAttr('totalLength', totalLength);

    let id = new Value(parentLayer.payload.readUInt16BE(4));
    layer.addItem({
      name: 'Identification',
      value: id,
      range: '4:6'
    });
    layer.setAttr('id', id);

    let flagTable = {
      'Reserved': 0x1,
      'Don\'t Fragment': 0x2,
      'More Fragments': 0x4,
    };

    let flags = Flags(flagTable, (parentLayer.payload.readUInt8(6) >> 5) & 0x7);

    layer.addItem({
      name: 'Flags',
      value: flags,
      range: '6:7',
      children: [
        {
          name: 'Reserved',
          value: new Value(flags.data['Reserved']),
          range: '6:7'
        },
        {
          name: 'Don\'t Fragment',
          value: new Value(flags.data['Don\'t Fragment']),
          range: '6:7'
        },
        {
          name: 'More Fragments',
          value: new Value(flags.data['More Fragments']),
          range: '6:7'
        }
      ]
    });

    let fragmentOffset = new Value(parentLayer.payload.readUInt8(6) & 0b0001111111111111);
    layer.addItem({
      name: 'Fragment Offset',
      value: fragmentOffset,
      range: '6:8',
    });
    layer.setAttr('fragmentOffset', fragmentOffset);

    let ttl = new Value(parentLayer.payload.readUInt8(8));
    layer.addItem({
      name: 'TTL',
      value: ttl,
      range: '8:9',
    });
    layer.setAttr('ttl', ttl);

    let protocolNumber = parentLayer.payload.readUInt8(9);
    let protocol = Enum(protocolTable, protocolNumber);
    layer.addItem({
      name: 'Protocol',
      value: protocol,
      range: '9:10'
    });
    layer.setAttr('protocol', protocol);

    let protocolName = protocolTable[protocolNumber]
    if (protocolName != null) {
      layer.namespace = `::Ethernet::IPv4::<${protocolName}>`;
    }

    let checksum = new Value(parentLayer.payload.readUInt16BE(10));
    layer.addItem({
      name: 'Header Checksum',
      value: checksum,
      range: '10:12',
    });
    layer.setAttr('checksum', checksum);

    let source = IPv4Address(parentLayer.payload.slice(12, 16));
    layer.addItem({
      name: 'Source IP Address',
      value: source,
      range: '12:16',
    });
    layer.setAttr('src', source);

    let destination = IPv4Address(parentLayer.payload.slice(16, 20));
    layer.addItem({
      name: 'Destination IP Address',
      value: destination,
      range: '16:20',
    });
    layer.setAttr('dst', destination);

    layer.payload = parentLayer.payload.slice(20, totalLength.data);
    layer.addItem({
      name: 'Payload',
      value: new Value(layer.payload),
      range: '20:' + totalLength.data
    });

    layer.summary = `${source.data} -> ${destination.data}`;
    if (protocolName != null) {
      layer.summary = `[${protocolName}] ` + layer.summary;
    }

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
      let flags = new FieldFlags((parentLayer.payload.readUInt8(6) >> 5) & 0x7);
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
      layer.attrs.fragmentOffset = parentLayer.payload.readUInt8(6) & 0b0001111111111111;

      assertLength(9);
      layer.fields.push({
        name: 'TTL',
        attr: 'ttl',
        data: parentLayer.payload.slice(8, 9),
      });
      layer.attrs.ttl = parentLayer.payload.readUInt8(8);

      assertLength(10);
      let protocol = new ProtocolEnum(parentLayer.payload.readUInt8(9));

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
      layer.attrs.checksum = parentLayer.payload.readUInt16BE(10);

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

let protocolTable = {
  0x00: 'HOPOPT',
  0x01: 'ICMP',
  0x02: 'IGMP',
  0x03: 'GGP',
  0x04: 'IP-in-IP',
  0x05: 'ST',
  0x06: 'TCP',
  0x07: 'CBT',
  0x08: 'EGP',
  0x09: 'IGP',
  0x0A: 'BBN-RCC-MON',
  0x0B: 'NVP-II',
  0x0C: 'PUP',
  0x0D: 'ARGUS',
  0x0E: 'EMCON',
  0x0F: 'XNET',
  0x10: 'CHAOS',
  0x11: 'UDP',
  0x12: 'MUX',
  0x13: 'DCN-MEAS',
  0x14: 'HMP',
  0x15: 'PRM',
  0x16: 'XNS-IDP',
  0x17: 'TRUNK-1',
  0x18: 'TRUNK-2',
  0x19: 'LEAF-1',
  0x1A: 'LEAF-2',
  0x1B: 'RDP',
  0x1C: 'IRTP',
  0x1D: 'ISO-TP4',
  0x1E: 'NETBLT',
  0x1F: 'MFE-NSP',
  0x20: 'MERIT-INP',
  0x21: 'DCCP',
  0x22: '3PC',
  0x23: 'IDPR',
  0x24: 'XTP',
  0x25: 'DDP',
  0x26: 'IDPR-CMTP',
  0x27: 'TP++',
  0x28: 'IL',
  0x29: 'IPv6',
  0x2A: 'SDRP',
  0x2B: 'Route',
  0x2C: 'Frag',
  0x2D: 'IDRP',
  0x2E: 'RSVP',
  0x2F: 'GRE',
  0x30: 'MHRP',
  0x31: 'BNA',
  0x32: 'ESP',
  0x33: 'AH',
  0x34: 'I-NLSP',
  0x35: 'SWIPE',
  0x36: 'NARP',
  0x37: 'MOBILE',
  0x38: 'TLSP',
  0x39: 'SKIP',
  0x3A: 'ICMP',
  0x3B: 'NoNxt',
  0x3C: 'Opts',
  0x3E: 'CFTP',
  0x40: 'SAT-EXPAK',
  0x41: 'KRYPTOLAN',
  0x42: 'RVD',
  0x43: 'IPPC',
  0x45: 'SAT-MON',
  0x46: 'VISA',
  0x47: 'IPCU',
  0x48: 'CPNX',
  0x49: 'CPHB',
  0x4A: 'WSN',
  0x4B: 'PVP',
  0x4C: 'BR-SAT-MON',
  0x4D: 'SUN-ND',
  0x4E: 'WB-MON',
  0x4F: 'WB-EXPAK',
  0x50: 'ISO-IP',
  0x51: 'VMTP',
  0x52: 'SECURE-VMTP',
  0x53: 'VINES',
  0x54: 'TTP',
  0x54: 'IPTM',
  0x55: 'NSFNET-IGP',
  0x56: 'DGP',
  0x57: 'TCF',
  0x58: 'EIGRP',
  0x59: 'OSPF',
  0x5A: 'Sprite-RPC',
  0x5B: 'LARP',
  0x5C: 'MTP',
  0x5D: 'AX.25',
  0x5E: 'IPIP',
  0x5F: 'MICP',
  0x60: 'SCC-SP',
  0x61: 'ETHERIP',
  0x62: 'ENCAP',
  0x64: 'GMTP',
  0x65: 'IFMP',
  0x66: 'PNNI',
  0x67: 'PIM',
  0x68: 'ARIS',
  0x69: 'SCPS',
  0x6A: 'QNX',
  0x6B: 'A/N',
  0x6C: 'IPComp',
  0x6D: 'SNP',
  0x6E: 'Compaq-Peer',
  0x6F: 'IPX-in-IP',
  0x70: 'VRRP',
  0x71: 'PGM',
  0x73: 'L2TP',
  0x74: 'DDX',
  0x75: 'IATP',
  0x76: 'STP',
  0x77: 'SRP',
  0x78: 'UTI',
  0x79: 'SMP',
  0x7A: 'SM',
  0x7B: 'PTP',
  0x7C: 'IS-IS',
  0x7D: 'FIRE',
  0x7E: 'CRTP',
  0x7F: 'CRUDP',
  0x80: 'SSCOPMCE',
  0x81: 'IPLT',
  0x82: 'SPS',
  0x83: 'PIPE',
  0x84: 'SCTP',
  0x85: 'FC',
  0x86: 'RSVP-E2E-IGNORE',
  0x87: 'RFC6275',
  0x88: 'UDPLite',
  0x89: 'MPLS-in-IP',
  0x8A: 'manet',
  0x8B: 'HIP',
  0x8C: 'Shim6',
  0x8D: 'WESP',
  0x8E: 'ROHC',
};
