# Semantic AsciiDoc markup for the PCRE2 manuals

This document records the conventions used by the PCRE2 AsciiDoc sources and
by `maint/AsciidocConverterPcre2.rb`. It applies to the long-form manuals, the
individual `pcre2_*` API pages, and the command manuals. The source files are
the authority; the custom converter defines the small vocabulary that has
backend-specific behavior.

The conventions below define the semantic markup and source style used
throughout the manuals.

## Goals

The markup must:

* Carry enough meaning to support restrained semantic presentation in HTML.
* Produce highly readable terminal output through Asciidoctor's manpage
  backend.
* Remain easy to type, read, review, and maintain in source form.
* Prefer standard AsciiDoc structure. Roles and the small custom inline
  vocabulary are appropriate when they expose distinctions already present in
  the manuals.
* Serve human and LLM readers. No downstream tool is expected to reconstruct a
  complete API model from rendered output.

The objective is presentational semantics, not a machine-validated
specification of PCRE2.

## Design principles

Use standard document structure before inventing inline syntax. Source
languages, description lists, block roles, tables, and admonitions communicate
most of the useful meaning with standard AsciiDoc.

Do not create separate inline forms for input and output parameters, options,
errors, callbacks, fields, defaults, deprecated names, or experimental names.
The surrounding prose, section, or list supplies that context.

Colour must not be the only distinction. In the manpage converter, functions
are roman, types and constants are bold, parameters are italic, and the other
inline semantic forms remain legible in monospace. HTML receives semantic
classes from the same backend-neutral AsciiDoc nodes.

The semantic vocabulary does not include raw HTML, styling-only backend
conditionals, or `pass:[]` workarounds.

## Inline public API vocabulary

The converter implements five short-form inline macros. They do not take a
trailing `[]`.

| Macro | Use | Example | HTML role | Manpage presentation |
| --- | --- | --- | --- | --- |
| `func:` | Public PCRE2 function, including `()` | `func:pcre2_match()` | `function` | Roman |
| `type:` | Public PCRE2/POSIX type | `type:PCRE2_SIZE`, `type:uint32_t` | `type` | Bold |
| `const:` | Public constant, option, flag, error, or null value | `const:PCRE2_UTF`, `const:NULL` | `constant` | Bold |
| `arg:` | Lower-case argument, callback, or structure-member name | `arg:match_data` | `parameter` | Italic |
| `char:` | Numeric character or code-point value | `char:0x0A`, `char:U+212A` | `character` | Monospace |

For example:

```adoc
The func:pcre2_substitute() function writes the result to arg:buffer.
Its arg:options argument may contain const:PCRE2_SUBSTITUTE_GLOBAL.
The buffer length has type type:PCRE2_SIZE.
The newline character is char:U+000A.
```

Keep punctuation visible in the source. In particular, a `func:` target
contains its `()`; the converter does not add them.

The macro recognizers deliberately accept only the naming shapes implemented
in `maint/AsciidocConverterPcre2.rb`:

* `func:` accepts lower-case `pcre2_` names followed by `()`.
* `type:` accepts upper-case `PCRE2_` names, lower-case `pcre2_` names, and
  conventional names ending in `_t`.
* `const:` accepts upper-case names made from letters, digits, and underscores.
* `arg:` accepts lower-case names made from letters, digits, and underscores.
* `char:` accepts hexadecimal `0x...` and Unicode `U+...` values.

Use these macros for individual public entities in prose and description-list
terms. Use `[.c]` instead for a C expression, a family or prefix rather than one
entity, a width-specific spelling shown as source text, or a non-PCRE2 helper:

```adoc
Functions beginning with [.c]`pcre2_serialize_` form one family.
The width-specific call is [.c]`pcre2_match_16()`.
The allocator calls [.c]`malloc()`.
```

Do not put inline macros inside source blocks. The block's language and role
carry the semantics there.

## Inline code roles

Use constrained monospace spans with a prefix role. Do not use postfix role
syntax.

### C fragments

