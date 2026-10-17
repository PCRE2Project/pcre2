# AsciiDoc conventions for the PCRE2 manuals

The manuals use standard AsciiDoc with a small semantic vocabulary for API
names, source code and pattern syntax. The goal is readable source and
consistent HTML and terminal presentation, not a machine-readable API schema.
Meaning must remain clear without colour.

## Rendering and styles

`AsciidocConverterPcre2.rb` supplies shared Asciidoctor extensions and separate
HTML and manpage converters. HTML builds on `asciidoctor-html5s`; both backends
use Rouge for explicitly typed programming-language source. Rendering fails on
documentation warnings.

There are two HTML consumers:

* `UpdateAlways.rb doc` generates complete standalone manuals, manpages and
  plain-text documentation from `doc/*.adoc`.
* `BuildPages.rb` builds the Jekyll website. Its plugin renders the same sources
  as HTML fragments in-process and uses Kramdown for Markdown. It does not read
  the generated standalone HTML. Imported document bodies are not Liquid
  templates.

For Markdown inside raw HTML table cells, use `markdown="block"` on the cell.

Local manual links are adjusted after rendering: `./target.html` offline,
`./target/` from the website index, and `./../target/` between website manuals.
Author normal AsciiDoc cross-references; do not change source links for either
output.

Styling has three owners:

| Source | Responsibility |
| --- | --- |
| `doc/html/css/document.css` | Shared brand, typography/reset, content and source-token styles |
| `doc/html/css/standalone.css` | Standalone document layout |
| `pages/css/style.css` | Website layout and font loading |

Standalone manuals load the two CSS files directly.
Shared SVGs live in `doc/html/images`. Standalone manuals are JavaScript-free,
using CSS character cards with anchor positioning and a basic relative-position
fallback. They work offline using
installed fonts and fallbacks; only the website ships Fira fonts and their
licence.

## Inline vocabulary

Use a macro for one public entity, and a code span for an expression or fragment.
These short-form macros do not take trailing `[]`.

| Form | Meaning | Example |
| --- | --- | --- |
| `func:` | PCRE2 function, including its parentheses | `func:pcre2_match()` |
| `type:` | PCRE2 type or conventional `_t` type | `type:PCRE2_SIZE` |
| `const:` | Upper-case constant, option or error name | `const:PCRE2_UTF` |
| `arg:` | Lower-case parameter, callback or member name | `arg:match_data` |
| `char:` | Hexadecimal character or Unicode code point | `char:U+000A` |
| `macroval:` | A supported numerical default or limit from the sources | `macroval:MAX_NAME_SIZE` |

In HTML, `char:` shows a character card on hover or keyboard focus, with its
Unicode name and code point. Hexadecimal forms use the Unicode interpretation;
non-printing characters use a box, and surrogates or out-of-range values are
labelled explicitly. Manpages retain only the original notation.

`macroval:bytes,NAME` formats a byte count with exact binary units where
possible. Keep other units in the prose. Supported names are listed in the
converter; unknown or ambiguous definitions fail rendering. Examples, historical
values and recommendations should retain literal numbers.

Use prefix roles on constrained monospace spans:

| Form | Use |
| --- | --- |
| `[.c]` | C expressions, calls, casts, identifier families and other fragments |
| `[.regex]` | PCRE2 pattern syntax |
| `[.subject]` | Literal input or matched text |
| `[.repl]` | Replacement syntax or literal replacement results |
| Plain backticks | File names, commands, diagnostics and other neutral computer text |

For example:

```adoc
Call func:pcre2_match() with arg:subject of type type:PCRE2_SPTR.
The ending offset is [.c]`ovector[1]`.
The pattern [.regex]`+cat|dog+` matches [.subject]`cat`.
```

Do not put macros inside source blocks or add separate macros for every kind
of API fact. Context supplies distinctions such as input/output, deprecation
and experimental status.

### Literal syntax and placeholders

Use backtick-plus spans when punctuation must pass through literally:

```adoc
[.regex]`+(*MARK:NAME)+`
[.regex]`+\Q...\E+`
```

Use ordinary backtick spans when attributes or emphasis must be processed:

```adoc
[.regex]`(?<__name__>[a-z]{plus})`
[.repl]`${_n_:{plus}__text__}`
```

Placeholders are italic; use double underscores when surrounding syntax would
prevent ordinary emphasis. Keep grammar delimiters, but put presentation quotes
or slash pairs outside the span. Character attributes such as `{backtick}`,
`{plus}`, `{backslash}`, `{blank}` and `{startsb}` avoid conflicts with AsciiDoc
parsing. Check the rendered text when combining these forms.

