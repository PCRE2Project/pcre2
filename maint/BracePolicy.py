#!/usr/bin/env python3

"""Apply PCRE2's post-clang-format brace and spacing policies.

This is intentionally a formatter-shaped, line-oriented pass rather than a C
parser. It only changes statements whose control header and body can be
identified confidently, and leaves clang-format disabled regions untouched.
"""

import argparse
import re
from pathlib import Path


CONTROL_RE = re.compile(r"^(\s*)(?:(else)\s+)?(if|for|while|do)\b")
ELSE_RE = re.compile(r"^\s*else\b(?!\s+if\b)")
ELSE_FOLLOWING_RE = re.compile(r"^\s*else\b")
CLANG_FORMAT_RE = re.compile(r"//\s*clang-format\s+(off|on)\b", re.IGNORECASE)
CLOSING_BRACE_RE = re.compile(r"^\s*}\s*(?:(?://.*)|(?:/\*.*\*/))?\s*$")


def excluded_lines(lines):
  """Return whether each line is in a protected formatter or macro region."""

  excluded = []
  disabled = False
  preprocessor_continuation = False
  for line in lines:
    control = CLANG_FORMAT_RE.search(line)
    preprocessor = preprocessor_continuation or line.lstrip().startswith("#")
    excluded.append(disabled or control is not None or preprocessor)
    if control is not None:
      disabled = control.group(1).lower() == "off"
    preprocessor_continuation = preprocessor and line.rstrip("\r\n").endswith("\\")
  return excluded


def code_only(line):
  """Remove strings and line comments for simple delimiter counting."""

  result = []
  index = 0
  quote = None
  while index < len(line):
    char = line[index]
    if quote is not None:
      if char == "\\":
        index += 2
        continue
      if char == quote:
        quote = None
      index += 1
      continue
    if char in "\"'":
      quote = char
      index += 1
      continue
    if char == "/" and index + 1 < len(line) and line[index + 1] == "/":
      break
    result.append(char)
    index += 1
  return "".join(result)


def control_at(lines, index, excluded):
  """Return (kind, header_end), or None for an unsupported control header."""

  if excluded[index]:
    return None
  match = CONTROL_RE.match(lines[index])
  if match is None:
    if ELSE_RE.match(lines[index]) is not None:
      return "else", index
    return None

  kind = match.group(3)
  if kind == "do":
    return kind, index

  depth = 0
  saw_parenthesis = False
  for header_end in range(index, len(lines)):
    if excluded[header_end]:
      return None
    code = code_only(lines[header_end])
    if header_end == index:
      code = code[match.end():]
    for char in code:
      if char == "(":
        depth += 1
        saw_parenthesis = True
      elif char == ")":
        depth -= 1
    if saw_parenthesis and depth == 0:
      return kind, header_end
  return None


def next_content(lines, index, excluded):
  """Find the next non-blank eligible line without crossing an exclusion."""

  for next_index in range(index + 1, len(lines)):
    if excluded[next_index]:
      return None
    if lines[next_index].strip():
      return next_index
  return None


def brace_end(lines, start, excluded):
  """Find the matching close brace for a body starting with an open brace."""

  depth = 0
  for index in range(start, len(lines)):
    if excluded[index]:
      return None
    code = code_only(lines[index])
    depth += code.count("{") - code.count("}")
    if depth == 0:
      return index
  return None


def body_for(lines, header_end, excluded):
  """Return the first body line for a control header."""

  body = next_content(lines, header_end, excluded)
  if body is None or lines[body].lstrip().startswith("#"):
    return None
  return body


def do_while_end(lines, body_end, excluded):
  """Include a separately formatted while condition in a do statement."""

  if body_end is None or "while" in code_only(lines[body_end]):
    return body_end
  terminator = next_content(lines, body_end, excluded)
  if terminator is None or not re.match(r"^\s*while\b", lines[terminator]):
    return body_end
  for index in range(terminator, len(lines)):
    if excluded[index]:
      return body_end
    if ";" in code_only(lines[index]):
      return index
  return body_end


def statement_end(lines, start, excluded):
  """Return the final line of a simple formatter-shaped statement."""

  if start is None or excluded[start]:
    return None
  if lines[start].lstrip().startswith("{"):
    return brace_end(lines, start, excluded)

  control = control_at(lines, start, excluded)
  if control is not None:
    kind, header_end = control
    body = body_for(lines, header_end, excluded)
    end = statement_end(lines, body, excluded)
    if end is None:
      return None
    if kind == "do":
      return do_while_end(lines, end, excluded)
    if kind == "if":
      following = next_content(lines, end, excluded)
      if following is not None and control_at(lines, following, excluded) is not None:
        following_kind, _ = control_at(lines, following, excluded)
        if following_kind == "else":
          return statement_end(lines, following, excluded)
      if following is not None and ELSE_RE.match(lines[following]) is not None:
        return statement_end(lines, following, excluded)
    return end

  for index in range(start, len(lines)):
    if excluded[index]:
      return None
    if ";" in code_only(lines[index]):
      return index
  return None


