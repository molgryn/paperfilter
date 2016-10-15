import {Value} from 'dripcap';

export function IPv4Address(buffer) {
  let val = `${buffer[0]}.${buffer[1]}.${buffer[2]}.${buffer[3]}`
  return new Value(val, 'dripcap/ipv4/addr');
}

export default function () {}