## Blocks and lists

Declare a source block's actual language; never choose one just for colour.
Untyped blocks and `text` remain unparsed. `regex`, `subject` and `repl` are
semantic forms, not programming-language lexers.

```adoc
[source,c,role=prototype]
----
int pcre2_match(const pcre2_code *code, PCRE2_SPTR subject,
                PCRE2_SIZE length, PCRE2_SIZE start_offset,
                uint32_t options, pcre2_match_data *match_data,
                pcre2_match_context *mcontext);
----
```

Use `prototype` for API declarations, `synopsis` for command synopses, and
`wide` only for the fixed-column demo listing. Source/listing blocks use `----`;
literal preformatted material uses `....`. Add `subs=normal` only when
substitutions are intended, for example in a pattern containing placeholders.
Do not use literal blocks to imitate structured lists.

Description lists suit parameters, options, return cases and syntax inventories:

```adoc
[.parameters]
arg:code::
    The compiled pattern.
arg:subject::
    The subject string.
```

The corresponding list roles are `parameters`, `options`, `values`, `returns`,
`errors` and `members`. Use `::` for top-level terms and `:::` for nested terms;
keep each term and delimiter on one source line. Consecutive terms may share a
description; lists with multi-term entries are spaced. The renderer chooses
label width and compact/spaced presentation;
an integer `width` attribute can override the label-width estimate.

Use ordinary lists for narrative sequences, continuation `+` and open blocks
for attached material, and `[loweralpha]` when lettered conditions matter.
Reserve tables for genuinely two-dimensional information; specify `cols` and
`options="header"` where appropriate.
Two-column lookup tables can add the `fit-first` role (for example,
`[.fit-first,cols="2,3"]`) to fit an unbroken first column and fill the remaining
width with the second. Below 600px, the columns stack and long names can wrap.

Use standard `NOTE`, `WARNING` and `IMPORTANT` blocks sparingly. Reuse standard
AsciiDoc structure rather than raw HTML or backend-specific passthrough tricks.

## Source presentation

Rouge tokens and shortcut macros share presentation profiles. C token fixups
recognize PCRE2 types/constants and function calls; they are naming heuristics,
not a C parser. Other identifiers may receive variable treatment. A `*` or `&`
operator not followed by whitespace is treated as punctuation.

| Category | Typeface | Colour when highlighted |
| --- | --- | --- |
| Keywords and operators | Roman | Secondary 3 |
| Literals | Roman | Secondary 2 |
| Preprocessor text | Roman | Secondary 2 |
| Named and keyword constants such as `NULL` | Bold | Secondary 2 |
| Types and `const` | Roman | Inherited |
| Functions | Bold | Theme |
| Comments | Italic | Neutral |
| Variables and parameters | Italic | Secondary 1 |
| Other tokens and neutral text | Roman | Inherited |

Manpages use the same typeface profiles without colour. Semantic regex,
subject and replacement text is unparsed: upright, theme-coloured HTML and
italic manpage text. Explicit highlighting uses the same colours inline and in
blocks, except that headings and contents links override pattern colours.
API shortcodes retain their token's weight and slant but inherit the
surrounding colour. Essential meaning must not rely on these styles.

## Headings and links

Use the normal inline vocabulary in prose headings. Manpage document titles,
such as `= pcre2_match(3)`, remain plain.

Give referenced sections explicit, stable, lower-case hyphenated anchors:

```adoc
[[non-printing-characters]]
=== Non-printing characters
```

Within a document, `<<non-printing-characters>>` uses the target heading as its
label. For another document, use
`xref:pcre2pattern.adoc#non-printing-characters[Non-printing characters]`;
match the heading's wording and formatting. Label whole-document links with
the document name.

Author credits use ordinary paragraphs and explicit `+` line breaks where
needed. Mark revision headings `[discrete]` so they stay out of the contents.

## Building

Generate distributed documentation with `ruby maint/UpdateAlways.rb doc`
(requires the converter gems and man/groff tools).

For the website, install `pages/Gemfile` with Bundler, then run
`ruby maint/BuildPages.rb --base-url https://example.org/pcre2/`
or set `PAGES_BASE_URL`. Node/npm runs pinned Pagefind; `--no-pagefind` skips
indexing for previews. The site lockfile selects Jekyll-compatible Rouge 4.x;
the standalone converter can also use Rouge 5.

After structural changes, inspect both HTML and manpage rendering. In
particular, check literal syntax, placeholders, nested lists, tables and links.
