#!/usr/bin/env python3
"""Move simple C variable declarations to their first assignment."""

import argparse
import re
from pathlib import Path

DECLARATION_RE = re.compile(r"^(?P<indent> +)"
                            r"(?P<type>(?:(?:const|volatile|restrict|signed|unsigned|short|long|"
                            r"struct +[A-Za-z_]\w*|union +[A-Za-z_]\w*|enum +[A-Za-z_]\w*) +)*"
                            r"[A-Za-z_]\w*(?: *\*+ *| +))"
                            r"(?P<name>[A-Za-z_]\w*) *; *"
                            r"(?P<comment>/\*.*\*/)? *$")
NON_TYPE_WORDS = {
    "break",
    "case",
    "continue",
    "default",
    "do",
    "else",
    "extern",
    "goto",
    "register",
    "return",
    "static",
    "typedef",
}


def declaration(line):
    """Extract the indentation, type, name, and comment from a declaration."""
    match = DECLARATION_RE.match(line.rstrip("\r\n"))
    if match is None or match.group("type").split()[0] in NON_TYPE_WORDS:
        return None
    return match.group("indent"), match.group("type"), match.group("name"), match.group("comment")


def indentation(line):
    return len(line) - len(line.lstrip(" "))


def assignments(lines):
    """Return declaration and assignment line pairs that can be combined."""
    moves = []
    for declaration_index, line in enumerate(lines):
        details = declaration(line)
        if details is None:
            continue

        indent, var_type, name, comment = details
        assignment_re = re.compile(rf"^{re.escape(indent)}{re.escape(name)}(?P<rest> *= *(?!=).*?; *)"
                                   r"(?P<comment>(?://.*|/\*.*\*/))? *[\r\n]*$")
        preprocessor_continuation = False

        for assignment_index in range(declaration_index + 1, len(lines)):
            candidate = lines[assignment_index]
            preprocessor = preprocessor_continuation or candidate.lstrip().startswith("#")
            if preprocessor:
                preprocessor_continuation = candidate.rstrip("\r\n").endswith("\\")
                continue
            preprocessor_continuation = False

            if not candidate.strip():
                continue
            if indentation(candidate) < len(indent):
                break

            assignment_match = assignment_re.match(candidate)
            if assignment_match is None:
                continue

            newline = "\n" if candidate.endswith("\n") else ""
            replacement = indent + var_type + name + assignment_match.group("rest").rstrip()
            if comment is not None:
                replacement += " " + comment
            if assignment_match.group("comment") is not None:
                replacement += " " + assignment_match.group("comment")
            moves.append((declaration_index, assignment_index, replacement + newline))
            break

    assignment_counts = {}
    for _, assignment_index, _ in moves:
        assignment_counts[assignment_index] = assignment_counts.get(assignment_index, 0) + 1
    return [move for move in moves if assignment_counts[move[1]] == 1]


def leading_scope_blanks(lines, removed):
    """Find blank lines left at a scope start after all declarations move."""
    blanks = set()
    for index, line in enumerate(lines):
        if line.strip() != "{":
            continue

        saw_removed = False
        scope_blanks = []
        for scope_index in range(index + 1, len(lines)):
            if scope_index in removed:
                saw_removed = True
            elif not lines[scope_index].strip():
                scope_blanks.append(scope_index)
            else:
                break
        if saw_removed:
            blanks.update(scope_blanks)
    return blanks


def rewrite(text, path):
    lines = text.splitlines(keepends=True)
    moves = assignments(lines)
    removed = {declaration_index for declaration_index, _, _ in moves}
    replacements = {assignment_index: replacement for _, assignment_index, replacement in moves}
    removed.update(leading_scope_blanks(lines, removed))

    for declaration_index, assignment_index, _ in moves:
        print(f"{path}:{declaration_index + 1} -> {assignment_index + 1}")

    return "".join(replacements.get(index, line) for index, line in enumerate(lines) if index not in removed)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true", help="report changes without writing files")
    parser.add_argument("files", nargs="+", type=Path)
    args = parser.parse_args()

    changed = 0
    for path in args.files:
        original = path.read_text()
        revised = rewrite(original, path)
        if original == revised:
            continue
        changed += 1
        if not args.dry_run:
            path.write_text(revised)
    print(f"{changed} file(s) {'would change' if args.dry_run else 'changed'}")


if __name__ == "__main__":
    main()
