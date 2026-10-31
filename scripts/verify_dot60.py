"""Check the supplied dot60 ELF against the checked-in profile (Python stdlib only)."""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

from verify_dot52 import executable_sections, matches


def build_ids(data):
    if len(data) < 64 or data[:6] != b'\x7fELF\x02\x01':
        raise ValueError('Expected a little-endian ELF64 file')
    if struct.unpack_from('<H', data, 18)[0] != 183:
        raise ValueError('Expected AArch64')
    offset = struct.unpack_from('<Q', data, 32)[0]
    stride, count = struct.unpack_from('<HH', data, 54)
    if stride < 56 or offset > len(data) or count * stride > len(data) - offset:
        raise ValueError('Invalid ELF program header table')
    result = []
    for index in range(count):
        kind, _, start, _, _, length, _, _ = struct.unpack_from('<IIQQQQQQ', data, offset + index*stride)
        if kind != 4:  # PT_NOTE
            continue
        if start > len(data) or length > len(data) - start:
            raise ValueError('ELF note segment exceeds file')
        end = start + length
        while start < end:
            if end - start < 12:
                raise ValueError('Truncated ELF note header')
            name_size, value_size, note_type = struct.unpack_from('<III', data, start)
            name = start + 12
            value = name + ((name_size+3) & ~3)
            next_note = value + ((value_size+3) & ~3)
            if value > end or next_note > end:
                raise ValueError('Truncated ELF note value')
            if note_type == 3 and data[name:name+name_size] == b'GNU\0':
                result.append(data[value:value+value_size].hex())
            start = next_note
    return result


def verify(binary, root):
    profile = json.loads((root/'compatibility/dot60-bindings.json').read_text())
    data = binary.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != profile['sha256']:
        raise ValueError('SHA-256 differs from the audited dot60 binary; do not reuse this profile')
    if build_ids(data) != [profile['build_id']]:
        raise ValueError('GNU build ID differs from the audited target')
    source = (root/'src/core/memory/Signatures.cpp').read_text()
    definitions = dict(re.findall(r'SignatureDefinition\{SignatureId::(\w+), "([^"]+)"', source))
    expected = {row['id']: row for row in profile['signatures']}
    if len(definitions) != 110 or definitions.keys() != expected.keys():
        raise ValueError('Source/profile signature inventory differs')
    sections = list(executable_sections(data))
    failures = []
    for name, pattern in definitions.items():
        row = expected[name]
        found = matches(pattern, sections)
        if pattern != row['pattern'] or found != [int(row['rva'], 16)]:
            failures.append((name, [hex(address) for address in found]))
    layouts = json.loads((root/'compatibility/dot60-layouts.json').read_text())
    for proof in layouts['instruction_proofs']:
        address, expected_bytes = int(proof['rva'], 16), bytes.fromhex(proof['bytes'])
        if not any(base <= address and address+len(expected_bytes) <= base+len(section)
                   and section[address-base:address-base+len(expected_bytes)] == expected_bytes
                   for base, section in sections):
            failures.append(('layout proof: '+proof['meaning'], hex(address)))
    if failures:
        raise ValueError(json.dumps(failures, indent=2))
    print(f'PASS: {len(definitions)}/{len(definitions)} patterns match once at their recorded executable RVAs.')
    print(f'PASS: {len(layouts["instruction_proofs"])} layout/ABI instruction fingerprints.')
    print('GNU build ID: '+profile['build_id'])
    print('SHA-256: '+digest)
    print('Static binary checks passed. Android gameplay and every retained SDK field are not covered.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    args = parser.parse_args()
    try:
        verify(args.binary, Path(__file__).resolve().parents[1])
    except (ValueError, OSError, struct.error) as error:
        raise SystemExit(str(error)) from error


if __name__ == '__main__':
    main()
