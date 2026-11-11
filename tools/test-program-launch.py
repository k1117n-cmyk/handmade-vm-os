"""Day 44: inherited storage, child execution, shared I/O and parent return."""
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

VM, PARENT, CHILD, ASM = (str(Path(arg).resolve()) for arg in sys.argv[1:5])
P = b'Command> '
Q = P + b'Goodbye from Handmade VM\nCPU halted.\n'
SUCCESS = b'A\nCPU halted.\nRun finished.\n'
FAILURE = b'Run failed.\n'
MISSING = object()
DIRECTORY = object()
count = 0

# Run the same 142 storage/editing vectors with the one approved extra help line.
subprocess.run([sys.executable, str(Path(__file__).with_name('test-line-storage.py')),
                VM, PARENT, ASM, '--launcher'], check=True)


def words(*instructions):
    return struct.pack('>' + 'I' * len(instructions), *instructions)


def assemble(source):
    with tempfile.TemporaryDirectory(prefix='launch-asm-') as directory:
        source_path = Path(directory) / 'child.asm'
        binary_path = Path(directory) / 'child.bin'
        source_path.write_text(source)
        r = subprocess.run([ASM, str(source_path), str(binary_path)], capture_output=True, timeout=5)
        assert r.returncode == 0 and not r.stderr, r
        return binary_path.read_bytes()


def check(name, data, expected, child=MISSING, initial=MISSING, final=MISSING, monitor=False):
    global count
    with tempfile.TemporaryDirectory(prefix='launch-io-') as directory:
        root = Path(directory)
        (root / 'programs').mkdir()
        child_path = root / 'programs/run-child.bin'
        if child is DIRECTORY:
            child_path.mkdir()
        elif child is not MISSING:
            child_path.write_bytes(child)
        line = root / 'line.txt'
        if initial is not MISSING:
            line.write_bytes(initial)
        r = subprocess.run([VM] if monitor else [VM, PARENT], input=data,
                           cwd=root, capture_output=True, timeout=15)
        assert (r.returncode, r.stdout, r.stderr) == (0, expected, b''), (name, r, expected)
        if final is MISSING:
            assert not line.exists(), name
        else:
            assert line.read_bytes() == final, name
        if child is not MISSING and child is not DIRECTORY:
            assert child_path.read_bytes() == child, name
    print(f'launch I/O: {name} OK')
    count += 1


sample = Path(CHILD).read_bytes()
assert sample == words(0x40000041, 0x60000000, 0x01000000)
check('run preserves edit', b'set keep\nrun\nshow\nquit\n', P + P + SUCCESS + P + b'keep\n' + Q, sample)
check('missing child returns', b'set keep\nrun\nshow\nquit\n', P + P + FAILURE + P + b'keep\n' + Q)
for length in range(4):
    check(f'reject {length} bytes', b'run\nquit\n', P + FAILURE + Q, sample[:length])
check('accept 4 bytes', b'run\nquit\n', P + b'CPU halted.\nRun finished.\n' + Q, words(0x01000000))
for length in (1024 * 1024 - 1, 1024 * 1024):
    check(f'accept size {length}', b'run\nquit\n', P + b'CPU halted.\nRun finished.\n' + Q,
          words(0x01000000) + b'\xA5' * (length - 4))
check('reject oversized', b'run\nquit\n', P + FAILURE + Q, sample + b'\0' * (1024 * 1024 + 1 - len(sample)))
check('directory read fails', b'run\nquit\n', P + FAILURE + Q, DIRECTORY)
check('invalid instruction', b'run\nshow\nquit\n', P + b'unknown instruction: FF000000\n' + FAILURE + P + b'\n' + Q, words(0xFF000000))
check('RET underflow', b'run\nquit\n', P + b'stack underflow\n' + FAILURE + Q, words(0x02000000))
check('out of range fetch', b'run\nquit\n', P + b'pc out of range: 0x00100000\n' + FAILURE + Q,
      words(0x680FFFFC) + b'\0' * (1024 * 1024 - 8) + words(0x40000000))
check('out of range write', b'run\nquit\n', P + b'memory address out of range: 0x00100000\n' + FAILURE + Q,
      words(0x401FFFFF, 0x20100000, 0x31100000))