def direct_braced(lines, body):
  return body is not None and lines[body].lstrip().startswith("{")


def body_contains_brace(lines, body, excluded):
  """Determine whether a body's control structure contains a block body."""

  if body is None:
    return False
  if direct_braced(lines, body):
    return True
  control = control_at(lines, body, excluded)
  if control is None:
    return False
  _, header_end = control
  return body_contains_brace(lines, body_for(lines, header_end, excluded), excluded)


def add_braces(add_before, add_after, lines, header_end, excluded):
  body = body_for(lines, header_end, excluded)
  if body is None or direct_braced(lines, body):
    return
  end = statement_end(lines, body, excluded)
  if end is None:
    return
  indent = re.match(r"\s*", lines[header_end]).group(0)
  add_before.setdefault(body, indent + "{\n")
  add_after.setdefault(end, indent + "}\n")


def collect_brace_additions(lines):
  """Collect one non-overlapping round of required brace insertions."""

  excluded = excluded_lines(lines)
  add_before = {}
  add_after = {}

  for index in range(len(lines)):
    control = control_at(lines, index, excluded)
    if control is None:
      continue
    kind, header_end = control
    body = body_for(lines, header_end, excluded)
    if body is None or direct_braced(lines, body):
      continue
    if body_contains_brace(lines, body, excluded):
      add_braces(add_before, add_after, lines, header_end, excluded)

    # Once an if/else chain uses braces for a direct branch body, every other
    # direct branch body in that chain must use braces too.
    if kind != "if" or lines[index].lstrip().startswith("else"):
      continue
    branches = [(header_end, body)]
    end = statement_end(lines, body, excluded)
    while end is not None:
      following = next_content(lines, end, excluded)
      if following is None:
        break
      next_control = control_at(lines, following, excluded)
      if next_control is not None and next_control[0] == "if" and lines[following].lstrip().startswith("else"):
        next_header_end = next_control[1]
      elif ELSE_RE.match(lines[following]) is not None:
        next_header_end = following
      else:
        break
      next_body = body_for(lines, next_header_end, excluded)
      if next_body is None:
        break
      branches.append((next_header_end, next_body))
      end = statement_end(lines, next_body, excluded)

    if any(direct_braced(lines, branch_body) for _, branch_body in branches):
      for branch_header_end, branch_body in branches:
        if not direct_braced(lines, branch_body):
          add_braces(add_before, add_after, lines, branch_header_end, excluded)

  return add_before, add_after


def apply_brace_policy(lines):
  """Iterate so a newly braced nested body also braces its parent."""

  for _ in range(20):
    add_before, add_after = collect_brace_additions(lines)
    if not add_before and not add_after:
      return lines
    rewritten = []
    for index, line in enumerate(lines):
      if index in add_before:
        rewritten.append(add_before[index])
      rewritten.append(line)
      if index in add_after:
        rewritten.append(add_after[index])
    lines = rewritten
  raise RuntimeError("brace policy did not converge")


def add_closing_brace_spacing(lines):
  """Ensure a blank line follows each standalone eligible closing brace."""

  excluded = excluded_lines(lines)
  rewritten = []
  for index, line in enumerate(lines):
    rewritten.append(line)
    if excluded[index] or CLOSING_BRACE_RE.match(line) is None:
      continue
    if index + 1 == len(lines):
      continue
    if index + 1 < len(lines):
      if (excluded[index + 1] or not lines[index + 1].strip() or
          CLOSING_BRACE_RE.match(lines[index + 1]) is not None or
          ELSE_FOLLOWING_RE.match(lines[index + 1]) is not None):
        continue
    rewritten.append("\n")
  return rewritten


def rewrite(text, braces=True, spacing=True):
  lines = text.splitlines(keepends=True)
  if braces:
    lines = apply_brace_policy(lines)
  if spacing:
    lines = add_closing_brace_spacing(lines)
  return "".join(lines)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument("--dry-run", action="store_true", help="report files that would change")
  parser.add_argument("--braces", action="store_true", help="apply only the added-braces behavior")
  parser.add_argument("--spaces", action="store_true", help="apply only the added-spaces behavior")
  parser.add_argument("files", nargs="+", type=Path)
  args = parser.parse_args()

  apply_both = not args.braces and not args.spaces
  braces = args.braces or apply_both
  spacing = args.spaces or apply_both

  changed = 0
  for path in args.files:
    original = path.read_text()
    revised = rewrite(original, braces=braces, spacing=spacing)
    if original == revised:
      continue
    changed += 1
    print(path)
    if not args.dry_run:
      path.write_text(revised)
  print(f"{changed} file(s) {'would change' if args.dry_run else 'changed'}")


if __name__ == "__main__":
  main()