#!/usr/bin/env python3
"""Move conservatively selected C declarations to their first assignment."""

import argparse
import re
from dataclasses import dataclass
from pathlib import Path

DECLARATION_RE = re.compile(
    r"^(?P<indent> +)"
    r"(?P<type>(?:(?:const|volatile|restrict|signed|unsigned|short|long|"
    r"struct +[A-Za-z_]\w*|union +[A-Za-z_]\w*|enum +[A-Za-z_]\w*) +)*"
    r"[A-Za-z_]\w*(?: *\*+ *| +))"
    r"(?P<names>[A-Za-z_]\w*(?: *, *[A-Za-z_]\w*)*) *; *"
    r"(?P<comment>(?://.*|/\*.*\*/))? *$"
)
NON_TYPE_WORDS = {
    "break", "case", "continue", "default", "do", "else", "extern",
    "goto", "register", "return", "static", "typedef",
}
MACRO_WRITE_RE = re.compile(r"\bGETCHAR[A-Z0-9_]*\s*\(\s*([A-Za-z_]\w*)\s*,")
LABEL_RE = re.compile(r"^\s*(?:case\b.*|default|[A-Za-z_]\w*)\s*:")
GOTO_RE = re.compile(r"\bgoto\s+[A-Za-z_]\w*\s*;")
CATEGORIES = (
    "movable",
    "used before assignment",
    "multiple assignment",
    "macro-assigned",
    "conditional",
    "label-goto-sensitive",
    "never directly assigned",
)


@dataclass(frozen=True)
class Declaration:
    indent: str
    var_type: str
    names: tuple
    comment: str | None


@dataclass(frozen=True)
class Finding:
    declaration_index: int
    assignment_index: int | None
    declaration: Declaration
    name: str
    category: str
    assignment: re.Match | None = None


def declaration(line):
    """Parse a declaration containing one or more simple names."""
    match = DECLARATION_RE.match(line.rstrip("\r\n"))
    if match is None or match.group("type").split()[0] in NON_TYPE_WORDS:
        return None
    return Declaration(match.group("indent"), match.group("type"),
                       tuple(re.split(r" *, *", match.group("names"))),
                       match.group("comment"))


def indentation(line):
    return len(line) - len(line.lstrip(" "))


def code_part(line):
    """Remove strings and comments sufficiently for conservative token checks."""
    line = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', '""', line)
    return re.sub(r"//.*|/\*.*?\*/", "", line)


def preprocessor_branches(lines):
    """Return the exact conditional branch containing each source line."""
    branches = []
    stack = []
    for index, line in enumerate(lines):
        stripped = line.lstrip()
        word_match = re.match(r"#\s*(\w+)", stripped)
        word = word_match.group(1) if word_match else ""
        if word == "endif":
            if stack:
                stack.pop()
        elif word in ("else", "elif") and stack:
            conditional, branch = stack[-1]
            stack[-1] = (conditional, branch + 1)

        branches.append(tuple(stack))
        if word in ("if", "ifdef", "ifndef"):
            stack.append((index, 0))
    return branches


def scope_start_declarations(lines):
    """Find declarations in the initial declaration run of each braced scope."""
    result = set()
    for brace_index, line in enumerate(lines):
        if line.strip() != "{":
            continue
        in_block_comment = False
        for index in range(brace_index + 1, len(lines)):
            stripped = lines[index].strip()
            if in_block_comment:
                in_block_comment = "*/" not in stripped
                continue
            if stripped.startswith("/*"):
                in_block_comment = "*/" not in stripped
                continue
            if not stripped or stripped.startswith("//") or stripped.startswith("#"):
                continue
            if declaration(lines[index]) is not None:
                result.add(index)
                continue
            break
    return result


def scope_end(lines, declaration_index, indent):
    for index in range(declaration_index + 1, len(lines)):
        stripped = lines[index].strip()
        if stripped.startswith("}") and indentation(lines[index]) < len(indent):
            return index
    return len(lines)


