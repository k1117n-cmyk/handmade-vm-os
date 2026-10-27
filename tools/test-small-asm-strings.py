"""Day 39: literal bytes, placement, relocation, diagnostics and execution."""
from pathlib import Path
import subprocess
import sys
import tempfile

ASM = str(Path(sys.argv[1]).resolve())
VM = str(Path(sys.argv[2]).resolve())
count = 0


def run(args):
    return subprocess.run(args, input=b'', capture_output=True, timeout=10)


with tempfile.TemporaryDirectory(prefix='small-asm-strings-') as directory:
    source = Path(directory) / 'input.asm'
    output = Path(directory) / 'output.bin'

    def good(text, expected):
        global count
        source.write_text(text)
        result = run([ASM, str(source), str(output)])
        assert (result.returncode, result.stderr) == (0, b''), result
        assert output.read_bytes() == expected, (text[:200], output.read_bytes()[:100])
        count += 1

    def bad(text, line, message):
        global count
        source.write_text(text)
        for existing in (False, True):
            if output.exists():
                output.unlink()
            if existing:
                output.write_bytes(b'keep')
            result = run([ASM, str(source), str(output)])
            assert (result.returncode, result.stdout) == (1, b''), result
            assert result.stderr == f'{source}:{line}: {message}\n'.encode(), result
            assert output.read_bytes() == b'keep' if existing else not output.exists()
        count += 1

    good('.string ""\n', b'\0')
    good('.sTrInG "Ab c;#:,0" ; comment: ignored\n', b'Ab c;#:,0\0')
    good(r'.string "a\n\r\t\\\";#:z"' + '\n', b'a\n\r\t\\";#:z\0')
    good('.string "a"\n.string "B"', b'a\0B\0')
    good('x: .string ":"\n.align 0x4\nMOVI R7, X\n', b':\0\0\0' + bytes.fromhex('40700000'))
    good('MOVI R0, text\nHALT\ntext: .string "OK"\n', bytes.fromhex('4000000801000000') + b'OK\0')
    good('.string ""\nx: .string "A"\n.align 4\nMOVI R0, x\n', b'\0A\0\0' + bytes.fromhex('40000001'))
    good('.string ""\npad: .align 4\nMOVI R0, pad\n', b'\0'*4 + bytes.fromhex('40000001'))
    good('.align 4\n.align 4\nHALT\n', bytes.fromhex('01000000'))
    good('.string "A"\r\n.align 4\r\nHALT\r\n', b'A\0\0\0' + bytes.fromhex('01000000'))
    # Every possible amount of padding; independent expected bytes and VM output.
    sample = Path('programs/asm-string.asm').read_text()
    for text in ('Hello, VM!', 'Hi', 'Hi!', 'A', ''):
        payload = text.encode() + b'\n\0'
        padding = bytes((-len(payload)) % 4)
        target = 4 + len(payload) + len(padding)
        expected = (0x68000000 | target).to_bytes(4, 'big') + payload + padding + bytes.fromhex('400000046000000101000000')
        good(sample.replace('Hello, VM!', text), expected)
        result = run([VM, str(output)])
        assert (result.returncode, result.stdout, result.stderr) == (0, text.encode()+b'\nCPU halted.\n', b''), result
        count += 1
    for text, line, message in [
        ('.string\n', 1, 'invalid string'),
        ('.string A\n', 1, 'invalid string'),
        ('.string "abc\n', 1, 'invalid string'),
        ('.string "a" "b"\n', 1, 'invalid string'),
        ('.string "a",\n', 1, 'invalid string'),
        ('.string "日本語"\n', 1, 'invalid string'),
        ('.string "a\tb"\n', 1, 'invalid string'),
        ('.string "a\x01b"\n', 1, 'invalid string'),
        (r'.string "\0"', 1, 'unsupported string escape'),
        (r'.string "\x41"', 1, 'unsupported string escape'),
        (r'.string "\q"', 1, 'unsupported string escape'),
        ('.string "abc' + '\\', 1, 'unsupported string escape'),
        ('.string "a\\"', 1, 'invalid string'),
        ('.align\n', 1, 'invalid alignment'),
        ('.align 0\n', 1, 'invalid alignment'),
        ('.align 8\n', 1, 'invalid alignment'),
        ('.align 4 4\n', 1, 'invalid alignment'),
        ('a: .align a\n', 1, 'invalid alignment'),
        ('.string ""\nHALT\n', 2, 'instruction address is not aligned'),
        ('MOVI R0, missing\n', 1, 'undefined label'),
        ('a: .string ":"\nA: HALT\n', 2, 'duplicate label'),
        ('a: b: .string "x"\n', 1, 'invalid label definition'),
        ('.string "a":\n', 1, 'invalid string'),
        ('.string "' + 'x'*245 + '"\n', 1, 'line too long'),
    ]:
        bad(text, line, message)
    good('.string "' + 'x'*244 + '"\n', b'x'*244 + b'\0')
    # Fill to four bytes before the 20-bit limit, then vary the trailing data.
    prefix = 'HALT\n' * 262143
    prefix_bytes = bytes.fromhex('01000000') * 262143
    good(prefix + '.string "ab"\nlast: .string ""\n', prefix_bytes + b'ab\0\0')
    good('MOVI R0, last\n' + 'HALT\n'*262142 + '.string "ab"\nlast: .string ""\n',
         bytes.fromhex('400fffff') + bytes.fromhex('01000000')*262142 + b'ab\0\0')
    good(prefix + '.string ""\n.align 4\n', prefix_bytes + bytes(4))
    bad(prefix + '.string "abcd"\n', 262144, 'program too large')
    bad(prefix + '.string "abc"\n.string ""\n', 262145, 'program too large')
    bad(prefix + '.string ""\n.align 4\npast:\n', 262146, 'label address out of range')
    bad(prefix + '.string ""\n.align 4\nHALT\n', 262146, 'program too large')

print(f'small-asm-strings: {count} cases passed')
