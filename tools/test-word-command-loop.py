"""Day 35: run the real VM and check complete output, including line boundaries."""
import subprocess
import sys

VM = sys.argv[1] if len(sys.argv) > 1 else './handmade-vm'
PROGRAM = 'programs/word-command-loop.bin'
HELP = b'Command> Commands:\nhelp: show help\nquit: quit\n'
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


def check(name, args, data, expected):
    result = subprocess.run(args, input=data, capture_output=True, timeout=5)
    assert (result.returncode, result.stdout, result.stderr) == (0, expected, b''), (
        name, result.returncode, result.stdout, result.stderr, expected)


for name, data, expected in cases:
    check(name, [VM, PROGRAM], data, expected)
check('monitor', [VM],
      PROGRAM.encode() + b'\nhelp\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + HELP + QUIT + b'>Goodbye from Handmade VM\n')
print(f'word-command-loop: {len(cases) + 1} cases passed')
