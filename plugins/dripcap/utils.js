import {Value} from 'dripcap';

export function Enum(table, value) {
  let name = table[value] || 'Unknown';
  let val = {};
  val[name] = value;
  return new Value(val, 'dripcap/enum');
}

export default function () {
    throw 0;
}