Use `c` for C expressions and fragments that are not a single public API
entity:

```adoc
The first ending offset is stored in [.c]`ovector[1]`.
Test [.c]`code != NULL` before calling func:pcre2_match().
```

This includes array and member access, casts, pointer expressions, library or
local function calls, and incomplete identifier families. The manpage renders
the complete span in monospace; the HTML retains the `c` role.

### Patterns

Use `regex` for literal PCRE2 pattern syntax, including a single
metacharacter, escape, option, property value when presented as syntax, or
complete pattern fragment:

```adoc
[.regex]`+(*MARK:NAME)+`
[.regex]`\R`
[.regex]`(?<__name__>[a-z]{plus})`
```

Multiline patterns use a `[source,regex]` block instead. Do not use neutral
monospace or a C block merely because a pattern is short.

### Subject and replacement text

Use `subject` for literal input or matched characters:

```adoc
The pattern [.regex]`cat` matches [.subject]`cat`.
```

Use `repl` for replacement-string syntax and literal replacement results:

```adoc
Replace it with [.repl]`${_n_}-copy`.
```

The role describes the characters inside the span. Presentation quotation
marks and slash delimiters sit outside the span; delimiters that are actual
syntax sit inside it.

### Neutral monospace

Use neutral monospace only for computer text that is not one of the categories
above. Examples include file and library names, command-line options and
commands, literal diagnostic/output text, program names, and syntax from
another language when no PCRE2 role applies:

```adoc
the file `pcre2.h`, the command `make check`, and the output `no match`
```

Terms quoted merely as terminology remain ordinary prose, not code.

## Protected and processed inline spans

AsciiDoc substitutions can change literal punctuation. Choose the span form
from its contents.

Prefer the backtick-plus form when the contents must pass through literally:

```adoc
[.regex]`+(*MARK:NAME)+`
[.regex]`+\Q...\E+`
[.repl]`+\u\L+`
```

This protects sequences such as `...`, `--`, braces, and other punctuation
from normal AsciiDoc substitutions.

Use a plain backtick form when substitutions are intentionally required:

* an italic placeholder occurs inside the span;
* an attribute such as `{plus}`, `{backslash}`, `{blank}`, or `{startsb}` must
  expand; or
* a literal leading/trailing single plus or a double-plus sequence would
  conflict with the passthrough delimiter.

Examples:

```adoc
[.regex]`\g<__n__>`
[.regex]`{plus}__n__`
[.repl]`${_n_:{plus}__string1__:__string2__}`
```

In a processed span, escape or replace any AsciiDoc metacharacter that must
remain literal. Choose the protected or processed form from the rules above,
and check the rendered text.

## Placeholders and delimiters

Use italic metavariables. In ordinary prose, write `_n_`, `_name_`, and similar
forms. Inside constrained monospace, a single underscore pair works where the
placeholder has a clear boundary, while double underscores are required when
adjacent syntax would otherwise prevent emphasis:

```adoc
[.regex]`{_n_,_m_}`
[.regex]`(?<__name__>...)`
[.repl]`$<__name__>`
```

Prefer the robust double-underscore form for a placeholder embedded directly
among syntax characters. Both forms render only the placeholder in italics;
the punctuation remains monospace.

Do not add decorative angle brackets around a metavariable. For example:

```adoc
[.regex]`\p{Bidi_Class:__class__}`
```

Keep brackets, quotes, braces, or slashes that belong to the grammar:

```adoc
[.regex]`\g<__n__>`
[.regex]`\k'__name__'`
[.regex]`(?C"__text__")`
[.regex]`+/x+`
```

The first three examples contain syntactic delimiters. The slash in `/x`,
`/xx`, `/s`, or `/v` is part of a Perl-style mode name. A slash pair used only
to display a pattern is not part of a `regex` span.

Use the `{backtick}` attribute when a literal backtick must be shown inside a
constrained span. Use character attributes such as `{plus}`, `{backslash}`,
`{blank}`, and `{startsb}` when literal source punctuation would collide with
AsciiDoc parsing.

