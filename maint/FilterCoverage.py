#! /usr/bin/env python3

# Filter LLVM's LCOV export: apply source exclusions, supply missing line
# coverpoints for branches, and recompute summaries from the retained records.
#
# Usage: python3 FilterCoverage.py coverage-lcov.info > coverage-lcov.filtered.info

import sys
import re


def scan_exclusions(srcpath):
    """Return a set of line numbers to exclude for this source file."""
    with open(srcpath, "r", encoding="utf-8") as fh:
        text = fh.readlines()
    excl = set()
    in_block = False
    for i, line in enumerate(text, start=1):
        if "LCOV_EXCL_LINE" in line:
            excl.add(i)
        if "LCOV_EXCL_START" in line:
            in_block = True
            excl.add(i)
            continue
        if "LCOV_EXCL_STOP" in line:
            excl.add(i)
            in_block = False
            continue
        if in_block:
            excl.add(i)

        # If line matches '^\s*#(if|endif|else|elif)\b', exclude it.
        # For some reason, Clang likes to output coverage for these lines,
        # even though they have no executable code.
        if re.match(r'^\s*#(if|endif|else|elif|ifdef|ifndef)\b', line):
            excl.add(i)

        # Similarly, Clang seems to generate DA entries for "}" lines inside
        # switch statements. Exclude these too.
        if re.match(r'^\s*}\s*(/\*.*?\*/)?$', line):
            excl.add(i)
    return excl


DA_RE = re.compile(r'^\s*DA:(\d+),(\d+)(,.*)?\s*$')
BRDA_RE = re.compile(r'^\s*BRDA:(\d+),(e?\d+),(.*),(-|\d+)\s*$')
FN_RE = re.compile(r'^\s*FN:(\d+),(?:(\d+),)?([^,\s]+)\s*$')
FNDA_RE = re.compile(r'^\s*FNDA:(\d+),([^,\s]*)\s*$')
SUMMARY_RE = re.compile(r'^\s*(LF|LH|BRF|BRH|FNF|FNH):(\d+)\s*$')


def process_block(block_lines):
    """Apply exclusions and regenerate LCOV summary records."""
    if not block_lines:
        return block_lines
    first = block_lines[0]
    assert first.lstrip().startswith('SF:')
    sf_path = first.split(':', 1)[1].strip()
    exclusions = scan_exclusions(sf_path)

    new_lines = []
    line_counts = {}
    branch_counts = {}
    function_counts = {}
    functions = set()
    brda_new_found = 0
    brda_new_hit = 0
    fn_exclusions = set()

    for line in block_lines:
        m_fn = FN_RE.match(line)
        assert (m_fn is not None) == line.lstrip().startswith('FN:')
        if m_fn:
            fn_line = int(m_fn.group(1))
            fn_name = m_fn.group(3)
            if fn_line in exclusions:
                fn_exclusions.add(fn_name)
            else:
                functions.add(fn_name)

    for line in block_lines:
        m_da = DA_RE.match(line)
        assert (m_da is not None) == line.lstrip().startswith('DA:')
        if m_da:
            line_num = int(m_da.group(1))
            execution_count = int(m_da.group(2))
            if line_num in exclusions:
                continue
            line_counts[line_num] = line_counts.get(line_num, 0) + execution_count
            new_lines.append(line)
            continue
        m_brda = BRDA_RE.match(line)
        assert (m_brda is not None) == line.lstrip().startswith('BRDA:')
        if m_brda:
            taken = m_brda.group(4)
            line_num = int(m_brda.group(1))
            if line_num in exclusions:
                continue
            brda_new_found += 1
            if taken != '-' and int(taken) > 0:
                brda_new_hit += 1
            blocks = branch_counts.setdefault(line_num, {})
            block = m_brda.group(2)
            blocks[block] = blocks.get(block, 0) + (0 if taken == '-' else int(taken))
            new_lines.append(line)
            continue
        m_fnda = FNDA_RE.match(line)
        assert (m_fnda is not None) == line.lstrip().startswith('FNDA:')
        if m_fnda:
            fn_name = m_fnda.group(2)
            count = int(m_fnda.group(1))
            if fn_name in fn_exclusions:
                continue
            function_counts[fn_name] = function_counts.get(fn_name, 0) + count
            new_lines.append(line)
            continue
        m_fn = FN_RE.match(line)
        if m_fn and m_fn.group(3) in fn_exclusions:
            continue
        new_lines.append(line)

    # LLVM can emit a branch without a mapped line (e.g. a memchr builtin).
    # Outcomes within one block are mutually exclusive. Their sum is a lower
    # bound on line executions; separate blocks may short-circuit.
    missing_lines = []
    for line_num, blocks in sorted(branch_counts.items()):
        if line_num in line_counts:
            continue
        count = max(blocks.values())
        line_counts[line_num] = count
        missing_lines.append(f"DA:{line_num},{count}\n")
        print(f"warning: inferred DA count {count} from branches at {sf_path}:{line_num}",
              file=sys.stderr)

    # LLVM's summaries aggregate function instantiations and folded branches
    # differently from its detailed LCOV records. Recount the records we emit,
    # rather than treating those known metric differences as corrupt input.
    summaries = {
        "LF": len(line_counts),
        "LH": sum(count > 0 for count in line_counts.values()),
        "BRF": brda_new_found,
        "BRH": brda_new_hit,
        "FNF": len(functions),
        "FNH": sum(function_counts.get(name, 0) > 0 for name in functions),
    }
    for i, line in enumerate(new_lines):
        match = SUMMARY_RE.match(line)
        if match:
            leading = re.match(r'^(\s*)', line).group(1)
            kind = match.group(1)
            new_lines[i] = f"{leading}{kind}:{summaries[kind]}\n"

    end = next((i for i, line in enumerate(new_lines)
                if line.strip() == "end_of_record"), len(new_lines))
    new_lines[end:end] = missing_lines
    return new_lines


def filter_lcov(in_fh, out_fh):
    lines = in_fh.readlines()

    i = 0
    out_lines = []
    while i < len(lines):
        line = lines[i]
        if line.lstrip().startswith('SF:'):
            # buffer block until end_of_record
            block = []
            while i < len(lines):
                block.append(lines[i])
                if lines[i].strip() == 'end_of_record':
                    i += 1
                    break
                i += 1
            processed = process_block(block)
            out_lines.extend(processed)
        else:
            out_lines.append(line)
            i += 1

    out_fh.writelines(out_lines)


if __name__ == "__main__":
    if len(sys.argv) > 3:
        print("Usage: python3 FilterCoverage.py [infile [outfile]]", file=sys.stderr)
        sys.exit(1)
    if len(sys.argv) > 2:
        with open(sys.argv[2], "w", encoding="utf-8") as out_fh:
            with open(sys.argv[1], "r", encoding="utf-8") as in_fh:
                filter_lcov(in_fh, out_fh)
    elif len(sys.argv) > 1:
        with open(sys.argv[1], "r", encoding="utf-8") as fh:
            filter_lcov(fh, sys.stdout)
    else:
        filter_lcov(sys.stdin, sys.stdout)
