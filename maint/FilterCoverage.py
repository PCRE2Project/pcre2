#! /usr/bin/env python3

# Script which is a simple LCOV filter: removes DA/BRDA entries for lines marked in-source with
# "LCOV_EXCL_LINE" or "LCOV_EXCL_START"/"LCOV_EXCL_STOP".
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
LF_RE = re.compile(r'^\s*LF:(\d+)\s*$')
LH_RE = re.compile(r'^\s*LH:(\d+)\s*$')
BRDA_RE = re.compile(r'^\s*BRDA:(\d+),([e\d]+),(.*),([-\d]+)\s*$')
BRF_RE = re.compile(r'^\s*BRF:(\d+)\s*$')
BRH_RE = re.compile(r'^\s*BRH:(\d+)\s*$')
FN_RE = re.compile(r'^\s*FN:(\d+),([^,\s]*)\s*$')
FNDA_RE = re.compile(r'^\s*FNDA:(\d+),([^,\s]*)\s*$')
FNF_RE = re.compile(r'^\s*FNF:(\d+)\s*$')
FNH_RE = re.compile(r'^\s*FNH:(\d+)\s*$')

def process_block(block_lines):
    """Return processed block lines with excluded DA/BRDA removed and LF/LH fixed."""
    if not block_lines:
        return block_lines
    # get SF path from first 'SF:' line (should be first)
    first = block_lines[0]
    assert first.lstrip().startswith('SF:')
    sf_path = first.split(':', 1)[1].strip()
    exclusions = scan_exclusions(sf_path)

    new_lines = []
    da_orig_found = 0
    da_orig_hit = 0
    da_new_found = 0
    da_new_hit = 0
    brda_orig_found = 0
    brda_orig_hit = 0
    brda_new_found = 0
    brda_new_hit = 0
    fnda_orig_found = 0
    fnda_orig_hit = 0
    fnda_new_found = 0
    fnda_new_hit = 0

    fn_exclusions = set()

    # Pass 1: identify FN exclusions
    for line in block_lines:
        m_fn = FN_RE.match(line)
        assert (m_fn is not None) == line.lstrip().startswith('FN:')
        if m_fn:
            fn_line = int(m_fn.group(1))
            fn_name = m_fn.group(2)
            if fn_line in exclusions:
                fn_exclusions.add(fn_name)

    # Pass 2: filter DA, BRDA, FN/FNDA; copy others verbatim
    for line in block_lines:
        m_da = DA_RE.match(line)
        assert (m_da is not None) == line.lstrip().startswith('DA:')
        if m_da:
            line_num = int(m_da.group(1))
            execution_count = int(m_da.group(2))
            da_orig_found += 1
            if execution_count > 0:
                da_orig_hit += 1
            if line_num in exclusions:
                # drop this DA line
                continue
            da_new_found += 1
            if execution_count > 0:
                da_new_hit += 1
            new_lines.append(line)
            continue
        m_brda = BRDA_RE.match(line)
        assert (m_brda is not None) == line.lstrip().startswith('BRDA:')
        if m_brda:
            brda_orig_found += 1
            taken = m_brda.group(4)
            if taken != '-' and int(taken) > 0:
                brda_orig_hit += 1
            if int(m_brda.group(1)) in exclusions:
                # drop this BRDA line
                continue
            brda_new_found += 1
            if taken != '-' and int(taken) > 0:
                brda_new_hit += 1
            new_lines.append(line)
            continue
        m_fnda = FNDA_RE.match(line)
        assert (m_fnda is not None) == line.lstrip().startswith('FNDA:')
        if m_fnda:
            fnda_orig_found += 1
            fn_name = m_fnda.group(2)
            count = int(m_fnda.group(1))
            if count > 0:
                fnda_orig_hit += 1
            if fn_name in fn_exclusions:
                # drop this FNDA line
                continue
            fnda_new_found += 1
            if count > 0:
                fnda_new_hit += 1
            new_lines.append(line)
            continue
        m_fn = FN_RE.match(line)
        assert (m_fn is not None) == line.lstrip().startswith('FN:')
        if m_fn:
            fn_line = int(m_fn.group(1))
            fn_name = m_fn.group(2)
            if fn_name in fn_exclusions:
                # drop this FN line
                continue
            new_lines.append(line)
            continue
        # other lines: append unchanged
        new_lines.append(line)

    # Pass 3: fix LF/LH, BRF/BRH, FNF/FNH
    # Mutate new_lines. If we find any LF/LH lines, check they have the expected original values.
    # If so, replace with new values. If not, print a warning.
    for i, line in enumerate(new_lines):
        # LF
        m_lf = LF_RE.match(line)
        assert (m_lf is not None) == line.lstrip().startswith('LF:')
        if m_lf:
            # preserve leading whitespace exactly
            leading = re.match(r'^(\s*)', line).group(1)
            # replace with recomputed value (number of DA entries remaining)
            new_lines[i] = f"{leading}LF:{da_new_found}\n"
            # warn if original disagreed (useful for debugging)
            try:
                lf_orig = int(m_lf.group(1))
                if lf_orig != da_orig_found:
                    print(f"warning: original LF ({lf_orig}) != counted DA entries ({da_orig_found}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

        # LH
        m_lh = LH_RE.match(line)
        assert (m_lh is not None) == line.lstrip().startswith('LH:')
        if m_lh:
            leading = re.match(r'^(\s*)', line).group(1)
            new_lines[i] = f"{leading}LH:{da_new_hit}\n"
            try:
                lh_orig = int(m_lh.group(1))
                if lh_orig != da_orig_hit:
                    print(f"warning: original LH ({lh_orig}) != counted DA hits ({da_orig_hit}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

        # BRF
        m_brf = BRF_RE.match(line)
        assert (m_brf is not None) == line.lstrip().startswith('BRF:')
        if m_brf:
            leading = re.match(r'^(\s*)', line).group(1)
            # replace with recomputed branch-found (if you computed brda_new_found above)
            new_lines[i] = f"{leading}BRF:{brda_new_found}\n"
            try:
                brf_orig = int(m_brf.group(1))
                if brf_orig != brda_orig_found:
                    print(f"warning: original BRF ({brf_orig}) != counted BRDA entries ({brda_orig_found}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

        # BRH
        m_brh = BRH_RE.match(line)
        assert (m_brh is not None) == line.lstrip().startswith('BRH:')
        if m_brh:
            leading = re.match(r'^(\s*)', line).group(1)
            new_lines[i] = f"{leading}BRH:{brda_new_hit}\n"
            try:
                brh_orig = int(m_brh.group(1))
                if brh_orig != brda_orig_hit:
                    print(f"warning: original BRH ({brh_orig}) != counted BRDA hits ({brda_orig_hit}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

        # FNF
        m_fnf = FNF_RE.match(line)
        assert (m_fnf is not None) == line.lstrip().startswith('FNF:')
        if m_fnf:
            leading = re.match(r'^(\s*)', line).group(1)
            new_lines[i] = f"{leading}FNF:{fnda_new_found}\n"
            try:
                fnf_orig = int(m_fnf.group(1))
                if fnf_orig != fnda_orig_found:
                    print(f"warning: original FNF ({fnf_orig}) != counted FNDA entries ({fnda_orig_found}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

        # FNH
        m_fnh = FNH_RE.match(line)
        assert (m_fnh is not None) == line.lstrip().startswith('FNH:')
        if m_fnh:
            leading = re.match(r'^(\s*)', line).group(1)
            new_lines[i] = f"{leading}FNH:{fnda_new_hit}\n"
            try:
                fnh_orig = int(m_fnh.group(1))
                if fnh_orig != fnda_orig_hit:
                    print(f"warning: original FNH ({fnh_orig}) != counted FNDA hits ({fnda_orig_hit}) for {sf_path}", file=sys.stderr)
            except Exception:
                pass
            continue

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
