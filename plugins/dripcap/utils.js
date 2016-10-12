import {Value} from 'dripcap';

export function Enum(table, value) {
  let name = table[value] || 'Unknown';
  let val = {};
  val[name] = value;
  return new Value(val, 'dripcap/enum');
}

export function Flags(table, value) {
  let val = {};
  for (let name in table) {
    val[name] = !!(table[name] & value);
  }
  return new Value(val, 'dripcap/flags');
}

export default function () {}
