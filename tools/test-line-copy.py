"""Execute the actual Day 42 copy routine and inspect memory and registers."""
from pathlib import Path
import subprocess
import sys
import tempfile

ASM, VM = (str(Path(arg).resolve()) for arg in sys.argv[1:3])
source = Path('programs/line-editor.asm').read_text()
routine = source[source.index('copy_line:'):source.index('; end_copy_line')]
vectors = [
    ('normal', [b'hello'], 0),
    ('empty', [b''], 0),
    ('maximum', [b'x' * 63], 0),
    ('shorter', [b'abcdef', b'x'], 0),
    ('clear', [b'abcdef', b''], 0),
    ('NUL', [b'ab\0tail'], 0),
    ('inline pointer', [b'hello'], 4),
    ('last byte pointer', [b''], 63),
]


def store(address, data):
    code = [f'MOVI R1, {address}']
    for byte in data:
        code += [f'MOVI R0, {byte}', 'STB [R1], R0', 'INC R1']
    return code


def check_bytes(address, data):
    code = [f'MOVI R1, {address}']
    for byte in data:
        code += ['LDB R0, [R1]', f'MOVI R2, {byte}',
                 'CMP R0, R2', 'JNZ fail', 'INC R1']
    return code


with tempfile.TemporaryDirectory(prefix='line-copy-') as directory:
    asm, binary = (Path(directory) / name for name in ('copy.asm', 'copy.bin'))
    for name, lines, offset in vectors:
        destination = bytearray([165] * 66)  # Both guards: 0x5FF and 0x640.
        code = store(0x5FF, destination)
        for line in lines:
            original = (b'?' * offset + line + b'\0').ljust(64, b'!')
            assert len(original) == 64
            code += store(0x500, original)
            preserved = {1: 101, 2: 102, 4: 104, 7: 107}
            code += [f'MOVI R{reg}, {value}' for reg, value in preserved.items()]
            code += [f'MOVI R6, {0x500 + offset}', 'CALLI copy_line']
            for reg, value in preserved.items():
                code += [f'MOVI R3, {value}', f'CMP R{reg}, R3', 'JNZ fail']
            content = line.split(b'\0')[0] + b'\0'
            for reg, address in ((5, 0x600 + len(content) - 1),
                                 (6, 0x500 + offset + len(content) - 1)):
                code += [f'MOVI R3, {address}', f'CMP R{reg}, R3', 'JNZ fail']
            destination[1:1 + len(content)] = content
            code += check_bytes(0x500, original)
            code += check_bytes(0x5FF, destination)
        # Code above buffers; data is initialized by instructions before the call.
        program = 'JUMP start\n' + '.string ""\n' * (0x800 - 4)
        program += 'start:\n' + '\n'.join(code) + '\n'
        program += 'MOVI R0, 65\nSYSCALL 0\nHALT\nfail: HALT\n' + routine
        asm.write_text(program)
        r = subprocess.run([ASM, str(asm), str(binary)], capture_output=True, timeout=5)
        assert (r.returncode, r.stderr) == (0, b''), r
        r = subprocess.run([VM, str(binary)], input=b'', capture_output=True, timeout=5)
        assert (r.returncode, r.stdout, r.stderr) == (0, b'A\nCPU halted.\n', b''), (name, r)
print(f'line-copy: {len(vectors)} cases passed')
