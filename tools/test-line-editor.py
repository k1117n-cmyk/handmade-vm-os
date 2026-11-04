"""Compare complete VM output for the Day 42 stored-line commands."""
from pathlib import Path
import subprocess
import sys

VM, PROGRAM = sys.argv[1:3]
P = b'Command> '
T = P + b'Text> '
Q = P + b'Goodbye from Handmade VM\nCPU halted.\n'
U = P + b'Unknown command\n'
E = P + b'input EOF\n'
H = P + (b'Commands:\nhelp: show help\nquit: quit\n'
         b'echo [text]: print text or read a line\n'
         b'set [text]: replace the stored line or read a line\n'
         b'show: print the stored line\n')
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


def check(name, args, data, expected):
    r = subprocess.run(args, input=data, capture_output=True, timeout=5)
    assert (r.returncode, r.stdout, r.stderr) == (0, expected, b''), (name, r, expected)


assert 0 < Path(PROGRAM).stat().st_size <= 0x500, 'program overlaps input buffer'
for name, data, expected in cases:
    check(name, [VM, PROGRAM], data, expected)
check('monitor and restart', [VM],
      PROGRAM.encode() + b'\nset hi\nshow\nquit\n' + PROGRAM.encode() + b'\nshow\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + P * 2 + b'hi\n' + Q + b'>' + P + b'\n' + Q + b'>Goodbye from Handmade VM\n')
print(f'line-editor: {len(cases) + 1} cases passed; {Path(PROGRAM).stat().st_size} bytes <= 0x500')
