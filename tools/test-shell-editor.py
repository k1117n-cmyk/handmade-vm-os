"""Day 45: fixed output vectors for separate shell/editor and child isolation.

All host files are confined to temporary directories. Expected output is defined
here independently of the assembler sources and of the VM's actual output.
"""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

VM, SHELL, EDITOR, CHILD, ASM = (str(Path(arg).resolve()) for arg in sys.argv[1:6])
S, E = b'Command> ', b'Editor> '
SQ = S + b'Goodbye from Handmade VM\nCPU halted.\n'
EQ = E + b'Goodbye from editor\nCPU halted.\n'
FIN, FAIL = b'Editor finished.\n', b'Editor failed.\n'
SU, EU = S + b'Unknown command\n', E + b'Unknown command\n'
SEOF, EEOF = S + b'input EOF\n', E + b'input EOF\n'
SH = S + (b'Commands:\nhelp: show help\necho [text]: print text or read a line\n'
          b'run: start run-child.bin\nedit: start editor.bin\nquit: quit\n')
EH = E + (b'Commands:\nhelp: show help\nset [text]: replace the stored line or read a line\n'
          b'show: print the stored line\nsave: write the stored line to line.txt\n'
          b'load: read the stored line from line.txt\nquit: return to shell\n')
T = E + b'Text> '
SAVED, LOADED = E + b'Saved.\n', E + b'Loaded.\n'
SAVE_FAIL, LOAD_FAIL = E + b'Save failed.\n', E + b'Load failed.\n'
MISSING, DIRECTORY = object(), object()
editor_blob, child_blob = Path(EDITOR).read_bytes(), Path(CHILD).read_bytes()
counts = {'shell': 0, 'editor direct': 0, 'editor via shell': 0, 'launch': 0}


def words(*instructions):
    return struct.pack('>' + 'I' * len(instructions), *instructions)


def assemble(source):
    with tempfile.TemporaryDirectory(prefix='editor-test-asm-') as directory:
        source_path, binary = Path(directory) / 'child.asm', Path(directory) / 'child.bin'
        source_path.write_text(source)
        r = subprocess.run([ASM, str(source_path), str(binary)], capture_output=True, timeout=5)
        assert (r.returncode, r.stderr) == (0, b''), r
        return binary.read_bytes()


def check(name, data, expected, *, group='shell', target=SHELL, editor=editor_blob,
          child=child_blob, initial=MISSING, final=MISSING, monitor=False):
    with tempfile.TemporaryDirectory(prefix='shell-editor-io-') as directory:
        root = Path(directory)
        (root / 'programs').mkdir()
        binaries = [('editor.bin', editor), ('run-child.bin', child)]
        for name_bin, content in binaries:
            path = root / 'programs' / name_bin
            if content is DIRECTORY:
                path.mkdir()
            elif content is not MISSING:
                path.write_bytes(content)
        line = root / 'line.txt'
        if initial is DIRECTORY:
            line.mkdir()
        elif initial is not MISSING:
            line.write_bytes(initial)
        r = subprocess.run([VM] if monitor else [VM, target], input=data,
                           cwd=root, capture_output=True, timeout=20)
        assert (r.returncode, r.stdout, r.stderr) == (0, expected, b''), (name, r, expected)
        if final is MISSING:
            assert not line.exists(), name
        elif final is DIRECTORY:
            assert line.is_dir(), name
        else:
            assert line.read_bytes() == final, (name, line.read_bytes(), final)
        for name_bin, content in binaries:
            path = root / 'programs' / name_bin
            if content is MISSING:
                assert not path.exists(), name
            elif content is DIRECTORY:
                assert path.is_dir(), name
            else:
                assert path.read_bytes() == content, name
    counts[group] += 1
    print(f'{group}: {name} OK')


def check_editor(name, data, expected, *, initial=MISSING, final=MISSING, eof=False):
    check(name, data, expected, group='editor direct', target=EDITOR, initial=initial, final=final)
    check(name, b'edit\n' + data + (b'' if eof else b'quit\n'),
          S + expected + (FAIL + SEOF if eof else FIN + SQ),
          group='editor via shell', initial=initial, final=final)


