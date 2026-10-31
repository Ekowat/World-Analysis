import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
from verify_dot60 import build_ids, matches
from package_levipack import build_manifest


def elf_with_note(note):
    data = bytearray(120)
    data[:6] = b'\x7fELF\x02\x01'
    struct.pack_into('<H', data, 18, 183)
    struct.pack_into('<Q', data, 32, 64)
    struct.pack_into('<HH', data, 54, 56, 1)
    struct.pack_into('<IIQQQQQQ', data, 64, 4, 4, 120, 120, 0, len(note), len(note), 4)
    return bytes(data)+note


class CompatibilityTests(unittest.TestCase):
    def test_build_id_and_four_byte_note_alignment(self):
        unknown = struct.pack('<III', 1, 1, 7)+b'x\0\0\0v\0\0\0'
        note = struct.pack('<III', 4, 20, 3)+b'GNU\0'+bytes(range(20))
        self.assertEqual(build_ids(elf_with_note(unknown+note)), [bytes(range(20)).hex()])

    def test_truncated_elf_note_rejected(self):
        note = struct.pack('<III', 4, 20, 3)+b'GNU\0'+b'\x01'*19
        with self.assertRaisesRegex(ValueError, 'Truncated'):
            build_ids(elf_with_note(note))

    def test_note_length_overflow_rejected(self):
        with self.assertRaisesRegex(ValueError, 'Truncated'):
            build_ids(elf_with_note(struct.pack('<III', 0xffffffff, 20, 3)))

    def test_invalid_program_headers_rejected(self):
        data = bytearray(elf_with_note(b''))
        struct.pack_into('<Q', data, 32, 0xffffffffffffffff)
        with self.assertRaisesRegex(ValueError, 'program header'):
            build_ids(data)

    def test_pattern_overlaps_and_boundaries(self):
        self.assertEqual(matches('AA ? AA', [(0x1000, bytes.fromhex('AA AA AA AA AA'))]),
                         [0x1000, 0x1001, 0x1002])
        self.assertEqual(matches('AA ? BB', [(0x1000, b'\xaa\x00'), (0x2000, b'\xbb')]), [])

    def test_launcher_version_contract(self):
        manifest = build_manifest(dict(Name='World Analysis', Author='Ekowat', Description='test', Version='1.5.3'))
        self.assertEqual(manifest['minecraft_versions'], ['1.26.60*'])
        self.assertNotIn('minecraft', manifest)


if __name__ == '__main__':
    unittest.main()
