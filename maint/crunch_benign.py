#!/usr/bin/env python3

"""Reduce obvious formatting-only noise in a unified diff for review.

The output is a review aid, not an applyable patch: hunk line counts are not
updated after removed changes.
"""

import argparse
import math
from pathlib import Path


def without_horizontal_whitespace(line):
  return line.replace(" ", "").replace("\t", "")


def without_whitespace(line):
  return "".join(line.split())


def matching_replacement_run(lines, index):
  """Return the number of matching -/+ lines starting at index, or zero."""
  removed_end = index
  while removed_end < len(lines) and lines[removed_end].startswith("-") and \
      not lines[removed_end].startswith("---"):
    removed_end += 1

  removed_count = removed_end - index
  if removed_count == 0:
    return 0

  added_end = removed_end
  while added_end < len(lines) and lines[added_end].startswith("+") and \
      not lines[added_end].startswith("+++"):
    added_end += 1

  if added_end - removed_end < removed_count:
    return 0

  for offset in range(removed_count):
    removed = lines[index + offset]
    added = lines[removed_end + offset]
    if (without_horizontal_whitespace(removed[1:]) !=
        without_horizontal_whitespace(added[1:])):
      return 0

  return removed_count


def maximum_reflow_lines(removed_count):
  if removed_count <= 2:
    return 2 * removed_count
  return math.ceil(removed_count * 1.5)


def minimum_reflow_lines(removed_count):
  return max(1, math.floor(removed_count * 2 / 3))


def matching_reflow_run(lines, index):
  """Return matching -/+ reflow sizes starting at index, or zeroes."""
  removed_end = index
  while removed_end < len(lines) and lines[removed_end].startswith("-") and \
      not lines[removed_end].startswith("---"):
    removed_end += 1

  removed_count = removed_end - index
  if removed_count == 0:
    return 0, 0

  added_end = removed_end
  while added_end < len(lines) and lines[added_end].startswith("+") and \
      not lines[added_end].startswith("+++"):
    added_end += 1

  added_count = added_end - removed_end
  if (added_count < minimum_reflow_lines(removed_count) or
      added_count > maximum_reflow_lines(removed_count)):
    return 0, 0

  removed = "".join(lines[index:removed_end])
  added = "".join(lines[removed_end:added_end])
  if without_whitespace(removed[1:].replace("\n-", "")) != \
      without_whitespace(added[1:].replace("\n+", "")):
    return 0, 0

  return removed_count, added_count


def is_changed_line(line):
  return (line.startswith(("+", "-")) and
          not line.startswith(("+++", "---")))


def trim_hunks(lines):
  """Drop empty hunks and keep two context lines around remaining changes."""
  result = []
  index = 0
  empty_hunks = 0
  trimmed_context_lines = 0

  while index < len(lines):
    if not lines[index].startswith("@@ "):
      result.append(lines[index])
      index += 1
      continue

    hunk_end = index + 1
    while (hunk_end < len(lines) and
           not lines[hunk_end].startswith(("@@ ", "diff --git "))):
      hunk_end += 1

    hunk = lines[index + 1:hunk_end]
    changed_indexes = [line_index for line_index, line in enumerate(hunk)
                       if is_changed_line(line)]
    if not changed_indexes:
      empty_hunks += 1
      index = hunk_end
      continue

    first = max(0, changed_indexes[0] - 2)
    last = min(len(hunk), changed_indexes[-1] + 3)
    result.append(lines[index])
    result.extend(hunk[first:last])
    trimmed_context_lines += len(hunk) - (last - first)
    index = hunk_end

  return result, empty_hunks, trimmed_context_lines


def crunch(lines):
  result = []
  index = 0
  whitespace_lines = 0
  reflow_lines = 0

  while index < len(lines):
    matching_lines = matching_replacement_run(lines, index)
    if matching_lines:
      result.extend(" " + lines[index + offset][1:]
                    for offset in range(matching_lines))
      index += 2 * matching_lines
      whitespace_lines += matching_lines
      continue

    removed_count, added_count = matching_reflow_run(lines, index)
    if removed_count:
      result.extend(" " + lines[index + offset][1:]
                    for offset in range(removed_count))
      index += removed_count + added_count
      reflow_lines += removed_count + added_count
    else:
      result.append(lines[index])
      index += 1

  result, empty_hunks, trimmed_context_lines = trim_hunks(result)
  return result, whitespace_lines, reflow_lines, empty_hunks, \
      trimmed_context_lines


def main():
  parser = argparse.ArgumentParser(
    description="Remove provably benign formatting noise from a unified diff.")
  parser.add_argument("diff", type=Path, help="unified diff to rewrite")
  arguments = parser.parse_args()

  original = arguments.diff.read_text()
  lines = original.splitlines(keepends=True)
  compacted, whitespace_lines, reflow_lines, empty_hunks, \
      trimmed_context_lines = crunch(lines)
  arguments.diff.write_text("".join(compacted))
  print(f"Removed {whitespace_lines} whitespace-only replacement lines and "
        f"{reflow_lines} bounded reflow lines, {empty_hunks} empty hunks, and "
        f"{trimmed_context_lines} context lines.")


if __name__ == "__main__":
  main()