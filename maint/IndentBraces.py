#!/usr/bin/env python3

import re
import sys
from pathlib import Path


def indent_braces(text):
  lines = text.splitlines(keepends=True)
  inside_function = False
  inside_clang_format_off = False

  for index, line in enumerate(lines):
    starts_with_open_brace = line.startswith("{")
    starts_with_close_brace = line.startswith("}")

    if "clang-format off" in line.lstrip(" \t"):
      inside_clang_format_off = True
    elif "clang-format on" in line.lstrip(" \t"):
      inside_clang_format_off = False

    if starts_with_open_brace:
      inside_function = True
    elif starts_with_close_brace:
      inside_function = False

    # This line doesn't need indentation adjustment
    if starts_with_open_brace or starts_with_close_brace:
      continue

    # If it starts with '^\s*#' then ignore the line
    if line.lstrip(" \t").startswith("#"):
      continue

    # If inside clang-format off, skip indentation adjustment
    if inside_clang_format_off:
      continue

    # Everything in a function gets nudged inwards
    if inside_function and line.strip() != "":
      line = "  " + line

    # All braces and case labels need to be de-indented if possible
    m = re.match(r"^(\s*)([{}]|(?:case\b.*|default|HSPACE_CASES|VSPACE_CASES):)", line)
    if m:
      spaces = m.group(1)
      frontpart = m.group(2)
      trailpart = line[len(spaces) + len(frontpart):]
      if len(spaces) >= 2:
        if len(trailpart) >= 1 and trailpart[0] not in "\r\n":
          trailpart = "  " + trailpart
        line = spaces[2:] + frontpart + trailpart

    # # Convert trailing /*...*/ comments to // style. Spec: must have non-whitespace
    # # before, and be at the end of the line
    # m = re.match(r"^(.*?)/\*(.*?)\*/(?=\s*$)", line)
    # if m and m.group(1).strip():
    #   trailpart = line[m.end():]
    #   line = f"{m.group(1)}//{m.group(2).rstrip()}{trailpart}"

    lines[index] = line

  return "".join(lines)


for filename in sys.argv[1:]:
  path = Path(filename)
  path.write_text(indent_braces(path.read_text()))