shell_vectors = [
    ('help', b'help\nquit\n', SH + SQ),
    ('inline echo', b'echo hello\nquit\n', S + b'hello\n' + SQ),
    ('bare echo', b'echo\n  hello  \nquit\n', S + b'Text>   hello  \n' + SQ),
    ('empty argument', b'echo \nquit\n', S + b'\n' + SQ),
    ('spaces', b'  echo   a  b  \n  help   \n  quit   \n', S + b'a  b  \n' + SH + SQ),
    ('quotes backslash tab literal', b'echo "a\\b\tc"\nquit\n', S + b'"a\\b\tc"\n' + SQ),
    ('editor commands refused', b'set hi\nshow\nsave\nload\nquit\n', SU * 4 + SQ),
    ('arguments rejected', b'help x\nquit x\nrun x\nedit x\nquit\n', SU * 4 + SQ),
    ('exact command names', b'EDIT\nedits\nRUN\nruns\nHELP\nquits\nquit\n', SU * 6 + SQ),
    ('tabs not separators', b'edit\tx\n\tedit\nedit \t\nquit\n', SU * 3 + SQ),
    ('empty commands', b'\n   \nquit\n', SU * 2 + SQ),
    ('CRLF and embedded CR', b'ec\rho h\ri\r\nquit\r\n', S + b'hi\n' + SQ),
    ('CR alone does not terminate', b'echo hi\rquit\nquit\n', S + b'hiquit\n' + SQ),
    ('NUL argument', b'echo ab\0tail\nquit\n', S + b'ab\n' + SQ),
    ('NUL name', b'echo\0tail\nquit\nquit\n', S + b'Text> quit\n' + SQ),
    ('UTF-8', 'echo こんにちは\nquit\n'.encode(), S + 'こんにちは\n'.encode() + SQ),
    ('UTF-8 truncation', b'echo ' + 'あ'.encode() * 22 + b'\nquit\n',
     S + ('あ'.encode() * 22)[:58] + b'\n' + SQ),
    ('empty EOF', b'', SEOF),
    ('partial edit EOF', b'edit', SEOF),
    ('echo partial text EOF', b'echo\npartial', S + b'Text> input EOF\n'),
    ('overlong command discarded', b'x' * 4096 + b'\nquit\n', SU + SQ),
]
for name, data, output in shell_vectors:
    check(name, data, output)
