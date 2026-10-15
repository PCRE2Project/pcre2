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

HTML rendering uses the `asciidoctor-html5s` gem through `Pcre2HtmlConverter`,
registered as the `html5` backend in `maint/AsciidocConverterPcre2.rb`. The
subclass adds list layout semantics to the backend's HTML. It produces semantic
HTML with fewer presentation wrappers; PCRE2 owns the stylesheet rather than
using the default Asciidoctor styling. The gem's typography substitutions apply to both HTML
and manpages: `--` becomes an en dash and `---` becomes an em dash in ordinary
prose. The manpage converter remains separate.

The extension supplies shared defaults before the document header is parsed:
`lang=en`, `source-highlighter=rouge`, and, for HTML only, `nofooter`,
`stylesheet=css/pcre2.css`, `linkcss`, and `docrole=offline-manual`. Document
headers can override or unset these defaults, and CLI/API attribute locks are
respected. HTML uses Asciidoctor's Rouge integration; the manpage converter
uses the same source-language policy and token classification. Rouge supplies
HTML token classes, but its stock stylesheet is disabled; token colours live in
our shared CSS.
Rendering rejects documentation warnings.

### The two HTML consumers

`pages/maint/Build.py` calls `maint/AsciidocConverterPcre2.rb --embedded
--output-dir content/doc` with all `doc/*.adoc` sources in one Ruby process.
Asciidoctor calls this **embedded** mode (`header_footer: false`); **standalone**
mode means a complete HTML document, and is used for distributed manuals instead.
The site builder adds Hugo front matter and renames the index to `_index.html`.
Hugo consumes only HTML and Markdown, never AsciiDoc or `doc/html`. The source
stock header is suppressed in fragments; the HTML postprocessor supplies the
manual title (using the site's page name), source version and revision date.
Hugo still supplies the site shell and browser page title. TOCs and footnotes
remain in the fragment.

A Nokogiri HTML5 DOM pass normalizes local anchor `href` attributes after
conversion, including authored links, linked images and raw HTML, not just
Asciidoctor xrefs. Known flat manual links become `./target.html` offline,
`./target/` from the Hugo manual index, or `./../target/` between Hugo manuals.
Index links become the Hugo documentation section root, including any query or
fragment. Sibling pages need the parent step because Hugo adds a directory level
for each manual. Same-page fragments, absolute URLs, mailto and root-relative
links are untouched. Unrelated local files retain their path and suffix.
Do not change correct source xrefs or URLs to accommodate either output.

`maint/UpdateAlways.rb doc` generates complete documents and bundles CSS in
`doc/html/css`. Standalone HTML requires no network access and does not ship
font assets: it prefers installed Fira fonts, with sans-serif and monospace
fallback stacks. Shared light/dark SVG sources in `doc/assets/images` are mounted
by Hugo and copied into `doc/html/images`. Standalone manuals have a minimal
Index/homepage navbar, an index-page banner, and a full-width footer band.
Both sites provide color-scheme and theme-color metadata.

The shared source is `doc/assets/css/document.css`: colours, font stacks,
ragged-right text without automatic hyphenation, block rhythm, lists, tables,
admonitions, TOCs, and Rouge colours. This is deliberately a global typography
and reset stylesheet, not isolated to `.prose`: plain HTML establishes the same
norms in both consumers. Font sizes use `rem`; line heights and vertical spacing
use the root line height (`rlh`, 24px at the 16px root). Horizontal indents use a
16px `--tab`, usually in pairs. Semantic document sections use `display: contents`
within `.prose`; homepage/layout sections are not flattened. First-heading
spacing explicitly accounts for the retained DOM ancestry.
Ordinary Goldmark lists use the default non-compact spacing; definition lists
remain stacked. Asciidoctor definition labels use separate lines for aliases,
with the last short label aligned to the description's first baseline. A common
double-tab indent is selected using the existing label-width heuristic;
`--tab-width` is an integer tab count. The browser wraps long terms above the
description, without a `long-term` classification. An 8px minimum column gap
prevents touching text; wrapped descriptions have no extra row gap and retain
the same indent. The description's flex basis reserves 0.25px of packing
tolerance against subpixel rounding; flex-grow restores its intended width.
Labels within that tolerance of the wrapping threshold may remain inline with
a correspondingly small indent shift. Extremely narrow columns cap the indent
at half the available width.
Manpages retain their existing width heuristic and formatting, except that
multi-term labels now appear on separate lines rather than comma-joined.

Native tables fill the reading column, preserve Asciidoctor column proportions,
wrap long cell content, and have consistent padding and striping. Asciidoctor's
table wrapper retains scrolling for exceptionally wide content. Rouge and Chroma
remain independent lexers but use the same small token theme in the shared CSS.
They already emit mostly matching Pygments-style class names, so no wholesale
class renaming or replacement lexer is needed.

### Source formatting

Only explicitly typed source is lexed. Untyped listings/monospace, `[source,text]`,
and the semantic `regex`, `subject`, and `repl` forms are never syntax-parsed.
Unknown `[source,LANGUAGE]` labels fail conversion rather than guessing. Hugo
also explicitly disables syntax guessing.

`[c]`/`[.c]` inline code is lexed after Asciidoctor has finished substitutions,
preserving explicit emphasis and links. `[c]` listing blocks are a shorthand for
`[source,c]`; other recognized source-language shorthands work the same way.
For semantic inline spans, keep the role syntax `[.regex]`, `[.subject]` and
`[.repl]`. The corresponding `[source,regex]` forms and listing shorthands provide
styling without invoking a lexer.

Parsed source passes through Rouge and the PCRE2 token fixups before either
backend renders it. Shortcut macros provide the same token annotations without
invoking a lexer: `func:` is `Name.Function`, `type:` is `Keyword.Type`, `arg:`
is `Name.Variable`, `char:` is `Literal.String.Char`, and `const:` is
`Name.Constant`, except for keyword constants described below.

HTML emits Rouge's token classes; the shared CSS groups these into profiles.
Manpages map those same token types to profiles and apply the corresponding
monospace Roman, Bold or Italic font. Adjacent tokens with the same manpage font
are combined into one run. Chroma retains its own lexer and its existing,
compatible token classes, which use the same CSS profiles.

| Profile | Rouge token types / shortcuts | HTML inside `pre` | Manpage font |
| --- | --- | --- | --- |
| Keyword/operator | `Keyword.*` except `Keyword.Type`; `Operator.*`; keyword `const:` values | Secondary 3 (green/yellow), Roman | Roman |
| Literal | All `Literal.*`; `char:` | Secondary 2 (red), Roman | Roman |
| Preprocessor | `Comment.Preproc`, `Comment.PreprocFile` | Secondary 2 (red), Italic | Italic |
| Constant | `Name.Constant`; other `const:` values | Secondary 2 (red), Bold | Bold |
| Type | `Keyword.Type`; `type:` | Secondary 1 (purple), Roman | Roman |
| Function | `Name.Function.*`; `func:` | Theme blue, Bold | Bold |
| Comment | Other `Comment.*` | Neutral grey, Italic | Italic |
| Variable | `Name`, `Name.Variable.*`, `Name.Label`; `arg:` | Foreground, Italic | Italic |
| Semantic pattern | Unparsed `regex`, `subject`, `repl` forms | Theme blue, Roman | Italic |
| Default | Other token types, including text and punctuation | Foreground, Roman | Roman |

Outside `pre`, syntax tokens retain their profile's weight and slant but use
the normal foreground colour. The deliberate exception is unparsed
regex/subject/replacement text: its custom `pcre2-semantic` token keeps theme
blue even inline. Explicit emphasis and links are preserved within annotations;
only explicit placeholders become italic in HTML semantic patterns.

The palette uses `--theme-muted` blue in light mode and `--theme-bright` blue
in dark mode, plus `--neutral-1` grey (`#888888`). Borders and shell shading use
`--neutral-2` and `--neutral-3`. The secondary colours are:

| Variable | Light mode | Dark mode |
| --- | --- | --- |
| `--theme-secondary-1` | Purple `#823ff1` | Purple `#C678DD` |
| `--theme-secondary-2` | Red `#d52753` | Red `#E06C75` |
| `--theme-secondary-3` | Green `#23974a` | Yellow `#ffea6b` |

Secondary 3 deliberately pairs green in light mode with yellow in dark mode.
Print uses black for every profile.
Bold code uses the theme's strong monospace weight; all other profiles use
regular weight.

C lexers cannot distinguish every identifier. Rouge supplements their tokens
with the established PCRE2/uppercase/`_t` naming rules and call detection.
`PCRE2_SIZE`, `PCRE2_UCHAR` and `PCRE2_SPTR`, including the code-unit aliases'
8/16/32 variants, are explicitly recognized as types before the uppercase
constant rule. Flags such as `PCRE2_UTF` and limits such as `PCRE2_SIZE_MAX`
remain constants.
`TRUE`, `FALSE`, `NULL`, `true`, `false` and `nullptr` are normalized to
`Keyword.Constant`. The same rule applies to an explicit constant annotation,
so `const:NULL` and parsed C `NULL` have the same profile.
Other C names receive variable treatment, whether in a prototype or a code
example. Chroma's generic C `n` tokens receive that same variable treatment;
some project types may therefore look like arguments. This lexical limitation
is intentional, not a reason to introduce a C parser.

Manpages use Rouge for declared languages (including C, shell and Perl).
Italic becomes underlining in terminal rendering. Explicit Courier font faces
keep both parsed source and shortcut macros monospaced, including nested
emphasis, without changing ordinary prose emphasis. Asciidoctor callout blocks
retain their protected, unhighlighted roff
path so marker linkage is not disturbed. Technical code in section headings
keeps its original case even when the surrounding manpage heading is uppercased.

`pages/static/css/style.css` owns the Hugo shell and its local `@font-face`
declarations; the font licence stays on the Hugo site only. Fonts come from
Mozilla Fira commit `fd8c8c0a3d353cd99e8ca1662942d165e6961407`.
Fira Sans uses genuine 400/600/700 weights in light mode and 350 (Book)/500/600
in dark mode and print, with matching italic faces. Mono uses its actual
400/500/700 weights. Synthetic weight is disabled; Mono may synthesize italic.
`doc/assets/css/standalone.css` supplies the standalone shell. Each shell owns
its width and outer gutters; brand tokens and content rhythm are shared. Title
decoration belongs to the shared styles, with `.document-title` for shells that
add small, neutral, right-aligned metadata below a heading. Each shell supplies
its own print layout and responsive TOC placement: an inline TOC on narrow
screens and in print, and a sticky, scrollable sidebar when the reading column
and 350px TOC fit at 1290px or wider.
The root reserves a stable scrollbar gutter, so changing page height cannot
change the usable width. Sidebar TOCs also reserve their own scrollbar gutter
to prevent text reflow when their scrollbars appear. Placement uses only the
viewport-width media query, not content size or JavaScript measurements.
Each shell uses matching horizontal grid tracks for its header and main area;
header contents line up with the reading column, leaving the TOC track separate.
The TOC is sticky, but the header and footer remain in normal flow. Print removes
the reserved gutters and returns to an unconstrained reading column.
The default TOC label is "Contents". Its only border is a thin left rule; links
use neutral grey, switching to foreground on hover or keyboard focus. Link
labels stay on one line with ellipses; their full text remains accessible and
is also supplied as a native tooltip. Sticky TOCs have no outer margin; inline
and print TOCs retain their separation from the article.
Hugo alone loads `pages/static/js/toc.js` to bold the topmost visible linked
heading using `aria-current="location"`. When no heading is visible, it retains
the last heading passed at the top of the viewport. Updates are coalesced into
animation frames and respond to scrolling, resizing, page restoration and
initial asset loading without changing focus or the URL. The TOC still works
without JavaScript. Standalone manuals include no script or JavaScript assets.
Hugo mounts the shared source as an asset; the generated `doc/html/css/pcre2.css`
concatenates the shared and standalone styles. Edit the sources, not generated
CSS. The tarball includes the CSS sources but installs only the generated bundle.

Run `ruby maint/AsciidocTest.rb` for converter regression tests. Build the full
site with `HUGO_BASE_URL=https://example.org/pcre2/ python3 pages/maint/Build.py`
(requires Ruby gems, Hugo, and Node/npm for Pagefind).
Run `python3 pages/maint/TestStyles.py` for rendered style regression tests
(also requires the Python `playwright` package and `python3 -m playwright install
--with-deps chromium`). It builds isolated Asciidoctor and Goldmark fixtures and checks
desktop/mobile, light/dark, print, asset ownership, and shell/content behavior.

## Inline public API vocabulary

The converter implements five short-form inline macros. They do not take a
trailing `[]`.

| Macro | Use | Example | HTML role | Manpage presentation |
| --- | --- | --- | --- | --- |
| `func:` | Public PCRE2 function, including `()` | `func:pcre2_match()` | `function` | Bold |
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

## Default values and limits

Use `macroval:NAME` (without trailing `[]`) for numerical defaults and
implementation limits in prose, for example `macroval:MAX_NAME_SIZE` and
`macroval:MAX_VARLOOKBEHIND`. It produces a decimal number with thousands
separators in both HTML and manpages. Use `macroval:bytes,NAME` when the value
is in bytes: it uses exact binary units through GiB, or writes ` bytes` for a
value that is not an exact multiple of 1024. Values in other units, such as
`HEAP_LIMIT` in KiB, must retain their unit in surrounding prose.

`Pcre2MacroValueInlineMacro` in `maint/AsciidocConverterPcre2.rb` lists the
supported names and reads their values from the C sources and
`src/config.h.generic`, not from a locally configured build. Every value must
be a unique, direct numeric `#define`; name a source constant rather than
extracting an expression or an unnamed limit. Unknown names, missing
definitions, and ambiguous definitions fail rendering instead of leaving stale
numbers in the output.

Keep literal numbers in examples, historical descriptions, character codes,
encoding widths, and recommendations that are not implementation limits.

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

The `prototype` role identifies API synopsis blocks and is retained in HTML.
Rouge identifies C keywords and functions; the shared token classifier recognizes
PCRE2 types and constants and treats other names as variables. The same treatment
applies to executable examples, not only prototypes.

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
calculation when a particular list needs it. HTML uses the same width heuristic
and exports `compact` or `spaced` on the `dl`, `ul`, or `ol` itself, including
nested lists. Each definition-list entry groups its term and description in a
`div.dlist-item`; aliases share one term separated by commas. Flexbox puts short
terms beside descriptions, long terms above them, and stacks all terms on small
screens. Inter-item spacing in a `spaced` list equals paragraph spacing; a
`compact` list has none.

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

* Functions, types and constants are bold, and parameters italic.
* Character values and neutral computer text remain monospaced.
* C declarations and source examples preserve their alignment.
* Inline C, regex, subject, and replacement spans remain intact and legible.
* Description lists render as four-en-indented `.TP` entries.
* Literal, listing, and source blocks use the surrounding text margin.
* The `wide` source role temporarily moves content to the physical left margin.
* Admonition labels and content remain visible without decorative styling.
* No essential distinction is communicated only by HTML colour.

The manpage converter uses Rouge for declared programming languages. Blocks
containing Asciidoctor callouts deliberately take the unhighlighted path so
protected callout markup survives. Regex, subject and replacement blocks are
italic without lexing; neutral text remains unparsed and monospaced.

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
