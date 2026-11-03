"""Day 41: execute the actual split routine with independent fixed vectors."""
from pathlib import Path
import subprocess
import sys
import tempfile

ASM = str(Path(sys.argv[1]).resolve())
VM = str(Path(sys.argv[2]).resolve())
source = Path('programs/command-arguments.asm').read_text()
routine = source[source.index('split_line:'):source.index('split_done:')]
routine += 'split_done:\n    RET\n'
vectors = [
    ('echo hello', b'echo\0hello\0', 0x505, 1),
    ('  echo   hi  ', b'  echo\0  hi  \0', 0x509, 1),
    ('echo', b'echo\0', 0x504, 0),
    ('echo ', b'echo\0\0', 0x505, 1),
    ('echo   ', b'echo\0  \0', 0x507, 1),
    ('', b'\0', 0x500, 0),
    ('   ', b'   \0', 0x503, 0),
    ('echo\thi', b'echo\thi\0', 0x507, 0),
    ('echo \thi', b'echo\0\thi\0', 0x505, 1),
    ('echo a\0tail', b'echo\0a\0tail\0', 0x505, 1),
    ('echo\0 tail', b'echo\0 tail\0', 0x504, 0),
    ('x' * 63, b'x' * 63 + b'\0', 0x53F, 0),
    (' ' * 63, b' ' * 63 + b'\0', 0x53F, 0),
    (' ' * 58 + 'echo ', b' ' * 58 + b'echo\0\0', 0x53F, 1),
]
with tempfile.TemporaryDirectory(prefix='command-split-') as directory:
    asm = Path(directory) / 'split.asm'
    binary = Path(directory) / 'split.bin'
    for text, expected, pointer, flag in vectors:
        setup = ['MOVI R1, 0x500']
        for byte in text.encode() + b'\0':
            setup += [f'MOVI R0, {byte}', 'STB [R1], R0', 'INC R1']
        checks = ['CALLI split_line', f'MOVI R2, {pointer}',
                  'CMP R6, R2', 'JNZ fail', f'MOVI R2, {flag}',
                  'CMP R7, R2', 'JNZ fail', 'MOVI R1, 0x500']
        for byte in expected:
            checks += ['LDB R0, [R1]', f'MOVI R2, {byte}',
                       'CMP R0, R2', 'JNZ fail', 'INC R1']
        # Harness may exceed 0x500, so place its code safely above the buffer.
        program = 'JUMP start\n' + '.string ""\n' * (0x600 - 4)
        program += 'start:\n' + '\n'.join(setup + checks) + '\n'
        program += 'MOVI R0, 65\nSYSCALL 0\nHALT\nfail: HALT\n' + routine
        asm.write_text(program)
        result = subprocess.run([ASM, str(asm), str(binary)], capture_output=True, timeout=5)
        assert (result.returncode, result.stderr) == (0, b''), result
        result = subprocess.run([VM, str(binary)], input=b'', capture_output=True, timeout=5)
        assert (result.returncode, result.stdout, result.stderr) == (0, b'A\nCPU halted.\n', b''), (text, result)
print(f'command-split: {len(vectors)} cases passed')
