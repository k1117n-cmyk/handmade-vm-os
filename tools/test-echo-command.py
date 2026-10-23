"""Day 36: run the real VM and check complete output, including line boundaries."""
import subprocess
import sys

VM = sys.argv[1] if len(sys.argv) > 1 else './handmade-vm'
PROGRAM = 'programs/echo-command.bin'
HELP = b'Command> Commands:\nhelp: show help\nquit: quit\necho: read and print a line\n'
UNKNOWN = b'Command> Unknown command\n'
QUIT = b'Command> Goodbye from Handmade VM\nCPU halted.\n'
EOF_OUTPUT = b'Command> input EOF\n'

cases = [
    ('quit', b'quit\n', QUIT),
    ('loop', b'help\nhello\nquit\n', HELP + UNKNOWN + QUIT),
    ('CRLF', b'help\r\nhello\r\nquit\r\n', HELP + UNKNOWN + QUIT),
    ('empty', b'\nquit\n', UNKNOWN + QUIT),
    ('empty CRLF', b'\r\nquit\r\n', UNKNOWN + QUIT),
    ('exact match', b'hel\nhelpful\nquits\nHELP\nquit\n', UNKNOWN * 4 + QUIT),
    ('shorter next line', b'helpful\nhelp\nq\nquit\n', UNKNOWN + HELP + UNKNOWN + QUIT),
    ('bare CR is ignored', b'help\rquit\nquit\n', UNKNOWN + QUIT),
    ('CR inside line', b'he\rlp\nquit\n', HELP + QUIT),
    ('empty EOF', b'', EOF_OUTPUT),
    ('partial EOF', b'help', EOF_OUTPUT),
    ('EOF after line', b'help\n', HELP + EOF_OUTPUT),
    ('EOF after CR', b'help\r', EOF_OUTPUT),
    ('EOF at limit', b'x' * 63, EOF_OUTPUT),
    ('EOF while discarding', b'x' * 100, EOF_OUTPUT),
    ('many calls', b'help\n' * 1000 + b'quit\n', HELP * 1000 + QUIT),
]
for length in (62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        cases.append((f'length {length} / {ending!r}',
                      b'x' * length + ending + b'help\nquit\n',
                      UNKNOWN + HELP + QUIT))
cases.append(('discard command-like tail', b'x' * 63 + b'quithelp\nquit\n', UNKNOWN + QUIT))


ECHO = b'Command> Text> '
cases.extend([
    ('echo line', b'echo\nhello\nquit\n', ECHO + b'hello\n' + QUIT),
    ('echo then help and unknown', b'echo\nhello\nhelp\nx\nquit\n', ECHO + b'hello\n' + HELP + UNKNOWN + QUIT),
    ('text is not a command', b'echo\nquit\necho\nhelp\necho\necho\nquit\n', ECHO + b'quit\n' + ECHO + b'help\n' + ECHO + b'echo\n' + QUIT),
    ('empty text', b'echo\n\nquit\n', ECHO + b'\n' + QUIT),
    ('spaces preserved', b'echo\n  hello world  \nquit\n', ECHO + b'  hello world  \n' + QUIT),
    ('no arguments or normalization', b'echo hello\n echo\necho \nECHO\nechoes\nec\nquit\n', UNKNOWN * 6 + QUIT),
    ('shorter text', b'echo\nabcdef\necho\na\necho\n\nquit\n', ECHO + b'abcdef\n' + ECHO + b'a\n' + ECHO + b'\n' + QUIT),
    ('echo CRLF', b'echo\r\nhello\r\nquit\r\n', ECHO + b'hello\n' + QUIT),
    ('text CR ignored', b'echo\nhe\rllo\rworld\nquit\n', ECHO + b'helloworld\n' + QUIT),
    ('text NUL', b'echo\nab\x00quit\nquit\n', ECHO + b'ab\n' + QUIT),
    ('command NUL', b'echo\x00tail\nhello\nquit\n', ECHO + b'hello\n' + QUIT),
    ('UTF-8 text', 'echo\nこんにちは\nquit\n'.encode(), ECHO + 'こんにちは\n'.encode() + QUIT),
    ('EOF before text', b'echo\n', ECHO + b'input EOF\n'),
    ('EOF during text', b'echo\nhello', ECHO + b'input EOF\n'),
    ('EOF after text CR', b'echo\nhello\r', ECHO + b'input EOF\n'),
    ('EOF at text limit', b'echo\n' + b'x' * 63, ECHO + b'input EOF\n'),
    ('EOF discarding text', b'echo\n' + b'x' * 4096, ECHO + b'input EOF\n'),
    ('EOF after text', b'echo\nhello\n', ECHO + b'hello\n' + EOF_OUTPUT),
    ('many echo calls', b'echo\nhello\n' * 1000 + b'quit\n', (ECHO + b'hello\n') * 1000 + QUIT),
    ('discard text tail', b'echo\n' + b'x' * 63 + b'quithelp\nhelp\nquit\n', ECHO + b'x' * 63 + b'\n' + HELP + QUIT),
])
for length in (62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        cases.append((f'text length {length} / {ending!r}',
                      b'echo\n' + b'x' * length + ending + b'echo\na\nquit\n',
                      ECHO + b'x' * min(length, 63) + b'\n' + ECHO + b'a\n' + QUIT))


def check(name, args, data, expected):
    result = subprocess.run(args, input=data, capture_output=True, timeout=5)
    assert (result.returncode, result.stdout, result.stderr) == (0, expected, b''), (
        name, result.returncode, result.stdout, result.stderr, expected)


for name, data, expected in cases:
    check(name, [VM, PROGRAM], data, expected)
check('monitor', [VM],
      PROGRAM.encode() + b'\necho\r\nquit\r\nhelp\r\nquit\r\nquit\n',
      b'Welcome to Handmade VM\n>' + ECHO + b'quit\n' + HELP + QUIT + b'>Goodbye from Handmade VM\n')
print(f'echo-command: {len(cases) + 1} cases passed')
