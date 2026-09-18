# Semantic AsciiDoc markup for `pcre2api`

This document guides humans and coding agents that edit
`doc/pcre2api.adoc`, its stylesheets, or the Asciidoctor support in `maint/`.
It describes why semantic markup is wanted, the deliberately small vocabulary
to use, and the behavior that must be preserved in HTML and manpage output.

## Goals

The markup must:

* Carry enough meaning to support attractive, restrained, colorized HTML.
* Continue to produce highly readable terminal output through Asciidoctor's
  manpage backend. Character-exact preservation of historical roff or terminal
  output is not required; reflowing and indentation changes are acceptable.
* Remain easy to type, read, review, and maintain in source form.
* Prefer simple, standard AsciiDoc. Roles and small custom inline macros are
  appropriate when they remove repetition or expose a useful distinction.
* Serve human and LLM readers. No downstream tool is expected to ingest an
  XML-like model or reconstruct the API from rendered output.

The objective is presentational semantics, not a complete formal schema of the
PCRE2 API.

## Design principles

Use standard document structure before inventing inline syntax. Block titles,
description lists, source languages, roles, and admonitions communicate most
of the useful meaning with standard AsciiDoc.

Add custom syntax only for distinctions that occur frequently in prose and
benefit from consistent presentation. PCRE2 needs much less machinery than a
machine-validated specification such as Vulkan. Git's small domain-specific
extension is a closer model: teach Asciidoctor a few repeated concepts and
leave ordinary prose ordinary.

Do not encode every possible fact. In particular, do not create distinct
inline macros for input parameters, output parameters, options, errors,
callbacks, fields, defaults, deprecated names, or experimental names. Their
surrounding section or list supplies that context.

Color must not be the only distinction. Functions, types, constants, and
parameters should also differ through code font, emphasis, weight, or another
non-color cue. The HTML palette should be readable in light and dark user
styles and should not turn normal prose into a rainbow.

## Inline API vocabulary

Use four short macros for recurring API entities:

```adoc
func:pcre2_substitute()
type:PCRE2_SIZE
const:PCRE2_SUBSTITUTE_GLOBAL
arg:match_data
```

Their meanings and fallback presentation are:

| Macro | Meaning | HTML role | Manpage presentation |
| --- | --- | --- | --- |
| `func:` | A PCRE2 function, including `()` | `function` | Roman |
| `type:` | A public typedef, opaque object, or structure type | `type` | Bold |
| `const:` | A public constant, option, flag, or error name | `constant` | Bold |
| `arg:` | A function argument or named callback/structure member | `parameter` | Italic |

For example:

```adoc
The func:pcre2_substitute() function writes the result to arg:outputbuffer.
Its arg:options argument may contain const:PCRE2_SUBSTITUTE_GLOBAL.
The buffer length has type type:PCRE2_SIZE.
```

Keep punctuation visible in the source. In particular, function targets
include `()` rather than relying on the renderer to add it.

These are short-form Asciidoctor inline macros without trailing `[]`. They
render as backend-neutral quoted inline nodes with roles, not as injected raw
HTML. The HTML and manpage converters therefore render the same source
appropriately.

Use standard constrained monospace roles for lexically highlighted fragments
that are not individual PCRE2 API entities.

An inline C expression or snippet uses the `c` role:

```adoc
The first ending offset is stored in [.c]`ovector[1]`.
Test [.c]`code != NULL` before calling func:pcre2_match().
```

The role identifies the C lexer, allowing the HTML renderer to highlight names,
operators, numbers, strings, and other tokens within the span. The manpage
renders the whole fragment in monospace. Use this for C expressions and
fragments where lexical highlighting adds value, including array access,
pointer member access, casts, and generic C constants. Continue to use the API
macros for individual PCRE2 functions, types, constants, and arguments.

Every inline regular-expression fragment uses the `regex` role, even when it
is only one metacharacter or escape:

```adoc
When using [.regex]`.*` in a pattern, be careful not to consume too much text.
The group [.regex]`(?<name>[a-z]+)` captures one or more lowercase letters.
[.regex]`\R` matches a newline sequence.
```

