#!/usr/bin/env python3
"""Extract and compare formatting-independent text from manpages and AsciiDoc.

With one input, print its normalized text. With two inputs, compare their
normalized word streams and show a unified diff when they differ.
"""

import argparse
import difflib
import re
from pathlib import Path


def troff_text(lines: list[str]) -> list[str]:
    result: list[str] = []
    for line in lines:
        if line.startswith('.TH'):
            continue
        if line.startswith(('.SH', '.SS')):
            result.append(re.sub(r'^\.S[HS]\s+"?(.*?)"?\s*$', r'\1', line))
        elif line.startswith(('.B ', '.I ')):
            result.append(line[3:])
        elif line.startswith('.\\"') or line.startswith('.'):
            continue
        else:
            result.append(line)
    return result


def asciidoc_text(lines: list[str]) -> list[str]:
    result: list[str] = []
    in_header = True
    for line in lines:
        if in_header:
            if line.startswith('== '):
                in_header = False
            else:
                continue
        if line in ('----', '....') or line.startswith('['):
            continue
        line = re.sub(r'(?:link|xref):[^\[]+\[([^]]*)\]', r'\1', line)
        result.append(re.sub(r'^={2,}\s+', '', line))
    return result


def normalize(path: Path) -> list[str]:
    lines = path.read_text(encoding='utf-8').splitlines()
    if path.suffix == '.adoc':
        lines = asciidoc_text(lines)
    else:
        lines = troff_text(lines)
    text = '\n'.join(lines)
    text = re.sub(r'\\f[IBPR]', '', text).replace(r'\e', '\\').replace(r'\&', '')
    return re.sub(r'[^A-Za-z0-9]+', ' ', text).split()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('documents', type=Path, nargs='+', metavar='DOCUMENT')
    arguments = parser.parse_args()
    if len(arguments.documents) not in (1, 2):
        parser.error('provide one document to normalize or two to compare')

    first = normalize(arguments.documents[0])
    if len(arguments.documents) == 1:
        print(' '.join(first))
        return

    second = normalize(arguments.documents[1])
    if first == second:
        print(f'{arguments.documents[0]} and {arguments.documents[1]}: text matches')
        return

    print('\n'.join(difflib.unified_diff(
        first,
        second,
        fromfile=str(arguments.documents[0]),
        tofile=str(arguments.documents[1]),
        lineterm='',
    )))
    raise SystemExit(1)


if __name__ == '__main__':
    main()