## Headings, declarations, and C examples

API names in prose headings use the same macro as prose:

```adoc
=== Option bits for func:pcre2_match()
```

Manpage document titles such as `= pcre2_match(3)` remain plain because the
manpage metadata and synopsis establish their identity.

Function declarations use C source blocks with the `prototype` role:

```adoc
[source,c,role=prototype]
----
int pcre2_match(const pcre2_code *code, PCRE2_SPTR subject,
                PCRE2_SIZE length, PCRE2_SIZE start_offset,
                uint32_t options, pcre2_match_data *match_data,
                pcre2_match_context *mcontext);
----
```

The `prototype` role lets the manpage converter italicize otherwise generic
parameter names while Rouge identifies C keywords, PCRE2 types, constants, and
functions. HTML receives both the language and role classes.

Use `[source,c]` for executable C examples and C declarations that are not API
synopses. The language must describe the contents; source in another language
must not be labelled C.

## Block formats

Use the narrowest block format that describes the content.

* `[source,regex]` is for multiline PCRE2 patterns.
* `[source,regex,subs=normal]` is for a pattern block that intentionally
  contains AsciiDoc emphasis or attributes, normally italic placeholders.
* `[source,c]` is for C code.
* `[source,text]` is for transcripts, diagnostic output, aligned byte layouts,
  annotated source-like displays, and other neutral preformatted text.
* `[source,shell]` is for shell commands.
* `[source,shell,role=synopsis]` is for a command-manual synopsis.
* `[source,c,role=wide]` is reserved for the fixed-column `pcre2demo` listing,
  which must extend from the physical left margin.

Use `----` delimiters for listing and source blocks, and `....` delimiters for
literal blocks. The delimiter supplies the block style, so bare `[listing]`
and `[literal]` style lines are omitted. An explicit style is used when the
attribute list also supplies necessary attributes, such as
`[literal,subs=normal]`.

Use literal blocks for genuinely preformatted non-source material, such as a
quoted preformatted passage or a subject display whose spacing matters. The
manpage converter renders literal, listing, and source blocks in a monospaced
font at the surrounding text margin. Do not use a literal block as a visual
substitute for a description list or a source block.

Do not add a source language solely to obtain colour. In particular,
`pcre2test` sessions and matched-output displays are text, not regular
expressions, even when they contain regex punctuation.

## Description lists

Description lists represent API parameters, options, values, return cases,
errors, and dense syntax inventories. Put the role on the list as a block
attribute:

```adoc
[.parameters]
arg:code::
    The compiled pattern.
arg:subject::
    The subject string.

[.returns]
Positive value::
    The number of captured substrings.

[.errors]
const:PCRE2_ERROR_NOMATCH::
    No match was found.
```

Description-list roles used in the sources are:

| Role | Use |
| --- | --- |
| `parameters` | Named function arguments |
| `options` | Option bits and option names |
| `values` | Enumerated values or information selectors |
| `returns` | Return cases |
| `errors` | Error values |
| `members` | Structure members |

These roles become HTML classes. All description lists use the same
four-en-indented `.TP` presentation in manpage output. The converter chooses a
useful label width from the distribution of visible term lengths, using a
compact fallback when a strict majority do not fit within its maximum. Lists
whose item bodies contain additional blocks have normal paragraph spacing;
simple lists are compact. A numeric `width` attribute may override the
calculation when a particular list needs it.

Use `::` for a top-level term and `:::` for a nested description list. A group
of consecutive terms may share the description attached to the final term.
Keep every term and its terminating delimiter on one source line, including
terms made from several inline spans. Splitting a term can make `::` render as
literal text.

Narrative sequences remain ordinary bulleted or numbered lists. The
`[loweralpha]` style is used when lettered conditions are significant.
Continuation `+` and open blocks `--` are used where a list item contains a
block or nested list. The `[horizontal]` style is used when its compact
two-column presentation is appropriate.