for length in (0, 1, 58, 62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        check(f'echo inline {length} {ending!r}', b'echo ' + b'x' * length + ending + b'quit\n',
              S + b'x' * min(length, 58) + b'\n' + SQ)
        check(f'echo next line {length} {ending!r}', b'echo\n' + b'x' * length + ending + b'quit\n',
              S + b'Text> ' + b'x' * min(length, 63) + b'\n' + SQ)

editor_vectors = [
    ('initial empty', b'show\nquit\n', E + b'\n' + EQ),
    ('inline set', b'set hello\nshow\nquit\n', E * 2 + b'hello\n' + EQ),
    ('replace shorter', b'set abcdef\nset x\nshow\nquit\n', E * 3 + b'x\n' + EQ),
    ('clear inline', b'set abc\nset \nshow\nquit\n', E * 3 + b'\n' + EQ),
    ('clear next line', b'set abc\nset\n\nshow\nquit\n', E + T + E + b'\n' + EQ),
    ('next line spaces', b'set\n  hello  \nshow\nquit\n', T + E + b'  hello  \n' + EQ),
    ('inline spaces', b'  set   a  b  \n  show   \nquit\n', E * 2 + b'a  b  \n' + EQ),
    ('separator only', b'set keep\nset    \nshow\nquit\n', E * 3 + b'\n' + EQ),
    ('help', b'help\nquit\n', EH + EQ),
    ('help and unknown preserve', b'set keep\nhelp\nwho\nshow\nquit\n', E + EH + EU + E + b'keep\n' + EQ),
    ('shell commands refused', b'set keep\necho hi\nrun\nedit\nshow\nquit\n', E + EU * 3 + E + b'keep\n' + EQ),
    ('arguments rejected', b'set keep\nhelp x\nshow x\nsave x\nload x\nquit x\nshow\nquit\n',
     E + EU * 5 + E + b'keep\n' + EQ),
    ('exact names', b'SET x\nsets x\nSHOW\nsaves\nloads\nquit\n', EU * 5 + EQ),
    ('tabs refused', b'set\tx\n\tset x\nshow\t\nquit\n', EU * 3 + EQ),
    ('empty commands preserve', b'set keep\n\n   \nshow\nquit\n', E + EU * 2 + E + b'keep\n' + EQ),
    ('literal quotes slash tab', b'set "a\\b\tc"\nshow\nquit\n', E * 2 + b'"a\\b\tc"\n' + EQ),
    ('CRLF and embedded CR', b'se\rt h\ri\r\nshow\r\nquit\r\n', E * 2 + b'hi\n' + EQ),
    ('CR alone', b'set hi\rquit\nshow\nquit\n', E * 2 + b'hiquit\n' + EQ),
    ('CR next line', b'set\na\rb\r\nshow\nquit\n', T + E + b'ab\n' + EQ),
    ('NUL inline', b'set ab\0tail\nshow\nquit\n', E * 2 + b'ab\n' + EQ),
    ('NUL next line', b'set\nab\0tail\nshow\nquit\n', T + E + b'ab\n' + EQ),
    ('NUL name', b'set\0tail\nhello\nshow\nquit\n', T + E + b'hello\n' + EQ),
    ('NUL before name', b'\0set hi\nshow\nquit\n', EU + E + b'\n' + EQ),
    ('text is not command', b'set\nquit\nshow\nquit\n', T + E + b'quit\n' + EQ),
    ('UTF-8', 'set こんにちは\nshow\nquit\n'.encode(), E * 2 + 'こんにちは\n'.encode() + EQ),
    ('UTF-8 truncation', b'set ' + 'あ'.encode() * 21 + b'\nshow\nquit\n',
     E * 2 + ('あ'.encode() * 21)[:59] + b'\n' + EQ),
    ('last byte separator', b' ' * 59 + b'set x\nshow\nquit\n', E * 2 + b'\n' + EQ),
    ('discarded separator', b' ' * 60 + b'set x\nhello\nshow\nquit\n', T + E + b'hello\n' + EQ),
    ('space-filled input', b' ' * 63 + b'set hi\nshow\nquit\n', EU + E + b'\n' + EQ),
    ('quit padding', b'  quit   \n', EQ),
    ('repeated copy calls', b'set abcdef\nset x\nshow\nset \nshow\n' * 1000 + b'quit\n',
     (E * 3 + b'x\n' + E * 2 + b'\n') * 1000 + EQ),
]
for name, data, output in editor_vectors:
    check_editor(name, data, output)
for name, data, output in [
    ('empty EOF', b'', EEOF),
    ('partial command EOF', b'set hello', EEOF),
    ('next line EOF', b'set\n', T + b'input EOF\n'),
    ('partial replacement EOF', b'set keep\nset\npartial', E + T + b'input EOF\n'),
    ('overflow EOF', b'set ' + b'x' * 4096, EEOF),
    ('text overflow EOF', b'set\n' + b'x' * 4096, T + b'input EOF\n'),
]:
    check_editor(name, data, output, eof=True)
for length in (0, 1, 59, 62, 63, 64, 4096):
    for ending in (b'\n', b'\r\n'):
        for inline in (True, False):
            content = b'x' * min(length, 59 if inline else 63)
            data = (b'set ' if inline else b'set\n') + b'x' * length + ending
            check_editor(f'set/save/load {length} {ending!r} inline={inline}',
                         data + b'save\nset other\nload\nshow\nquit\n',
                         (E if inline else T) + SAVED + E + LOADED + E + content + b'\n' + EQ,
                         final=content)

check_editor('save initial empty', b'save\nquit\n', SAVED + EQ, final=b'')
check_editor('overwrite longer file', b'set x\nsave\nquit\n', E + SAVED + EQ,
             initial=b'old longer contents', final=b'x')
check_editor('load missing preserves', b'set keep\nload\nshow\nquit\n', E + LOAD_FAIL + E + b'keep\n' + EQ)
check_editor('save/load failures continue', b'set keep\nsave\nload\nshow\nquit\n',
             E + SAVE_FAIL + LOAD_FAIL + E + b'keep\n' + EQ, initial=DIRECTORY, final=DIRECTORY)
check_editor('no automatic load/save', b'show\nset unsaved\nquit\n', E + b'\n' + E + EQ,
             initial=b'original', final=b'original')
check_editor('EOF does not save', b'set unsaved\n', E + EEOF,
             initial=b'original', final=b'original', eof=True)
check_editor('partial save EOF', b'set unsaved\nsave', E + EEOF,
             initial=b'original', final=b'original', eof=True)
check_editor('storage padding', b'set hi\n save   \nset x\n load   \nshow\nquit\n',
             E + SAVED + E + LOADED + E + b'hi\n' + EQ, final=b'hi')
check_editor('storage CRLF', b'set hi\r\nsa\rve\r\nset x\r\nlo\rad\r\nshow\r\nquit\r\n',
             E + SAVED + E + LOADED + E + b'hi\n' + EQ, final=b'hi')
check_editor('save NUL prefix only', b'set ab\0tail\nsave\nquit\n', E + SAVED + EQ, final=b'ab')
for content in (b'', b'x', b'x' * 62, b'x' * 63, b'  a  b  ', 'こんにちは'.encode(), b'\xff\x80'):
    check_editor(f'valid file {content[:64]!r}', b'set other\nload\nshow\nquit\n',
                 E + LOADED + E + content + b'\n' + EQ, initial=content, final=content)
for content in (b'x' * 64, b'x' * 65, b'x' * 4096, b'\0', b'a\0b', b'hello\n', b'hello\r\n',
                b'a\rb', b'\n', b'x' * 62 + b'\n', b'x' * 62 + b'\0'):
    check_editor(f'invalid file {content[:65]!r}', b'set keep\nload\nshow\nquit\n',
                 E + LOAD_FAIL + E + b'keep\n' + EQ, initial=content, final=content)
check_editor('repeated storage', b'set abcdef\nsave\nset x\nload\nshow\n' * 1000 + b'quit\n',
             (E + SAVED + E + LOADED + E + b'abcdef\n') * 1000 + EQ, final=b'abcdef')

# End-to-end ownership, explicit persistence, independent runs, and input sharing.
check('edit/save/reopen/load and shell echo',
      b'edit\nset hello\nshow\nsave\nquit\necho back\nedit\nshow\nload\nshow\nquit\nquit\n',
      S + E * 2 + b'hello\n' + SAVED + EQ + FIN + S + b'back\n' + S + E + b'\n' +
      LOADED + E + b'hello\n' + EQ + FIN + SQ, group='launch', final=b'hello')
check('unsaved child not reused', b'edit\nset lost\nquit\nedit\nshow\nquit\nquit\n',
      S + E + EQ + FIN + S + E + b'\n' + EQ + FIN + SQ, group='launch')
check('run and edit select distinct files', b'run\nedit\nquit\nrun\nquit\n',
      (S + b'A\nCPU halted.\nRun finished.\n') + S + EQ + FIN +
      S + b'A\nCPU halted.\nRun finished.\n' + SQ, group='launch')
check('spaces CR and NUL edit', b'  ed\rit  \r\nquit\nedit\0ignored\nquit\nquit\n',
      (S + EQ + FIN) * 2 + SQ, group='launch')
check('1000 fresh editor sessions', b'edit\nshow\nset lost\nquit\n' * 1000 + b'quit\n',
      (S + E + b'\n' + E + EQ + FIN) * 1000 + SQ, group='launch')
for editor, name in [(MISSING, 'missing editor'), (DIRECTORY, 'directory editor')]:
    check(name, b'edit\necho back\nquit\n', S + FAIL + S + b'back\n' + SQ,
          group='launch', editor=editor)
for length in range(4):
    check(f'reject short editor {length}', b'edit\nquit\n', S + FAIL + SQ,
          group='launch', editor=b'\0' * length)
for length in (4, 1024 * 1024 - 1, 1024 * 1024):
    check(f'accept editor {length}', b'edit\nquit\n', S + b'CPU halted.\n' + FIN + SQ,
          group='launch', editor=words(0x01000000) + b'\0' * (length - 4))
check('oversize editor rejected', b'edit\nquit\n', S + FAIL + SQ,
      group='launch', editor=words(0x01000000) + b'\0' * (1024 * 1024 - 3))
for name, binary, diagnostic in [
    ('invalid instruction', words(0xFF000000), b'unknown instruction: FF000000\n'),
    ('stack underflow', words(0x02000000), b'stack underflow\n'),
    ('out of range write', words(0x401FFFFF, 0x20100000, 0x31100000), b'memory address out of range: 0x00100000\n'),
    ('unimplemented 7 retained', words(0x60000007), b'unimplemented syscall: 7\n'),
]:
    check(name, b'edit\necho back\nquit\n', S + diagnostic + FAIL + S + b'back\n' + SQ,
          group='launch', editor=binary)
check('out of range fetch', b'edit\necho back\nquit\n',
      S + b'pc out of range: 0x00100000\n' + FAIL + S + b'back\n' + SQ,
      group='launch', editor=words(0x680FFFFC) + b'\0' * (1024 * 1024 - 8) + words(0x40000000))
check('child consumes shared input', b'edit\nXecho back\nquit\n',
      S + b'XCPU halted.\n' + FIN + S + b'back\n' + SQ, group='launch',
      editor=words(0x60000002, 0x60000003, 0x01000000))
check('child EOF before shell EOF', b'edit\n', S + b'input EOF\n' + FAIL + SEOF,
      group='launch', editor=words(0x60000002, 0x01000000))
for number in (6, 8):
    nested = assemble(f'''SYSCALL {number}
MOVI R1, 1
CMP R0, R1
JNZ failed
HALT
failed: SYSCALL 7
''')
    check(f'editor child rejects nested {number}', b'edit\nquit\n', S + b'CPU halted.\n' + FIN + SQ,
          group='launch', editor=nested)
    check(f'run child rejects nested {number}', b'run\nquit\n', S + b'CPU halted.\nRun finished.\n' + SQ,
          group='launch', child=nested)
# Verify register/memory/flag initialization over repeated actual launches.
fresh = 'JZ failed\n'
for register in range(1, 8):
    fresh += f'CMP R0, R{register}\nJNZ failed\n'
fresh += '''MOVI R1, 0x600
LDB R0, [R1]
MOVI R2, 0
CMP R0, R2
JNZ failed
MOVI R0, 88
STB [R1], R0
CALLI finish
finish:
CMP R0, R0
HALT
failed: SYSCALL 7
'''
check('1000 initialized child VM states', b'edit\n' * 1000 + b'quit\n',
      (S + b'CPU halted.\n' + FIN) * 1000 + SQ, group='launch', editor=assemble(fresh))
check('monitor return and shell restart',
      SHELL.encode() + b'\nedit\nset hi\nsave\nquit\nquit\n' +
      SHELL.encode() + b'\nedit\nshow\nload\nshow\nquit\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + S + E + SAVED + EQ + FIN + SQ + b'>' +
      S + E + b'\n' + LOADED + E + b'hi\n' + EQ + FIN + SQ + b'>Goodbye from Handmade VM\n',
      group='launch', monitor=True, final=b'hi')

# Independent data/instruction walk and reassembly, including the old run sample.
with tempfile.TemporaryDirectory(prefix='shell-editor-layout-') as directory:
    for source, binary in [('programs/shell.asm', SHELL), ('programs/editor.asm', EDITOR),
                           ('programs/run-child.asm', CHILD)]:
        rebuilt = Path(directory) / Path(binary).name
        r = subprocess.run([ASM, source, str(rebuilt)], capture_output=True, timeout=5)
        assert (r.returncode, r.stderr) == (0, b''), r
        assert rebuilt.read_bytes() == Path(binary).read_bytes(), source
assert child_blob == words(0x40000041, 0x60000000, 0x01000000)
string_sets = [
    (SHELL, [S, SH[len(S):], b'Goodbye from Handmade VM\n', b'Unknown command\n',
             b'help', b'quit', b'echo', b'Text> ', b'run', b'edit', b'Run finished.\n',
             b'Run failed.\n', FIN, FAIL], {6, 8}, {4, 5}),
    (EDITOR, [E, EH[len(E):], b'Goodbye from editor\n', b'Unknown command\n',
              b'help', b'quit', b'Text> ', b'set', b'show', b'save', b'load',
              b'Saved.\n', b'Save failed.\n', b'Loaded.\n', b'Load failed.\n'], {4, 5}, {6, 8}),
]
for binary, strings, required, forbidden in string_sets:
    blob = Path(binary).read_bytes()
    code_size = blob.index(strings[0] + b'\0')
    assert code_size % 4 == 0 and 0 < len(blob) <= 0x500
    instructions = struct.unpack('>' + 'I' * (code_size // 4), blob[:code_size])
    assert blob[code_size:] == b''.join(s + b'\0' for s in strings), binary
    for number in required:
        assert instructions.count(0x60000000 | number) == 1, binary
    for number in forbidden:
        assert (0x60000000 | number) not in instructions, binary
    print(f'{Path(binary).name} layout: {code_size // 4} instructions / {code_size} code bytes; '
          f'{len(strings)} strings / {len(blob) - code_size} data bytes; total {len(blob)} <= 0x500')
print('shell/editor/old child reassembly byte equality OK; old child 3 instructions / 12 bytes')
for group, count in counts.items():
    print(f'shell-editor {group}: {count} cases passed')
print(f'shell-editor I/O: {sum(counts.values())} cases passed')
