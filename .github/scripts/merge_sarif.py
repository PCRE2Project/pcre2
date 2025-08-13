#! /usr/bin/env python3

# The purpose of this file is to adapt the output from
# Clang's static analyzer into a format suitable for GitHub
# Actions. The problem is that Clang outputs a separate "run"
# per file in its SARIF output, but GitHub requires a single
# run per tool (Clang is wrong here).

import sys
import json

if len(sys.argv) < 2:
    print("Usage: munge-sarif.py INPUT", file=sys.stderr)
    sys.exit(1)

data = None
with open(sys.argv[1], 'rb') as f:
    data = json.load(f)

# Arbitrarily pick the first run as the one from which to copy all the properties
base_run = data['runs'][0]

# We don't need these, GitHub ignores them
base_run['artifacts'] = []

# Concatenate results
for r in data['runs'][1:]:
    base_run['results'].extend(r['results'])

data['runs'] = [base_run]

def fix_region(region):
  startLine = region.get('startLine', None)
  startColumn = region.get('startColumn', 1)
  endLine = region.get('endLine', None)
  endColumn = region.get('endColumn', None)
  if startLine is None:
    raise ValueError("Region must have startLine")
  if endLine is not None and endLine < startLine:
    region['endLine'] = startLine
    del region['endColumn']
    endLine = startLine
    endColumn = None
  if endColumn is not None and (endLine == startLine or endLine is None) and endColumn < startColumn:
    region['endColumn'] = startColumn
    endColumn = startColumn

# Recursively scan the data dictionary, and apply the fix_region() function
# to all "region":Region key-value pairs.
def fix_regions(data):
    if isinstance(data, dict):
        if 'region' in data:
            fix_region(data['region'])
        for key, value in data.items():
            fix_regions(value)
    elif isinstance(data, list):
        for item in data:
            fix_regions(item)

fix_regions(data)

with open(sys.argv[1], 'w') as f:
    json.dump(data, f, indent=2)