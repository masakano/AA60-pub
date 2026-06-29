#!/usr/bin/env python3
import os
import re
import sys

# Match a single line that is just a C++ string literal (with optional indentation)
line_re = re.compile(r'^(?P<indent>\s*)"(?P<content>.*)"\s*$')


def iter_cpp_files(paths):
    for p in paths:
        if os.path.isdir(p):
            for dirpath, _, filenames in os.walk(p):
                for name in filenames:
                    if name.endswith('.cpp'):
                        yield os.path.join(dirpath, name)
        else:
            if p.endswith('.cpp'):
                yield p


def format_file(path):
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    out = []
    i = 0
    changed = False
    while i < len(lines):
        m = line_re.match(lines[i].rstrip('\n'))
        if not m:
            out.append(lines[i])
            i += 1
            continue

        # Collect a block of consecutive string-literal-only lines
        block = []
        while i < len(lines):
            m2 = line_re.match(lines[i].rstrip('\n'))
            if not m2:
                break
            block.append((lines[i], m2.group('indent'), m2.group('content')))
            i += 1

        # Only process blocks that contain '#version'
        if not any('#version' in content for _, _, content in block):
            out.extend([line for line, _, _ in block])
            continue

        # Determine max length before '\n' for lines that contain it
        lengths = []
        for _, _, content in block:
            idx = content.rfind('\\n')
            if idx != -1:
                lengths.append(len(content[:idx]))
        max_len = max(lengths) if lengths else None

        if max_len is None:
            out.extend([line for line, _, _ in block])
            continue

        # Pad spaces so that all '\n' align within this block
        for line, indent, content in block:
            idx = content.rfind('\\n')
            if idx == -1:
                out.append(line)
                continue
            pre = content[:idx]
            post = content[idx:]
            pad = ' ' * max(0, max_len - len(pre))
            new_content = pre + pad + post
            new_line = f"{indent}\"{new_content}\"\n"
            if new_line != line:
                changed = True
            out.append(new_line)

    if changed:
        with open(path, 'w', encoding='utf-8') as f:
            f.writelines(out)


def print_help():
    msg = """Usage: format.py [PATH...]

Pad shader string literal blocks (those containing '#version') so that the
'\\n' escape sequences align within each block.

Arguments:
  PATH  One or more .cpp files or directories to process.
        If omitted, defaults to oglplus/samples.
"""
    print(msg)


def main(argv):
    if '--help' in argv or '-h' in argv:
        print_help()
        return 0

    if len(argv) < 2:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        targets = [script_dir]
    else:
        import glob

        targets = []
        for a in argv[1:]:
            if a.startswith('-'):
                continue
            expanded = glob.glob(a)
            if expanded:
                targets.extend(expanded)
            else:
                targets.append(a)

    for file_path in iter_cpp_files(targets):
        format_file(file_path)
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))
