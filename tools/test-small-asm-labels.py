"""Day 38: label resolution, byte compatibility, diagnostics and VM execution."""
from pathlib import Path
import subprocess
import sys
import tempfile

ASM = str(Path(sys.argv[1] if len(sys.argv) > 1 else './tools/small-asm').resolve())
VM = str(Path(sys.argv[2] if len(sys.argv) > 2 else './handmade-vm').resolve())
count = 0


def run(args):
    return subprocess.run(args, input=b'', capture_output=True, timeout=10)


with tempfile.TemporaryDirectory(prefix='small-asm-labels-') as directory:
    source = Path(directory) / 'input.asm'
    output = Path(directory) / 'output.bin'

    def assemble(text):
        source.write_text(text)
        return run([ASM, str(source), str(output)])

    def good(text, expected):
        global count
        result = assemble(text)
        assert (result.returncode, result.stderr) == (0, b''), (text[:200], result)
        assert output.read_bytes() == expected, text[:200]
        count += 1

    def words(text, values):
        good(text, b''.join(value.to_bytes(4, 'big') for value in values))

    def bad(text, line, message):
        global count
        # Both a new destination and an existing destination must survive errors.
        for existing in (False, True):
            if output.exists():
                output.unlink()
            if existing:
                output.write_bytes(b'keep existing output')
            result = assemble(text)
            assert (result.returncode, result.stdout) == (1, b''), result
            assert result.stderr == f'{source}:{line}: {message}\n'.encode(), result
            assert (output.read_bytes() == b'keep existing output') if existing else not output.exists()
        count += 1

    words('start: JUMP end\nJZ start\nJNZ end\nCALLI start\nend: HALT\n',
          [0x68000010, 0x6A000000, 0x6B000010, 0x69000000, 0x01000000])
    words('jump Next\n\n# ignored: HALT\nnext : ; comment: x\n_other9:\n halt',
          [0x68000004, 0x01000000])
    words('JUMP target\nINC R7\ntarget: HALT\n', [0x68000008, 0x20700000, 0x01000000])
    words('HALT: JUMP halt\nR0: CALLI r0\n', [0x68000000, 0x69000004])
    words('loop:\nJUMP 4\nCALLI loop\nJZ 0x0\nJNZ +4\n',
          [0x68000004, 0x69000000, 0x6A000000, 0x6B000004])
    words('entry:\r\n\thalt\r\nend:\r\n', [0x01000000])
    words('loop:', [])
    words('JUMP end\nHALT\nend:', [0x68000008, 0x01000000])
    name = 'a' * 63
    words(f'JUMP {name}\n{name}: HALT\n', [0x68000004, 0x01000000])
    words(''.join(f'label{i}:\n' for i in range(256)) + 'CALLI label255\n', [0x69000000])

    for text, line, message in [
        ('loop:\nLOOP: HALT\n', 2, 'duplicate label'),
        ('loop: HALT\nloop: RET\n', 2, 'duplicate label'),
        ('; first\nCALLI routine\n', 2, 'undefined label'),
        ('HALT\nJUMP missing\n', 2, 'undefined label'),
        ('JZ missing\n', 1, 'undefined label'),
        ('JNZ missing\n', 1, 'undefined label'),
        ('a: b: HALT\n', 1, 'invalid label definition'),
        ('HALT\n: RET\n', 2, 'invalid label definition'),
        ('1loop: HALT\n', 1, 'invalid label definition'),
        ('a-b: HALT\n', 1, 'invalid label definition'),
        ('a b: HALT\n', 1, 'invalid label definition'),
        ('a,b: HALT\n', 1, 'invalid label definition'),
        ('日本語: HALT\n', 1, 'invalid label definition'),
        ('a' * 64 + ': HALT\n', 1, 'invalid label definition'),
        ('JUMP ' + 'a' * 64 + '\n', 1, 'unsupported or invalid instruction'),
        ('a: MOVI R0, a\n', 1, 'unsupported or invalid instruction'),
        ('a: SYSCALL a\n', 1, 'unsupported or invalid instruction'),
        ('a: JUMP a+4\n', 1, 'unsupported or invalid instruction'),
        ('a: JUMP a R0\n', 1, 'unsupported or invalid instruction'),
        ('a: UNKNOWN\n', 1, 'unsupported or invalid instruction'),
        ('a: HALT 0\n', 1, 'unsupported or invalid instruction'),
        ('a: .byte 65\n', 1, 'unsupported or invalid instruction'),
        ('a: ' + ' ' * 256 + '\n', 1, 'line too long'),
        (''.join(f'label{i}:\n' for i in range(257)), 257, 'too many labels'),
    ]:
        bad(text, line, message)

    # The immediate field is 20 bits; exercise the last aligned address and overflow.
    good('JUMP last\n' + 'HALT\n' * 262142 + 'last: HALT\n',
         bytes.fromhex('680ffffc') + bytes.fromhex('01000000') * 262143)
    bad('HALT\n' * 262144 + 'past:\n', 262145, 'label address out of range')
    bad('HALT\n' * 262145, 262145, 'program too large')

    # Keep the numeric source as an independent reference and compare its binary too.
    sample = Path('programs/asm-label-call.asm').read_text()
    expected = Path('programs/asm-memory-call.bin').read_bytes()
    good(Path('programs/asm-memory-call.asm').read_text(), expected)
    good(sample, expected)
    result = run([VM, str(output)])
    assert (result.returncode, result.stdout, result.stderr) == (0, b'A\nB\nCPU halted.\n', b''), result
    count += 1
    # Insert a harmless instruction before the routine: both CALLIs must move to 0x2C.
    moved = expected[:28] + bytes.fromhex('6900002c6900002c') + expected[36:40] + bytes.fromhex('20700000') + expected[40:]
    good(sample.replace('HALT\n', 'HALT\nINC R7\n'), moved)
    result = run([VM, str(output)])
    assert (result.returncode, result.stdout, result.stderr) == (0, b'A\nB\nCPU halted.\n', b''), result
    count += 1
    # A forward conditional branch and a backward loop execute, rather than only encode.
    words('MOVI R0, 65\nMOVI R1, 67\nloop: SYSCALL 0\nINC R0\nCMP R0, R1\nJZ done\nJNZ loop\ndone: HALT\n',
          [0x40000041, 0x40100043, 0x60000000, 0x20000000, 0x24010000, 0x6A00001C, 0x6B000008, 0x01000000])
    result = run([VM, str(output)])
    assert (result.returncode, result.stdout, result.stderr) == (0, b'A\nB\nCPU halted.\n', b''), result
    count += 1

print(f'small-asm-labels: {count} cases passed')
