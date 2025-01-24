#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil
import re

script_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(f"{script_dir}/../../maint")
from UpdateCommon import CURRENT_RELEASE

print(f"Building site for release {CURRENT_RELEASE}")

# Helper function to take a document and extract its title

def extract_title(doc_str):
    # The title is formatted as ".*\n=+\n\n"
    title = re.search(r'(.*)\n=+\n\n', doc_str)
    return (title[1], doc_str[title.end():])

# Change directory to the `pages` directory

os.chdir(f"{script_dir}/..")

# Clean the automatic directories

clean_dirs = [
    "public",
    "content/doc",
]
for directory in clean_dirs:
    if os.path.exists(directory):
        shutil.rmtree(directory)

# Import the documentation

# - 1) Each individual man page

man_pages = [entry.path for entry in os.scandir('../doc') if entry.is_file() and entry.name.endswith(('.1', '.3'))]
for file in man_pages:
    base = os.path.splitext(os.path.basename(file))[0]
    os.makedirs(f'content/doc/{base}', exist_ok=True)
    with open(file, 'r') as infile, open(f'content/doc/{base}/index.html', 'w') as outfile:
        outfile.write(f"""+++
title = "{base}"
aliases = ["/doc/html/{base}.html"]
+++

""")

        # Run and capture the output
        result = subprocess.run(['perl', '../maint/132html', '-noheader', base], stdin=infile, capture_output=True, text=True)
        output = result.stdout

        # Adjust the links for the website's structure
        def adjust_link(match):
            existing = match[0]
            href = match[1]
            try_match = re.match(r'^(https?://|#)', href)
            if try_match:
                return existing
            try_match = re.match(r'^([^/]+).html((?:#[^/]+)?)$', href)
            if try_match:
                return f'href="../{try_match[1]}/{try_match[2]}"'
            try_match = re.match(r'^(README|NON-AUTOTOOLS-BUILD).txt$', href)
            if try_match:
                return f'href="../../guide/{try_match[1].lower()}/"'
            raise Exception(f"Could not adjust link {href}")

        output = re.sub(r'href="([^"]*)"', adjust_link, output)

        outfile.write(output)

# - 2) The index page

with open('../doc/html/index.html', 'r') as doc_index:
    index_content = doc_index.read()
with open('content/doc/_index.md', 'w') as f:
    f.write("""+++
title = "Manual pages"
+++

<p>
The reference manual for PCRE2 consists of a number of pages that are listed
below in alphabetical order. If you are new to PCRE2, please read the first one
first.
</p>
""")

    # Extract both the tables from the input file
    tables = re.search(r'<table>.*</table>', index_content, re.DOTALL)

    # Adjust the links for the website's structure
    def adjust_link(match):
        href = match[1]
        try_match = re.match(r'^([^/]+).html$', href)
        if try_match:
            return f'href="./{try_match[1]}/"'
        raise Exception(f"Could not adjust link {href}")

    tables = re.sub(r'href="([^"]*)"', adjust_link, tables[0])

    f.write(tables + "\n")

# Import the project pages

os.makedirs(f'content/project', exist_ok=True)
for file in ['AUTHORS.md', 'LICENCE.md', 'SECURITY.md']:
    with open(f'../{file}', 'r') as infile, open(f'content/project/{file}', 'w') as outfile:
        (title, content) = extract_title(infile.read())

        # Adjust the links for the website's structure
        def adjust_link(match):
            existing = match[0]
            href = match[1]
            try_match = re.match(r'^(https?://|#)', href)
            if try_match:
                return existing
            try_match = re.match(r'^./(LICENCE|SECURITY).md$', href)
            if try_match:
                return f'(../{try_match[1].lower()}/)'
            raise Exception(f"Could not adjust link {href}")

        content = re.sub(r'(?<=\])\((.*?)\)', adjust_link, content)

        outfile.write(f"""+++
title = "{title}"
+++

{content}
""")

# Import the guide pages

os.makedirs(f'content/guide', exist_ok=True)
for file in ['README', 'NON-AUTOTOOLS-BUILD']:
    with open(f'../{file}', 'r') as infile, open(f'content/guide/{file}.md', 'w') as outfile:
        content = infile.read()
        outfile.write(f"""+++
title = "{file}"
+++

```
{content}
```
""")

# Run commands to build site

base_url = None

if "HUGO_BASE_URL" in os.environ:
    base_url = os.environ['HUGO_BASE_URL']
elif "CODESPACE_NAME" in os.environ and "GITHUB_CODESPACES_PORT_FORWARDING_DOMAIN" in os.environ:
    base_url = f"https://{os.environ['CODESPACE_NAME']}-1313.{os.environ['GITHUB_CODESPACES_PORT_FORWARDING_DOMAIN']}/"
else:
    raise Exception("Environment variables CODESPACE_NAME and GITHUB_CODESPACES_PORT_FORWARDING_DOMAIN are not set")

commands = [
    ["hugo", "--minify", "-b", base_url],
    ["npx", "-y", "pagefind", "--site", "public"]
]

for command in commands:
    result = subprocess.run(command, env={**os.environ, "HUGO_PARAMS_release": CURRENT_RELEASE})
    if result.returncode != 0:
        raise Exception(f"Command '{command}' failed with exit code {result.returncode}")