def analyze(lines):
    """Categorize variables declared at the start of a scope."""
    findings = []
    branches = preprocessor_branches(lines)
    for declaration_index in sorted(scope_start_declarations(lines)):
        details = declaration(lines[declaration_index])
        end = scope_end(lines, declaration_index, details.indent)
        for name in details.names:
            token_re = re.compile(rf"\b{re.escape(name)}\b")
            assignment_re = re.compile(
                rf"^{re.escape(details.indent)}{re.escape(name)}"
                r"(?P<rest> *= *(?!=).*?; *)"
                r"(?P<comment>(?://.*|/\*.*\*/))? *[\r\n]*$")
            direct = []
            first_event = None
            for index in range(declaration_index + 1, end):
                code = code_part(lines[index])
                assignment_match = assignment_re.match(lines[index])
                if assignment_match is not None:
                    direct.append((index, assignment_match))
                    rhs = assignment_match.group("rest").split("=", 1)[1]
                    event = "use" if token_re.search(code_part(rhs)) else "direct"
                    if first_event is None:
                        first_event = (event, index)
                    continue
                if any(found == name for found in MACRO_WRITE_RE.findall(code)):
                    if first_event is None:
                        first_event = ("macro", index)
                    continue
                if token_re.search(code) and first_event is None:
                    first_event = ("use", index)

            first_assignment = direct[0][0] if direct else None
            span_end = first_assignment + 1 if first_assignment is not None else end
            first_event_kind = first_event[0] if first_event else None
            first_event_index = first_event[1] if first_event else None
            conditional = first_event_index is not None and (
                branches[first_event_index] != branches[declaration_index]
            )
            label_sensitive = any(
                LABEL_RE.search(code_part(lines[index])) or
                GOTO_RE.search(code_part(lines[index]))
                for index in range(declaration_index + 1, span_end)
            )

            if first_event_kind == "use":
                category = "used before assignment"
            elif first_event_kind == "macro":
                category = "macro-assigned"
            elif first_event_kind is None:
                category = "never directly assigned"
            elif len(direct) > 1:
                category = "multiple assignment"
            elif conditional:
                category = "conditional"
            elif label_sensitive:
                category = "label-goto-sensitive"
            else:
                category = "movable"
            assignment_index, match = direct[0] if direct else (None, None)
            findings.append(Finding(declaration_index, assignment_index, details,
                                    name, category, match))
    return findings


def assignments(lines):
    """Return safe declaration/assignment combinations."""
    return [finding for finding in analyze(lines) if finding.category == "movable"]


def print_report(file_findings):
    """Print grouped automatic and manual queues plus an exact proportion."""
    grouped = {category: [] for category in CATEGORIES}
    for path, finding in file_findings:
        grouped[finding.category].append((path, finding))

    movable = grouped["movable"]
    remaining = sum(len(grouped[category]) for category in CATEGORIES[1:])
    total = len(movable) + remaining

    print("AUTOMATIC MOVE QUEUE")
    print(f"movable ({len(movable)})")
    for path, finding in movable:
        print(f"  {path}:{finding.declaration_index + 1}: {finding.name}"
              f" -> {finding.assignment_index + 1}")

    print("\nMANUAL QUEUE")
    for category in CATEGORIES[1:]:
        candidates = grouped[category]
        print(f"{category} ({len(candidates)})")
        for path, finding in candidates:
            print(f"  {path}:{finding.declaration_index + 1}: {finding.name}")

    print("\nSUMMARY")
    for category in CATEGORIES:
        print(f"{category}: {len(grouped[category])}")
    print(f"remaining scope-start uninitialized variables: {remaining}")
    denominator = f"{len(movable)}+{remaining}"
    exact = f"{len(movable)}/{total}" if total else "0/0"
    print("moved/(moved + remaining scope-start uninitialized variables): "
          f"{len(movable)}/({denominator}) = {exact}")


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


def rewrite(text, path, report=True):
    lines = text.splitlines(keepends=True)
    findings = analyze(lines)
    moves = [finding for finding in findings if finding.category == "movable"]
    by_declaration = {}
    for finding in findings:
        by_declaration.setdefault(finding.declaration_index, []).append(finding)

    removed = set()
    declaration_replacements = {}
    assignment_replacements = {}
    for declaration_index, declaration_findings in by_declaration.items():
        moved_names = {finding.name for finding in declaration_findings
                       if finding.category == "movable"}
        if not moved_names:
            continue
        details = declaration_findings[0].declaration
        remaining = [name for name in details.names if name not in moved_names]
        if remaining:
            declaration_replacements[declaration_index] = (
                details.indent + details.var_type + ", ".join(remaining) + ";"
                + ((" " + details.comment) if details.comment else "")
                + ("\n" if lines[declaration_index].endswith("\n") else "")
            )
        else:
            removed.add(declaration_index)

        comment_available = not remaining
        for finding in declaration_findings:
            if finding.category != "movable":
                continue
            candidate = lines[finding.assignment_index]
            replacement = details.indent + details.var_type + finding.name
            replacement += finding.assignment.group("rest").rstrip()
            if comment_available and details.comment:
                replacement += " " + details.comment
                comment_available = False
            if finding.assignment.group("comment"):
                replacement += " " + finding.assignment.group("comment")
            if candidate.endswith("\n"):
                replacement += "\n"
            assignment_replacements[finding.assignment_index] = replacement

    removed.update(leading_scope_blanks(lines, removed))
    if report:
        for finding in findings:
            print(f"{path}:{finding.declaration_index + 1}: "
                  f"{finding.name}: {finding.category}")

    replacements = {**declaration_replacements, **assignment_replacements}
    return "".join(replacements.get(index, line)
                   for index, line in enumerate(lines) if index not in removed)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument("--dry-run", action="store_true",
                       help="report changes without writing files")
    modes.add_argument("--report-only", action="store_true",
                       help="print categorized candidates without changing files")
    parser.add_argument("files", nargs="+", type=Path)
    args = parser.parse_args()

    if args.report_only:
        file_findings = []
        for path in args.files:
            lines = path.read_text().splitlines(keepends=True)
            file_findings.extend((path, finding) for finding in analyze(lines))
        print_report(file_findings)
        return

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