The `regex` role allows PCRE2-aware, per-character or per-token HTML
highlighting of escapes, groups, character classes, quantifiers, alternation,
and other pattern syntax. It also supplies the hook for a subtle outline and
background around the complete fragment. In the manpage it becomes ordinary
monospace text.

Do not write regex syntax bare in prose or as neutral monospace. This includes
short forms such as `[.regex]`.*``, `[.regex]`+``, `[.regex]`\R``, and
`[.regex]`(*MARK)``. The role attribute precedes the constrained inline span;
do not use postfix forms such as `` `.*`[regex] ``.

Use neutral monospace only for literals and computer text that are neither an
API entity, a C fragment, nor regex syntax:

```adoc
the file `pcre2.h`, the command `make check`, and the literal output `no match`
```

## Function headings, declarations, and examples

When a function is the subject of a section, mark up its name in the heading
in the same way as a reference in prose:

```adoc
[[pcre2-match]]
=== func:pcre2_match()

[source,c,role=prototype]
----
int pcre2_match(const pcre2_code *code, PCRE2_SPTR subject,
                PCRE2_SIZE length, PCRE2_SIZE start_offset,
                uint32_t options, pcre2_match_data *match_data,
                pcre2_match_context *mcontext);
----
```

Always specify `c` for C declarations and examples. The `prototype` role lets
HTML distinguish declarations from executable examples while the manpage
continues to use the full-width source-block rendering provided by
`maint/AsciidocConverterPcre2.rb`. The manpage converter uses Rouge to
highlight C blocks with the same roman, bold, and italic distinctions as the
inline API vocabulary. Rouge supplies the lexical tokens; the `prototype`
role also lets the converter identify parameter names. Generic names in other
C blocks remain roman unless Rouge identifies them more specifically. The
Asciidoctor and Rouge Ruby gems are required by `maint/AsciidocRender`.

Use normal `[source,c]` blocks for C examples. Use a block title when it helps
identify a complete example, but do not caption every small fragment.

## Description lists

Description lists are the default representation for named parameters,
members, options, enumerated values, return cases, and errors. Add a block role
to describe the list as a whole:

```adoc
.Parameters
[.parameters]
arg:code::
    The compiled pattern.

arg:subject::
    The subject string.

.Return values
[.returns]
Positive value::
    The number of captured substrings.

`0`::
    The ovector was too small for all captured substrings.

const:PCRE2_ERROR_NOMATCH::
    No match was found.
```

Use these list roles where applicable:

* `parameters`
* `members`
* `options`
* `values`
* `returns`
* `errors`

The roles are useful HTML and CSS hooks. They intentionally collapse to the
same straightforward tagged-paragraph layout in the manpage.

The manpage converter sizes the term column to the longest term plus one
character, capped at 24 characters. Use an explicit `width` attribute only
when that default is unsuitable, for example `[.parameters,width=12]`.

Do not repeat context in inline macro names. For example, an option and an
error both use `const:`; the surrounding `.Options` or `.Errors` list tells the
reader which it is.

The `PCRE2_INFO_NEWLINE` result is a representative conversion from an aligned
literal block to a semantic list:

```adoc
.Values
[.values]
const:PCRE2_NEWLINE_CR::
    Carriage return (`CR`).

const:PCRE2_NEWLINE_LF::
    Line feed (`LF`).

const:PCRE2_NEWLINE_CRLF::
    Carriage return followed by line feed (`CRLF`).
```

Prefer description lists to tables for one-name/one-description material.
Tables are appropriate only for genuinely two-dimensional relationships where
readers compare several independent columns. Any new table must be checked in
rendered terminal output; stock roff table generation is less robust than
description-list output.

## Source, pattern, and layout blocks

Use source blocks whenever text is code and alignment matters:

```adoc
[source,c]
----
pcre2_match_data *match_data =
    pcre2_match_data_create_from_pattern(code, NULL);
----
```

