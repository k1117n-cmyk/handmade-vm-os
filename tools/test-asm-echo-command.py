"""Day 40: compare code and relocated strings with the Day 36 writer."""
from pathlib import Path
import struct
import sys

old = Path(sys.argv[1]).read_bytes()
new = Path(sys.argv[2]).read_bytes()
code_size = 70 * 4
assert code_size <= len(new) <= 0x500, 'code/data overlaps input buffer'
# Instruction index, writer string address, destination register, exact data.
strings = [
    (0, 0x300, 0, b'Command> \0'),
    (22, 0x340, 0, b'Commands:\nhelp: show help\nquit: quit\necho: read and print a line\n\0'),
    (25, 0x3A0, 0, b'Goodbye from Handmade VM\n\0'),
    (28, 0x3E0, 0, b'Unknown command\n\0'),
    (4, 0x420, 2, b'help\0'),
    (10, 0x430, 2, b'quit\0'),
    (16, 0x440, 2, b'echo\0'),
    (31, 0x460, 0, b'Text> \0'),
]
old_code = struct.unpack('>70I', old[:code_size])
new_code = list(struct.unpack('>70I', new[:code_size]))
address = code_size
for index, previous, register, data in strings:
    opcode = 0x40000000 | register << 20
    assert old_code[index] == opcode | previous, (index, 'writer reference')
    assert new_code[index] == opcode | address, (index, 'asm reference')
    assert old[previous:previous + len(data)] == data, (index, 'writer string')
    assert new[address:address + len(data)] == data, (index, 'asm string')
    new_code[index] = old_code[index]
    address += len(data)
assert tuple(new_code) == old_code, 'non-relocated instruction changed'
assert len(new) == address, 'unexpected trailing data'
print(f'asm-echo layout: 70 instructions, 8 strings, {len(new)} bytes; buffer at 0x500')
