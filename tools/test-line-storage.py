"""Day 43: inherited line editing plus storage output, file bytes and restart."""
from pathlib import Path
import subprocess
import sys
import tempfile
import struct

VM, PROGRAM, ASM = (str(Path(arg).resolve()) for arg in sys.argv[1:4])
P = b'Command> '
T = P + b'Text> '
Q = P + b'Goodbye from Handmade VM\nCPU halted.\n'
U = P + b'Unknown command\n'
E = P + b'input EOF\n'
H = P + (b'Commands:\nhelp: show help\nquit: quit\n'
         b'echo [text]: print text or read a line\n'
         b'set [text]: replace the stored line or read a line\n'
         b'show: print the stored line\n'
         b'save: write the stored line to line.txt\n'
         b'load: read the stored line from line.txt\n')
cases = [
    ('initial empty', b'show\nquit\n', P + b'\n' + Q),
    ('inline set', b'set hello\nshow\nquit\n', P + P + b'hello\n' + Q),
    ('replace shorter', b'set abcdef\nset x\nshow\nquit\n', P * 3 + b'x\n' + Q),
    ('show repeatedly', b'set hello\nshow\nshow\nquit\n', P * 2 + b'hello\n' + P + b'hello\n' + Q),
    ('clear inline', b'set abc\nset \nshow\nquit\n', P * 3 + b'\n' + Q),
    ('clear next line', b'set abc\nset\n\nshow\nquit\n', P + T + P + b'\n' + Q),
    ('next line', b'set\nhello\nshow\nquit\n', T + P + b'hello\n' + Q),
    ('next line spaces', b'set\n  hello  \nshow\nquit\n', T + P + b'  hello  \n' + Q),
    ('inline spaces', b'  set   a  b  \n  show   \nquit\n', P * 2 + b'a  b  \n' + Q),
    ('separator only', b' set    \nhelp\nshow\nquit\n', P + H + P + b'\n' + Q),
    ('help and unknown preserve', b'set keep\nhelp\nwho\nshow\nquit\n', P + H + U + P + b'keep\n' + Q),
    ('inline echo preserves', b'set keep\necho other\nshow\nquit\n', P * 2 + b'other\n' + P + b'keep\n' + Q),
    ('next line echo preserves', b'set keep\necho\nother\nshow\nquit\n', P + T + b'other\n' + P + b'keep\n' + Q),
    ('empty echo preserves', b'set keep\necho \nshow\nquit\n', P * 2 + b'\n' + P + b'keep\n' + Q),
    ('reject arguments', b'set keep\nshow x\nhelp x\nquit x\nshow\nquit\n', P + U * 3 + P + b'keep\n' + Q),
    ('exact names', b'set keep\nSET x\nsets x\nSHOW\nshows\nshow\nquit\n', P + U * 4 + P + b'keep\n' + Q),
    ('empty command preserves', b'set keep\n\n   \nshow\nquit\n', P + U * 2 + P + b'keep\n' + Q),
    ('tab not separator', b'set\tx\n\tset x\nshow\t\nquit\n', U * 3 + Q),
    ('literal quotes slash tab', b'set "a\\b\tc"\nshow\nquit\n', P * 2 + b'"a\\b\tc"\n' + Q),
    ('CRLF', b'set\r\nhello\r\nshow\r\nquit\r\n', T + P + b'hello\n' + Q),
    ('CR inline', b'se\rt h\ri\r\nshow\nquit\n', P * 2 + b'hi\n' + Q),
    ('CR alone', b'set hi\rquit\nshow\nquit\n', P * 2 + b'hiquit\n' + Q),
    ('CR next line', b'set\na\rb\r\nshow\nquit\n', T + P + b'ab\n' + Q),
    ('NUL inline', b'set ab\0tail\nshow\nquit\n', P * 2 + b'ab\n' + Q),
    ('NUL clear', b'set keep\nset \0tail\nshow\nquit\n', P * 3 + b'\n' + Q),
    ('NUL next line', b'set\nab\0tail\nshow\nquit\n', T + P + b'ab\n' + Q),
    ('NUL name', b'set\0ignored\nhello\nshow\nquit\n', T + P + b'hello\n' + Q),
    ('NUL before name', b'\0set hi\nshow\nquit\n', U + P + b'\n' + Q),
    ('text is not command', b'set\nquit\nshow\nquit\n', T + P + b'quit\n' + Q),
    ('UTF-8', 'set こんにちは\nshow\nquit\n'.encode(), P * 2 + 'こんにちは\n'.encode() + Q),
    ('UTF-8 cut', b'set ' + 'あ'.encode() * 21 + b'\nshow\nquit\n', P * 2 + ('あ'.encode() * 21)[:59] + b'\n' + Q),
    ('nonascii space', 'set　hi\nquit\n'.encode(), U + Q),
    ('last byte separator', b' ' * 59 + b'set x\nshow\nquit\n', P * 2 + b'\n' + Q),
    ('discarded separator', b' ' * 60 + b'set x\nhello\nshow\nquit\n', T + P + b'hello\n' + Q),
    ('spaces fill buffer', b' ' * 63 + b'set hi\nshow\nquit\n', U + P + b'\n' + Q),
    ('empty EOF', b'', E),
    ('partial set EOF', b'set hi', E),
    ('set next line EOF', b'set\n', T + b'input EOF\n'),
    ('set partial text EOF', b'set\nhello', T + b'input EOF\n'),
    ('overflow EOF', b'set ' + b'x' * 4096, E),
    ('text overflow EOF', b'set\n' + b'x' * 4096, T + b'input EOF\n'),
    ('committed then EOF', b'set hi\nshow\n', P * 2 + b'hi\n' + E),
    ('partial replacement EOF', b'set keep\nset\npartial', P + T + b'input EOF\n'),
    ('quit padding', b'  quit   \n', Q),
    ('help', b'help\nquit\n', H + Q),
    ('repeated calls', b'set abcdef\nset x\nshow\nset \nshow\n' * 1000 + b'quit\n',
     (P * 3 + b'x\n' + P * 2 + b'\n') * 1000 + Q),
]
for length in (0, 1, 59, 62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        cases.append((f'inline {length} {ending!r}',
                      b'set ' + b'x' * length + ending + b'show\nquit\n',
                      P * 2 + b'x' * min(length, 59) + b'\n' + Q))
        cases.append((f'next line {length} {ending!r}',
                      b'set\n' + b'x' * length + ending + b'show\nquit\n',
                      T + P + b'x' * min(length, 63) + b'\n' + Q))



MISSING = object()
DIRECTORY = object()
count = 0


def check(name, data, expected, initial=MISSING, final=MISSING, monitor=False):
    global count
    with tempfile.TemporaryDirectory(prefix='line-storage-io-') as directory:
        path = Path(directory) / 'line.txt'
        if initial is DIRECTORY:
            path.mkdir()
        elif initial is not MISSING:
            path.write_bytes(initial)
        args = [VM] if monitor else [VM, PROGRAM]
        r = subprocess.run(args, input=data, capture_output=True, cwd=directory, timeout=10)
        assert (r.returncode, r.stdout, r.stderr) == (0, expected, b''), (name, r, expected)
        if final is MISSING:
            assert not path.exists(), name
        elif final is DIRECTORY:
            assert path.is_dir(), name
        else:
            assert path.read_bytes() == final, (name, path.read_bytes(), final)
    count += 1


# Day 42's 74 direct vectors, with the two approved help lines added.
for name, data, expected in cases:
    check('inherited ' + name, data, expected)
check('inherited monitor and restart',
      PROGRAM.encode() + b'\nset hi\nshow\nquit\n' + PROGRAM.encode() + b'\nshow\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + P * 2 + b'hi\n' + Q + b'>' + P + b'\n' + Q + b'>Goodbye from Handmade VM\n',
      monitor=True)
inherited_count = count
S = P + b'Saved.\n'
L = P + b'Loaded.\n'
SF = P + b'Save failed.\n'
LF = P + b'Load failed.\n'

check('save empty initial line', b'save\nquit\n', S + Q, final=b'')
check('roundtrip replaces shorter', b'set hello\nsave\nset other contents\nload\nshow\nquit\n',
      P + S + P + L + P + b'hello\n' + Q, final=b'hello')
check('overwrite longer file', b'set x\nsave\nshow\nquit\n', P + S + P + b'x\n' + Q,
      initial=b'old long contents', final=b'x')
check('clear then save', b'set hello\nset \nsave\nload\nshow\nquit\n',
      P * 2 + S + L + P + b'\n' + Q, initial=b'old', final=b'')
check('missing load preserves', b'set keep\nload\nshow\nquit\n',
      P + LF + P + b'keep\n' + Q)
check('missing load then save', b'set keep\nload\nsave\nshow\nquit\n',
      P + LF + S + P + b'keep\n' + Q, final=b'keep')
for content in (b'', b'x', b'x' * 62, b'x' * 63, b'  a  b  ', 'こんにちは'.encode(), b'\xff\x80'):
    check('load valid ' + repr(content), b'set old text\nload\nshow\nquit\n',
          P + L + P + content + b'\n' + Q, initial=content, final=content)
for content in (b'x' * 64, b'x' * 65, b'x' * 4096, b'\0', b'a\0b', b'hello\n',
                b'hello\r\n', b'a\rb', b'\n', b'x' * 62 + b'\n', b'x' * 62 + b'\0'):
    check('load invalid ' + repr(content[:65]), b'set keep\nload\nshow\nquit\n',
          P + LF + P + b'keep\n' + Q, initial=content, final=content)
check('directory open errors', b'set keep\nsave\nload\nshow\nquit\n',
      P + SF + LF + P + b'keep\n' + Q, initial=DIRECTORY, final=DIRECTORY)
check('nonempty arguments rejected', b'set keep\nsave x\nload x\nshow\nquit\n',
      P + U * 2 + P + b'keep\n' + Q, initial=b'original', final=b'original')
check('names and tabs rejected', b'SAVE\nLOAD\nsaves\nloads\nsave\tx\nload \t\nquit\n',
      U * 6 + Q, initial=b'original', final=b'original')
check('spaces accepted', b'set hi\n  save   \nset x\n load   \nshow\nquit\n',
      P + S + P + L + P + b'hi\n' + Q, final=b'hi')
check('CRLF and CR in commands', b'set hi\r\nsa\rve\r\nset x\r\nlo\rad\r\nshow\r\nquit\r\n',
      P + S + P + L + P + b'hi\n' + Q, final=b'hi')
check('NUL in save input', b'set ab\0ignored\nsave\nload\nshow\nquit\n',
      P + S + L + P + b'ab\n' + Q, final=b'ab')
check('partial save EOF does not write', b'set keep\nsave', P + E,
      initial=b'original', final=b'original')
check('quit does not save', b'set keep\nquit\n', P + Q, initial=b'original', final=b'original')
check('EOF does not save', b'set keep\n', P + E, initial=b'original', final=b'original')
check('startup does not load', b'show\nquit\n', P + b'\n' + Q, initial=b'original', final=b'original')
for length in (0, 1, 59, 62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        for inline in (True, False):
            content = (b'x' * length)[:59 if inline else 63]
            data = (b'set ' if inline else b'set\n') + b'x' * length + ending
            check(f'save length {length} {ending!r} inline={inline}',
                  data + b'save\nset other\nload\nshow\nquit\n',
                  (P if inline else T) + S + P + L + P + content + b'\n' + Q,
                  final=content)
check('UTF-8 cut is preserved by storage', b'set ' + 'あ'.encode() * 21 + b'\nsave\nset x\nload\nshow\nquit\n',
      P + S + P + L + P + ('あ'.encode() * 21)[:59] + b'\n' + Q,
      final=('あ'.encode() * 21)[:59])
check('help echo unknown preserve loaded text', b'load\nhelp\necho other\nwho\nshow\nquit\n',
      L + H + P + b'other\n' + U + P + b'keep\n' + Q, initial=b'keep', final=b'keep')
check('repeated save load', b'set abcdef\nsave\nset x\nload\nshow\n' * 1000 + b'quit\n',
      (P + S + P + L + P + b'abcdef\n') * 1000 + Q, final=b'abcdef')

# Separate VM processes share the file but start with an empty edit buffer.
with tempfile.TemporaryDirectory(prefix='line-storage-restart-') as directory:
    for data, output in [(b'set persistent\nsave\nquit\n', P + S + Q),
                         (b'show\nload\nshow\nquit\n', P + b'\n' + L + P + b'persistent\n' + Q)]:
        r = subprocess.run([VM, PROGRAM], input=data, cwd=directory, capture_output=True, timeout=5)
        assert (r.returncode, r.stdout, r.stderr) == (0, output, b''), r
        assert (Path(directory) / 'line.txt').read_bytes() == b'persistent'
    count += 1
check('monitor saves and reloads',
      PROGRAM.encode() + b'\nset hi\nsave\nquit\n' + PROGRAM.encode() + b'\nshow\nload\nshow\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + P + S + Q + b'>' + P + b'\n' + L + P + b'hi\n' + Q + b'>Goodbye from Handmade VM\n',
      final=b'hi', monitor=True)

# Reassembly and a source-independent instruction/string walk verify placement.
with tempfile.TemporaryDirectory(prefix='line-storage-layout-') as directory:
    binary = Path(directory) / 'rebuilt.bin'
    r = subprocess.run([ASM, str(Path('programs/line-storage.asm').resolve()), str(binary)], capture_output=True, timeout=5)
    assert (r.returncode, r.stderr) == (0, b''), r
    assert binary.read_bytes() == Path(PROGRAM).read_bytes()
blob = Path(PROGRAM).read_bytes()
# The first Command> string marks the end of the instruction block.
code_size = blob.index(b'Command> \0')
assert code_size % 4 == 0 and 0 < len(blob) <= 0x500
words = struct.unpack('>' + 'I' * (code_size // 4), blob[:code_size])
assert 0x60000004 in words and 0x60000005 in words
strings = [b'Command> ', H[len(P):].split(b'save:')[0], b'Goodbye from Handmade VM\n',
           b'Unknown command\n', b'help', b'quit', b'echo', b'Text> ', b'set', b'show',
           b'save', b'load', b'Saved.\n', b'Save failed.\n', b'Loaded.\n', b'Load failed.\n',
           b'save: write the stored line to line.txt\nload: read the stored line from line.txt\n']
assert blob[code_size:] == b''.join(s + b'\0' for s in strings)
print(f'line-storage: {count} cases passed ({inherited_count} inherited); reassembly byte equality OK')
print(f'line-storage layout: {len(words)} instructions / {code_size} code bytes; {len(strings)} strings / {len(blob)-code_size} data bytes; total {len(blob)} <= 0x500')