Pseudocode or byte layouts whose spacing is significant may use a descriptive
role:

```adoc
[source,text,role=binary-layout]
----
00 01 d  a  t  e  00 ??
00 05 d  a  y  00 ?? ??
----
```

Use `[source,regex,role=pattern]` for multiline PCRE2 patterns. This gives the
HTML renderer a PCRE2 regex-highlighting hook and distinguishes the block from
program source while retaining a clear preformatted manpage representation:

```adoc
[source,regex,role=pattern]
----
^(?<key>[[:alpha:]_][[:alnum:]_]*)=(?<value>.*)$
----
```

All regex syntax must be inside either a `[.regex]` inline span or a
`[source,regex,role=pattern]` block. Do not use a C source block, generic
literal block, or unmarked prose merely because a pattern is short.

Reserve `[literal]` for genuinely preformatted non-code text. Do not use it to
simulate a description list, enum table, option list, or admonition. The
PCRE2 converters deliberately treat literal blocks as formatted prose, so the
choice between a source and literal block has visible backend consequences.

## Admonitions

Use standard admonition blocks instead of escaped labels such as
`pass:[NOTE:]`:

```adoc
[IMPORTANT]
====
The unused bits of arg:options must be zero.
====
```

Choose admonitions consistently:

* `NOTE` adds explanation or an important qualification.
* `TIP` gives optional practical guidance.
* `IMPORTANT` states a correctness requirement that is easy to miss.
* `WARNING` is reserved for likely memory corruption, invalid data, security
  exposure, or similarly serious consequences.

Keep status notes as ordinary admonitions without role annotations:

```adoc
[NOTE]
====
The pattern conversion API is experimental and may change.
====
```

Do not mark every occurrence of "must", every precondition, or every minor
aside. Unlike a normative conformance specification, this document does not
need machine-verifiable normative-language macros.

## Cross-references and prose

Use ordinary AsciiDoc cross-references with human-readable labels:

```adoc
See xref:pcre2pattern.adoc[the pattern syntax documentation].
```

Keep stable explicit anchors for sections that other pages reference. Do not
encode navigation into an API-name macro when an ordinary `xref:` expresses
the relationship.

Normal prose should remain normal prose. Reflowing text and changing
indentation are acceptable when they improve source readability. Avoid raw
HTML, backend conditionals used only for styling, passthroughs that merely
escape ordinary punctuation, and formatting tricks that obscure the source.

## Manpage requirements

Roles and colors may disappear in terminal output; meaning must remain clear
from headings, block titles, list structure, font choice, and prose. In
particular:

* Functions remain in roman text, types and constants are bold, and parameters
  are italic.
* Declarations and source examples remain legible in monospace.
* Inline C and regex fragments remain intact and legible in monospace.
* Semantic description lists render as compact, four-en-indented PCRE2 `.TP`
  entries.
* Source examples use the available terminal width and preserve alignment.
* Admonitions retain their label and content even without decorative styling.
* No essential distinction is communicated only by HTML color.

Do not special-case the source document to reproduce historical roff byte for
byte. Do fix regressions that make terminal output ambiguous, excessively
indented, badly wrapped, or dependent on HTML-only content.

## Dos and don'ts

Do:

* Prefer standard AsciiDoc structure.
* Use the smallest semantic distinction that improves presentation.
* Mark C blocks with their language.
* Mark inline C snippets with `[.c]` and every regex fragment with `[.regex]`.
* Use description lists for named reference material.
* Keep custom macros short, readable, and backend-neutral.
* Render and inspect both HTML and manpage output.

Do not:

* Build a complete API schema in AsciiDoc.
* Build an elaborate macro vocabulary or machine-readable markup.
* Add a custom macro for every category of fact.
* Use raw HTML or HTML-only prose.
* Leave regex syntax bare or mark it as generic inline code.
* Use literal blocks as visual substitutes for semantic structures.
* Use tables for simple name/description pairs.
* Preserve obsolete formatting at the cost of readable source or output.
* Rely on color as the sole carrier of meaning.