check('unimplemented syscall', b'run\nquit\n', P + b'unimplemented syscall: 7\n' + FAILURE + Q, words(0x60000007))
check('spaces and CR', b'  r\ru\rn  \r\nquit\n', P + SUCCESS + Q, sample)
check('NUL command', b'run\0ignored\nquit\n', P + SUCCESS + Q, sample)
check('argument and exact name rejection', b'run x\nRUN\nruns\nrun\tx\nrun \t\nquit\n',
      (P + b'Unknown command\n') * 5 + Q, sample)
check('partial command EOF', b'run', P + b'input EOF\n', sample)
check('overlong command discard', b'run ' + b'x' * 4096 + b'\nquit\n', P + b'Unknown command\n' + Q, sample)
check('input shared with child', b'run\nXshow\nquit\n',
      P + b'XCPU halted.\nRun finished.\n' + P + b'\n' + Q, words(0x60000002, 0x60000003, 0x01000000))
check('child EOF returns before parent EOF', b'run\n',
      P + b'input EOF\nRun failed.\n' + P + b'input EOF\n', words(0x60000002, 0x01000000))

# Reject nested launches and check R0=1 inside the child before normal HALT.
nested = assemble('''SYSCALL 6
MOVI R1, 1
CMP R0, R1
JNZ failed
HALT
failed: SYSCALL 7
''')
check('nested request result', b'run\nquit\n', P + b'CPU halted.\nRun finished.\n' + Q, nested)

# The second child must see zeroed memory/registers and a false initial flag.
fresh_source = 'JZ failed\n'
for register in range(1, 8):
    fresh_source += f'CMP R0, R{register}\nJNZ failed\n'
fresh_source += '''MOVI R1, 0x600
LDB R0, [R1]
MOVI R2, 0
CMP R0, R2
JNZ failed
MOVI R0, 88
STB [R1], R0
CMP R0, R0
HALT
failed: SYSCALL 7
'''
fresh = assemble(fresh_source)
check('fresh children on repeated run', b'set keep\n' + b'run\n' * 1000 + b'show\nquit\n',
      P + (P + b'CPU halted.\nRun finished.\n') * 1000 + P + b'keep\n' + Q, fresh)

save_child = assemble('''MOVI R0, text
SYSCALL 4
HALT
text: .string "child"
''') + b'\0' * 64
check('child save visible to parent load', b'set parent\nrun\nshow\nload\nshow\nquit\n',
      P + P + b'CPU halted.\nRun finished.\n' + P + b'parent\n' + P + b'Loaded.\n' + P + b'child\n' + Q,
      save_child, initial=b'old', final=b'child')
load_child = assemble('''MOVI R0, 0x600
SYSCALL 5
MOVI R0, 0x600
SYSCALL 1
HALT
''')
check('child load uses shared file', b'set keep\nrun\nshow\nquit\n',
      P + P + b'from fileCPU halted.\nRun finished.\n' + P + b'keep\n' + Q,
      load_child, initial=b'from file', final=b'from file')
check('monitor and parent restart', PARENT.encode() + b'\nset keep\nrun\nshow\nquit\n' +
      PARENT.encode() + b'\nshow\nquit\nquit\n',
      b'Welcome to Handmade VM\n>' + P + P + SUCCESS + P + b'keep\n' + Q +
      b'>' + P + b'\n' + Q + b'>Goodbye from Handmade VM\n', sample, monitor=True)

# Check shipped artifacts against rebuilt bytes and independently inspect layout.
with tempfile.TemporaryDirectory(prefix='launch-layout-') as directory:
    for source, shipped in [('programs/program-launcher.asm', PARENT), ('programs/run-child.asm', CHILD)]:
        rebuilt = Path(directory) / Path(shipped).name
        r = subprocess.run([ASM, source, str(rebuilt)], capture_output=True, timeout=5)
        assert r.returncode == 0 and not r.stderr, r
        assert rebuilt.read_bytes() == Path(shipped).read_bytes(), source
blob = Path(PARENT).read_bytes()
code_size = blob.index(b'Command> \0')
assert code_size % 4 == 0 and len(blob) <= 0x500
instructions = struct.unpack('>' + 'I' * (code_size // 4), blob[:code_size])
assert instructions.count(0x60000006) == 1
print(f'program-launch I/O: {count} cases passed + 142 inherited storage cases')
print(f'program-launch layout: {code_size // 4} instructions / {code_size} code bytes; '
      f'{blob[code_size:].count(0)} strings / {len(blob) - code_size} data bytes; total {len(blob)} <= 0x500')
print('program-launch parent/child reassembly byte equality OK; child 3 instructions / 12 bytes')
