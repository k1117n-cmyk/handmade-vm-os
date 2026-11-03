"""Day 41: complete VM output for command splitting and inherited line input."""
from pathlib import Path
import subprocess
import sys

VM = sys.argv[1]
PROGRAM = sys.argv[2]
PROMPT = b'Command> '
HELP = PROMPT + b'Commands:\nhelp: show help\nquit: quit\necho [text]: print text or read a line\n'
QUIT = PROMPT + b'Goodbye from Handmade VM\nCPU halted.\n'
UNKNOWN = PROMPT + b'Unknown command\n'
EOF = PROMPT + b'input EOF\n'
TEXT = PROMPT + b'Text> '
cases = [
    ('inline', b'echo hello\nquit\n', PROMPT + b'hello\n' + QUIT),
    ('bare echo', b'echo\nhello\nquit\n', TEXT + b'hello\n' + QUIT),
    ('leading spaces', b'   echo hi\nquit\n', PROMPT + b'hi\n' + QUIT),
    ('leading bare echo', b'  echo\nhello\nquit\n', TEXT + b'hello\n' + QUIT),
    ('separators', b'echo   hi\nquit\n', PROMPT + b'hi\n' + QUIT),
    ('internal and trailing spaces', b'echo a  b  \nquit\n', PROMPT + b'a  b  \n' + QUIT),
    ('empty argument', b'echo \nhelp\nquit\n', PROMPT + b'\n' + HELP + QUIT),
    ('empty repeated separators', b'  echo    \nquit\n', PROMPT + b'\n' + QUIT),
    ('empty text line', b'echo\n\nquit\n', TEXT + b'\n' + QUIT),
    ('text spaces unchanged', b'echo\n  hi  \nquit\n', TEXT + b'  hi  \n' + QUIT),
    ('help padding', b'  help   \nquit\n', HELP + QUIT),
    ('quit padding', b'  quit   \n', QUIT),
    ('reject help argument', b'help x\nquit\n', UNKNOWN + QUIT),
    ('reject quit argument', b'quit x\nhelp\nquit\n', UNKNOWN + HELP + QUIT),
    ('empty command', b'\nquit\n', UNKNOWN + QUIT),
    ('spaces only', b'     \nquit\n', UNKNOWN + QUIT),
    ('exact name', b'ECHO hi\nechoes hi\nech hi\nquits\nHELP\nquit\n', UNKNOWN * 5 + QUIT),
    ('tab not separator', b'echo\thi\n\techo hi\nhelp\t\nquit\t\nquit\n', UNKNOWN * 4 + QUIT),
    ('tab in argument', b'echo \thi\nhelp \t\nquit \t\nquit\n', PROMPT + b'\thi\n' + UNKNOWN * 2 + QUIT),
    ('quotes literal', b'echo "a b"\nquit\n', PROMPT + b'"a b"\n' + QUIT),
    ('backslash literal', b'echo a\\nb\nquit\n', PROMPT + b'a\\nb\n' + QUIT),
    ('command words in text', b'echo quit\necho help\necho\nquit\nquit\n', PROMPT + b'quit\n' + PROMPT + b'help\n' + TEXT + b'quit\n' + QUIT),
    ('shorter next input', b'echo abcdef\necho x\necho \nhelp\nquit\n', PROMPT + b'abcdef\n' + PROMPT + b'x\n' + PROMPT + b'\n' + HELP + QUIT),
    ('CRLF', b'echo hi\r\necho\r\nhello\r\nquit\r\n', PROMPT + b'hi\n' + TEXT + b'hello\n' + QUIT),
    ('CR inside input', b'ec\rho h\ri\r\nquit\n', PROMPT + b'hi\n' + QUIT),
    ('CR alone not terminator', b'echo hi\rquit\nquit\n', PROMPT + b'hiquit\n' + QUIT),
    ('NUL in argument', b'echo ab\0quit\nquit\n', PROMPT + b'ab\n' + QUIT),
    ('NUL after separator', b'echo \0quit\nhelp\nquit\n', PROMPT + b'\n' + HELP + QUIT),
    ('NUL in command', b'echo\0 ignored\nhello\nquit\n', TEXT + b'hello\n' + QUIT),
    ('NUL before separator', b'help\0 x\nquit\n', HELP + QUIT),
    ('NUL text line', b'echo\nab\0tail\nquit\n', TEXT + b'ab\n' + QUIT),
    ('NUL empty command', b'\0echo hi\nquit\n', UNKNOWN + QUIT),
    ('UTF-8', 'echo こんにちは\nquit\n'.encode(), PROMPT + 'こんにちは\n'.encode() + QUIT),
    ('nonascii space', 'echo　hi\nquit\n'.encode(), UNKNOWN + QUIT),
    ('empty EOF', b'', EOF),
    ('partial inline EOF', b'echo hi', EOF),
    ('partial CR EOF', b'echo hi\r', EOF),
    ('empty text EOF', b'echo\n', TEXT + b'input EOF\n'),
    ('partial text EOF', b'echo\nhello', TEXT + b'input EOF\n'),
    ('inline then EOF', b'echo hi\n', PROMPT + b'hi\n' + EOF),
    ('empty argument then EOF', b'echo \n', PROMPT + b'\n' + EOF),
    ('overflow EOF', b'echo ' + b'x' * 4096, EOF),
    ('text overflow EOF', b'echo\n' + b'x' * 4096, TEXT + b'input EOF\n'),
    ('separator at last byte', b' ' * 58 + b'echo x\nquit\n', PROMPT + b'\n' + QUIT),
    ('separator discarded', b' ' * 59 + b'echo x\nhello\nquit\n', TEXT + b'hello\n' + QUIT),
    ('spaces fill buffer', b' ' * 63 + b'quit\nquit\n', UNKNOWN + QUIT),
    ('discard command tail', b'echo ' + b'x' * 58 + b'quithelp\nhelp\nquit\n', PROMPT + b'x' * 58 + b'\n' + HELP + QUIT),
    ('UTF-8 cut', b'echo ' + 'あ'.encode() * 20 + b'\nquit\n', PROMPT + ('あ'.encode() * 20)[:58] + b'\n' + QUIT),
    ('many loops', b'echo a\necho \nhelp\n' * 1000 + b'quit\n', (PROMPT + b'a\n' + PROMPT + b'\n' + HELP) * 1000 + QUIT),
]
for length in (62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        cases.append((f'inline length {length} {ending!r}',
                      b'echo ' + b'x' * (length - 5) + ending + b'quit\n',
                      PROMPT + b'x' * min(length - 5, 58) + b'\n' + QUIT))
        cases.append((f'text length {length} {ending!r}',
                      b'echo\n' + b'x' * length + ending + b'quit\n',
                      TEXT + b'x' * min(length, 63) + b'\n' + QUIT))
        cases.append((f'unknown length {length} {ending!r}',
                      b'x' * length + ending + b'quit\n', UNKNOWN + QUIT))


def check(name, args, data, expected):
    result = subprocess.run(args, input=data, capture_output=True, timeout=5)
    assert (result.returncode, result.stdout, result.stderr) == (0, expected, b''), (name, result, expected)


assert 0 < Path(PROGRAM).stat().st_size <= 0x500, 'program overlaps input buffer'
for name, data, expected in cases:
    check(name, [VM, PROGRAM], data, expected)
check('monitor', [VM], PROGRAM.encode() + b'\necho hi\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + PROMPT + b'hi\n' + QUIT + b'>Goodbye from Handmade VM\n')
print(f'command-arguments: {len(cases) + 1} cases passed; {Path(PROGRAM).stat().st_size} bytes <= 0x500')