Prefer a description list to a table for one-name/one-description material.
Tables are reserved for genuinely two-dimensional comparisons. Tables declare
`cols` and `options="header"` and are checked in both HTML and manpage output
because roff table layout is less forgiving.

## Admonitions

Standalone callouts use standard block admonitions:

```adoc
[WARNING]
====
Passing invalid UTF with checking disabled can crash or loop.
====
```

`NOTE`, `WARNING`, and `IMPORTANT` are present in the manuals. Use a note for
an explanatory qualification, a warning for a serious unsafe or corrupting
consequence, and important for a correctness requirement that must stand out.
Do not turn every precondition or occurrence of "must" into an admonition.

Narrative warnings may begin with the literal word `Warning:`. Standalone
warnings use a block admonition. Neither form uses `pass:[NOTE:]` or a similar
escaped label.

## Cross-references and ordinary prose

Give every section that is the target of a cross-reference an explicit,
stable anchor immediately before its heading:

```adoc
[[non-printing-characters]]
=== Non-printing characters
```

Use concise lower-case, hyphen-separated IDs. Once an ID is referenced, treat
it as a stable public target.

For a section in the same document, use the short form and let Asciidoctor take
the link text from the target heading:

```adoc
See the section <<non-printing-characters>> above.
```

For a section in another document, use `xref:` with the anchor and explicit
link text:

```adoc
See the section
xref:pcre2pattern.adoc#non-printing-characters[Non-printing characters]
in the
xref:pcre2pattern.adoc[`pcre2pattern`]
documentation.
```

The explicit link text must match the target heading exactly, including its
capitalization, wording, and inline formatting. A link to another document as
a whole uses the document name as its label, as in the second `xref:` above.

Normal prose remains normal prose. Quotation marks used for terminology,
English words, error messages, or cited titles do not by themselves imply a
semantic role. Semantic markup does not change technical wording.

## Backend requirements

Roles and colours may disappear in terminal output; meaning must remain clear
from headings, list structure, font choice, and prose.

* Functions are roman, types and constants bold, and parameters italic.
* Character values and neutral computer text remain monospaced.
* C declarations and source examples preserve their alignment.
* Inline C, regex, subject, and replacement spans remain intact and legible.
* Description lists render as four-en-indented `.TP` entries.
* Literal, listing, and source blocks use the surrounding text margin.
* The `wide` source role temporarily moves content to the physical left margin.
* Admonition labels and content remain visible without decorative styling.
* No essential distinction is communicated only by HTML colour.

The manpage converter uses Rouge only for C source. C blocks containing
Asciidoctor callouts deliberately take the unhighlighted path so protected
callout markup survives. Regex, shell, and text blocks remain monospaced
without C token classification.

## Author and revision sections

Author credits use ordinary paragraphs with explicit line breaks for a short
address:

```adoc
== Author

Philip Hazel +
Retired from University Computing Service +
Cambridge, England.
```

The revision information that follows is a discrete section so it does not
appear in the table of contents:

```adoc
[discrete]
== Revision

Last updated: 25 October 2025 +
Copyright (C) 1997-2024 University of Cambridge.
```

## Review checklist

When editing markup:

1. Choose the semantic category from the text's meaning, not its punctuation.
2. Use a public API macro for one public entity and an inline role for a
   fragment.
3. Decide whether each inline span must be protected or processed.
4. Verify that italic placeholders render as emphasis and syntax delimiters
   remain literal.
5. Give each source block its actual language; use `subs=normal` only when
   substitutions are intended.
6. Keep description-list term delimiters on the term line.
7. Check tables, nested lists, list continuations, and admonitions in context.
8. Render both HTML and manpage output after structural changes.

Do not:

* build a complete API schema in AsciiDoc;
* add a custom macro for every category of fact;
* leave literal pattern syntax bare or mark it as generic code;
* put presentation quotes inside a semantic span;
* use a passthrough around placeholders that must be italicized;
* label non-C source as C or matched output as regex; or
* use literal blocks or tables as substitutes for description lists.
