---
name: atomic-query
description: Map semantic queries to existing Codegraph and source tools, disambiguate graph candidates and expose factual limitations without making CWE judgments.
---

# Atomic Query

Only this Skill selects backend tools. Read the actual configured tool's help and
capabilities: a checkout's source and installed dist may differ even at the same
version. Prefer available Codegraph MCP; otherwise call its built CLI directly:

```bash
DO_NOT_TRACK=1 node "$CODEGRAPH/dist/bin/codegraph.js" query SYMBOL --path "$PROJECT" --limit 1000 --json
DO_NOT_TRACK=1 node "$CODEGRAPH/dist/bin/codegraph.js" callers SYMBOL --path "$PROJECT" --limit 1000 --json
DO_NOT_TRACK=1 node "$CODEGRAPH/dist/bin/codegraph.js" callees SYMBOL --path "$PROJECT" --limit 1000 --json
DO_NOT_TRACK=1 node "$CODEGRAPH/dist/bin/codegraph.js" node SYMBOL --file PATH --path "$PROJECT"
DO_NOT_TRACK=1 node "$CODEGRAPH/dist/bin/codegraph.js" explore "QUALIFIED_SYMBOL RELATED_SYMBOL" --path "$PROJECT"
```

PROJECT is the complete baseline repository view. Never create a one-file index for
a cross-file question. Use explicit project paths from Session meta.json. Follow
[worktree baseline](../../../references/worktree-baseline.md) for the cache lease and
index binding. Only pre-process prepares/updates the cached index; downstream workers
return a stale-index block for recovery instead of changing it. Never borrow the
target/ancestor index or copy a second repository into Session;
never silently mix source versions. Record missing build configuration separately.

Save raw output before extracting facts. The built version may emit flat callers/
callees unions, grouped definitions, plain-text missing-symbol messages, or truncation
on stderr. Inspect the actual shape. An exit code of zero does not guarantee JSON,
resolved symbol identity or complete results. Flags such as callers --file are only
allowed if this exact build's help supports them. Don't emulate missing flags by
silently filtering away alternative definitions.

## Mapping and fallback

**find_definition**: query or node on a qualified name. Filter exact identity/path
matches; inspect all overload candidates and declarations. If absent, repository
literal identifier search (rg -n -F) plus scoped source reading finds declarations,
inline bodies, macros or aliases. Record text-candidate until declaration/type is
established. A declaration without implementation is missing evidence, not non-null.

**find_callers / find_callees**: use the corresponding graph operation, then node or
explore for the particular caller/callee and argument expressions. Preserve graph
edges as candidate relations. Verify receiver type/signature at source when ambiguous.
If graph edges are absent, stale or merged across overloads, search identifier uses
in the full repository and inspect each relevant callsite and its surrounding body.
Return exact call text and candidate destinations separately. Empty graph results
never prove no callers exist. Callback/virtual/function-pointer resolution can remain
partial, with candidates and the unresolved target recorded.

**find_references**: use node/explore symbol relationships where provided, supplemented
by identifier search across repository source. Classify concrete uses with surrounding
scope and declaration; matching spelling in comments, strings, shadowed locals or
unrelated receiver types is not an identity-resolved reference. Return these as
text-candidate/ambiguous or discard with a recorded reason. Don't promise an exhaustive
symbol reference index when the tool only supplies calls.

**find_assignments**: obtain the complete enclosing function and relevant declaration
via node/explore. Observe initializers, direct assignments, resets, reference/out-arg
calls, captures and visible alias writes. If ast-grep is already available, it can
supply syntax matches; otherwise inspect line-numbered source. Preserve source ranges
and mark syntax-observation. A call with pointer/ref arguments can be a potential
write site without proving a write. Expand that callee's definition separately.
Do not infer reaching definitions or nullability here; the CWE rule does that.

**find_pattern**: literal uses rg -F or native source search. Structural kinds
(dereference/null_guard/return/assignment/call) use complete scoped source inspection
or available ast-grep syntax patterns. Capture whole multi-line constructs, including
short-circuit predicates, early returns and enclosing branch braces. Guard presence
is a syntax fact; dominance and feasibility are NOT query outputs. Do not substitute
searching the word `if` for examining guards.

**find_related_symbols**: node/explore neighbors plus referenced declarations, aliases,
member/parameter types and includes from the current body/header. Return only concrete
relationships needed by the query purpose, with source references. Mark unresolved
macro/type configuration rather than inventing a resolved type.

## Reliability boundary

A best-effort fallback is supported but partial when requested semantic resolution
is unachieved. Do not label every operation unsupported because the backend lacks a
same-named API. Equally, don't relabel lexical matching as compiler-grade dataflow.
No vulnerability/confidence fields belong here. Tool failures, unknown versions,
truncation, ambiguity and omissions are propagated through semantic-query.
