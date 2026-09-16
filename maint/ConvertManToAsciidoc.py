#!/usr/bin/env python3
"""Convert the PCRE2 troff manpage subset to AsciiDoc.

This is a transition helper. It intentionally supports only constructs that
occur in the PCRE2 manpages and leaves unfamiliar input visible for review.
"""

import argparse
import re
from datetime import datetime
from pathlib import Path


TH_RE = re.compile(r'^\.TH\s+(\S+)\s+(\S+)\s+"([^"]+)"\s+"([^"]+)"\s*$')
SECTION_RE = re.compile(r'^\.S([HS])\s*(?:"(.*)"|(.*))\s*$')


def inline(text: str) -> str:
    """Translate the inline escape sequences used by the PCRE2 manpages."""
    text = text.rstrip()
    text = re.sub(r'\\fB(.*?)\\f[PR]', r'*\1*', text)
    text = re.sub(r'\\fI(.*?)\\f[PR]', r'_\1_', text)
    return text.replace(r'\e', '\\').replace(r'\&', '')


def literal(text: str) -> str:
    """Retain the text of formatted lines inside a listing block."""
    text = text.rstrip()
    text = re.sub(r'\\f[BI](.*?)\\f[PR]', r'\1', text)
    return text.replace(r'\e', '\\').replace(r'\&', '')


def macro_argument(text: str) -> str:
    """Remove troff's optional quotes around a one-line macro argument."""
    if len(text) >= 2 and text.startswith('"') and text.endswith('"'):
        return text[1:-1]
    return text


def convert(source: Path) -> str:
    lines = source.read_text(encoding='utf-8').splitlines()
    output: list[str] = []
    title_seen = False
    in_literal = False
    pending_tag = False
    index = 0

    while index < len(lines):
        line = lines[index]
        index += 1

        match = TH_RE.match(line)
        if match:
            title, volume, revision, source_name = match.groups()
            revision = datetime.strptime(revision, '%d %B %Y').date().isoformat()
            output.extend((
                f'= {title}({volume})',
                ':doctype: manpage',
                f':manname: {title}',
                f':manvolnum: {volume}',
                ':manmanual: PCRE2',
                f':mansource: {source_name}',
                f':docdate: {revision}',
                f':revdate: {revision}',
                '',
            ))
            title_seen = True
            continue

        match = SECTION_RE.match(line)
        if match:
            level, quoted, plain = match.groups()
            heading = quoted if quoted is not None else plain
            output.extend((('==' if level == 'H' else '===') + f' {inline(heading)}', ''))
            pending_tag = False
            continue

        if line == '.nf':
            output.extend(('[source]', '----'))
            in_literal = True
            continue

        if line == '.fi':
            output.extend(('----', ''))
            in_literal = False
            continue

        if line.startswith('.TP'):
            pending_tag = True
            continue

        if line in ('.P', '.PP', '.rs', '.sp', '.'):
            if output and output[-1] != '':
                output.append('')
            continue

        if line.startswith('.\\" HREF'):
            if index >= len(lines):
                raise ValueError(f'{source}: HREF directive without link text')
            label = inline(lines[index])
            target = literal(lines[index])
            index += 1
            output.append(f'link:{target}.html[{label}]')
            continue

        if line.startswith('.\\"'):
            continue

        if line.startswith('.B '):
            content = macro_argument(line[3:])
            if in_literal:
                output.append(literal(content))
            elif pending_tag:
                output.append(f'{inline(content)}::')
                pending_tag = False
            else:
                output.append(f'*{inline(content)}*')
            continue

        if line.startswith('.I '):
            output.append(f'_{inline(macro_argument(line[3:]))}_')
            continue

        if line.startswith('.'):
            raise ValueError(f'{source}: unsupported troff request: {line}')

        content = literal(line) if in_literal else inline(line)
        if pending_tag:
            output.append(f'{content}::')
            pending_tag = False
        else:
            output.append(content)

    if not title_seen:
        raise ValueError(f'{source}: missing .TH request')
    if in_literal:
        raise ValueError(f'{source}: unterminated .nf block')

    while output and output[-1] == '':
        output.pop()
    return '\n'.join(output) + '\n'


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path, help='input .1 or .3 manpage')
    parser.add_argument('output', type=Path, help='output .adoc file')
    arguments = parser.parse_args()
    arguments.output.write_text(convert(arguments.source), encoding='utf-8')


if __name__ == '__main__':
    main()