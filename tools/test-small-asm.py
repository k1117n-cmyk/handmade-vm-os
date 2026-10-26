"""Day 37: check the assembler's bytes, errors, writer compatibility and VM output."""
from pathlib import Path
import subprocess
import sys
import tempfile

ASM = str(Path(sys.argv[1] if len(sys.argv) > 1 else './tools/small-asm').resolve())
VM = str(Path(sys.argv[2] if len(sys.argv) > 2 else './handmade-vm').resolve())
count = 0


def run(args):
    return subprocess.run(args, input=b'', capture_output=True, timeout=5)


with tempfile.TemporaryDirectory(prefix='small-asm-test-') as directory:
    source = Path(directory) / 'input.asm'
    output = Path(directory) / 'output.bin'

    def assemble(text):
        source.write_text(text)
        return run([ASM, str(source), str(output)])

    def good(text, expected):
        global count
        result = assemble(text)
        assert result.returncode == 0 and result.stderr == b'', (text, result)
        assert output.read_bytes() == expected, (text, output.read_bytes(), expected)
        count += 1

    def words(text, values):
        good(text, b''.join(value.to_bytes(4, 'big') for value in values))

    # Independent fixed vectors include asymmetric rd/rs to catch swapped fields.
    words('LDB R2, [R7]\nSTB [R7], R2\nINC R7\nCALLI 0x20\nRET\n',
          [0x30270000, 0x31720000, 0x20700000, 0x69000020, 0x02000000])
    words('LDB R0,[R0]\nSTB [R0],R0\nINC R0\nCALLI 0\nCALLI 0xfffff\n',
          [0x30000000, 0x31000000, 0x20000000, 0x69000000, 0x690fffff])
    words('  ldb r7,[r0] ; comment\r\n\tstb [r0],r7 # comment\ninc r1\ncalli 32\nret',
          [0x30700000, 0x31070000, 0x20100000, 0x69000020, 0x02000000])
    words('CALLI 040\nCALLI +32\nCALLI -0\nCALLI 3\n',
          [0x69000020, 0x69000020, 0x69000000, 0x69000003])
    words('LDB R0 [R1]\nSTB [R1],,R0\n', [0x30010000, 0x31100000])
    good('; comment\n\n# comment\n', b'')

    invalid = [
        'RET R0', 'RET 0', 'INC', 'INC R8', 'INC R10', 'INC R-1',
        'INC [R0]', 'INC R0 R1', 'CALLI', 'CALLI -1', 'CALLI 0x100000',
        'CALLI 1048576', 'CALLI 999999999999999999999999999999',
        'CALLI 12x', 'CALLI 08', 'CALLI 0 R0',
        'LDB', 'LDB R0', 'LDB R0, R1', 'LDB [R0], R1',
        'LDB R8, [R0]', 'LDB R0, [R8]', 'LDB R0, [R10]',
        'LDB R0, [R1', 'LDB R0, R1]', 'LDB R0, [[R1]]',
        'LDB R0, [ R1 ]', 'LDB R0, [R1]x', 'LDB R0, []',
        'LDB R0, [R1], R2', 'STB', 'STB [R1]', 'STB R1, R0',
        'STB R0, [R1]', 'STB [R8], R0', 'STB [R0], R8',
        'STB [R0], [R1]', 'STB [[R0]], R1', 'STB [R0], R1 R2',
        '.byte 65', 'MOV R0, R1',
    ]
    for text in invalid:
        result = assemble('; first line\n' + text + '\n')
        assert result.returncode == 1 and result.stdout == b'', (text, result)
        assert result.stderr == f'{source}:2: unsupported or invalid instruction\n'.encode(), (text, result)
        count += 1

    result = assemble('INC R0' + ' ' * 256 + '\n')
    assert result.returncode == 1 and b':1: line too long\n' in result.stderr, result
    count += 1

    # Compare existing programs with the committed binaries, not regenerated expectations.
    for name in ('hello', 'echo-char', 'prompt-echo', 'one-char-command', 'command-loop'):
        good(Path(f'programs/{name}.asm').read_text(), Path(f'programs/{name}.bin').read_bytes())

    # Transcribed from Day 36's input and comparison routines. Addresses are absolute.
    routines = '''MOVI R1, 0x500
SYSCALL 2
MOVI R2, 10
CMP R0, R2
JZ 0xD4
MOVI R2, 13
CMP R0, R2
JZ 0xA0
MOVI R2, 0x53F
CMP R1, R2
JZ 0xA0
STB [R1], R0
INC R1
JUMP 0xA0
MOVI R0, 0
STB [R1], R0
RET
MOVI R4, 0
LDB R0, [R1]
LDB R3, [R2]
CMP R0, R3
JNZ 0x110
CMP R0, R4
JZ 0x108
INC R1
INC R2
JUMP 0xE4
MOVI R0, 1
RET
MOVI R0, 0
RET
'''
    echo = Path('programs/echo-command.bin').read_bytes()
    good(routines, echo[0x9c:0x118])
    good('CALLI 0x9c\n', echo[0x84:0x88])

    words(Path('programs/asm-memory-call.asm').read_text(), [
        0x40100500, 0x40000041, 0x31100000, 0x20100000,
        0x40000042, 0x31100000, 0x40100500, 0x69000028,
        0x69000028, 0x01000000, 0x30010000, 0x60000000,
        0x20100000, 0x02000000,
    ])
    result = run([VM, str(output)])
    assert (result.returncode, result.stdout, result.stderr) == (0, b'A\nB\nCPU halted.\n', b''), result
    count += 1

print(f'small-asm: {count} cases passed')
