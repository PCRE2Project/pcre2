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
    title = re.match(r'(?:# ([^\n]+)\n+|([^\n]+)\n=+\n+)', doc_str)
    if title is None:
        raise ValueError("Document does not start with a Markdown title")
    return (title[1] or title[2], doc_str[title.end():])

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

os.makedirs('content/doc', exist_ok=True)
doc_pages = sorted(
    entry.path for entry in os.scandir('../doc')
    if entry.is_file() and entry.name.endswith('.adoc')
)
for file in doc_pages:
    base = os.path.splitext(os.path.basename(file))[0]
    is_index = base == 'index'
    output_name = '_index.adoc' if is_index else f'{base}.adoc'
    title = 'Manual pages' if is_index else base
    relfileprefix = '' if is_index else '../'

    with open(file, 'r') as infile:
        lines = infile.readlines()
    if not lines or not lines[0].startswith('= '):
        raise ValueError(f"AsciiDoc document {file} does not start with a title")

    lines[1:1] = [
        ':outfilesuffix: /\n',
        f':relfileprefix: {relfileprefix}\n',
    ]

    with open(f'content/doc/{output_name}', 'w') as outfile:
        outfile.write(f"""+++
title = "{title}"
+++

""")
        outfile.writelines(lines)

# Import the project pages

os.makedirs(f'content/project', exist_ok=True)
project_pages = ['AUTHORS.md', 'LICENCE.md', 'SECURITY.md', 'SUPPORT-LIFECYCLE.md']
for file in project_pages:
    with open(f'../{file}', 'r') as infile, open(f'content/project/{file}', 'w') as outfile:
        (title, content) = extract_title(infile.read())

        # Adjust the links for the website's structure
        def adjust_link(match):
            existing = match[0]
            href = match[1]
            try_match = re.match(r'^(https?://|#)', href)
            if try_match:
                return existing
            try_match = re.match(r'^\./(AUTHORS|LICENCE|SECURITY|SUPPORT-LIFECYCLE)\.md$', href)
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

build_env = {**os.environ, "HUGO_PARAMS_release": CURRENT_RELEASE}
ruby_load_path = os.path.abspath(f"{script_dir}/../../maint")
if "RUBYLIB" in build_env:
    ruby_load_path += os.pathsep + build_env["RUBYLIB"]
build_env["RUBYLIB"] = ruby_load_path

for command in commands:
    result = subprocess.run(command, env=build_env)
    if result.returncode != 0:
        raise Exception(f"Command '{command}' failed with exit code {result.returncode}")
