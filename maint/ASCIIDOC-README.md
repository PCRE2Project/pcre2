# AsciiDoc Manpage Transition

## Status

The trial conversion replaces these three troff source files with AsciiDoc:

* `doc/pcre2-config.1`
* `doc/pcre2_code_free.3`
* `doc/pcre2_compile.3`

Their rendered manpages are generated in `doc/man/`, retaining the original
filenames and sections. Their HTML files retain the existing `doc/html/`
locations. All other `.1` and `.3` files remain troff sources during the
transition.

## Tools

`RenderAsciidoc` invokes Asciidoctor to derive a manpage and HTML document from
one `.adoc` source. `UpdateAlways` renders converted pages before generating
the other release documentation.

Asciidoctor's manpage title parser removes underscores from a document title.
`RenderAsciidoc` restores the title in the generated `.TH` request from the
output filename, preserving PCRE2's existing manpage titles.

`ConvertManToAsciidoc.py` is the one-time conversion helper. It supports the
troff constructs used by the trial pages: headings, paragraphs, tagged
paragraphs, literal blocks, inline bold and italic text, and PCRE2 HTML-link
directives. It rejects unknown troff requests so each later conversion is
reviewed rather than silently degraded.

`CompareDocText.py` normalizes a troff source or an AsciiDoc source into a
whitespace-separated alphanumeric word stream. Compare an old and converted
source before deleting the old source:

```
python3 maint/CompareDocText.py doc/pcre2-config.1 doc/pcre2-config.adoc
```

The comparison deliberately ignores document metadata and formatting syntax;
inspect any reported word diff before accepting a conversion.

## Conversion Process

1. Convert one `.1` or `.3` source using `ConvertManToAsciidoc.py`.
2. Compare the old and new source with `CompareDocText.py`.
3. Review the `.adoc` and render it with `RenderAsciidoc`.
4. Replace the old source, adjust `UpdateAlways` and installation metadata, and
   run `maint/UpdateAlways doc`.

Pandoc was considered as a starting point, but the narrowly-scoped Python
converter preserves PCRE2's small troff subset more predictably for these
trials. The converter remains checked in only while the transition is active.

## Next Steps

Extend the converter for the remaining documented troff constructs, convert
each page individually, and continue comparing text before removing its troff
source. Once every page is converted, simplify `UpdateAlways` so all manpages
and HTML documents are rendered from AsciiDoc and retire the troff-only tools.