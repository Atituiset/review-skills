---
name: semantic-query
description: Execute seven repository-scoped fact queries with explicit identity, provenance, dependency-aware reuse, ambiguity and partial-result semantics for review rules.
---

# Semantic Query

This is the interface used by every review rule. Read
[contracts](../../../references/contracts.md), [request](../../../schemas/query-request.schema.json),
[result](../../../schemas/query-result.schema.json), and follow
[atomic-query](../atomic-query/SKILL.md) for tool execution. No custom adapter program.

Supported operations and required scope:

| Operation | Scope / additional inputs | Facts sought |
| --- | --- | --- |
| find_definition | symbol; optional paths/within to disambiguate | exact definitions and declarations, return/body code |
| find_references | symbol + preferably declaration identity | candidate uses tied to locations, not just matching spelling |
| find_callers | qualified callee + definition path/range where known | incoming callsites, caller bodies/arguments |
| find_callees | caller identity/path/range | outgoing calls, candidate target definitions |
| find_assignments | symbol + within function range | initialization, reassignments and syntactically visible writes/aliases |
| find_pattern | pattern.kind; literal text if appropriate | dereference, null guard, return, assignment, call or literal code |
| find_related_symbols | symbol + current context | declarations, types, includes, alias/call neighbors needed next |

An empty scope.paths means the full repository view, not just the target. Resolve
symbol identity by qualified name + declaration path + range/signature. Preserve
multiple candidates when unresolved; never silently choose the first overload or
same-named member. Scope.within bounds local operations; graph expansion may return
related_locations in other files, recorded as dependencies.

For each query write purpose (a factual question, not a vulnerability assertion),
request ID and exact request. Reuse only if operation, normalized scope/pattern/limit,
repository baseline, tool identity AND all saved dependency hashes match. An unchanged
function alone does not validate a query on a changed callee, macro or header. Within
an immutable complete view, record that view plus the actual participating files.
In live-hashed mode graph-wide negative results must be refreshed after any index or
source change, even if previously returned files did not change.

Run atomic-query, preserve raw stdout/stderr or MCP result, then record facts. Each
fact is exact source content with inclusive range, kind and precision. Syntax-derived
assignment/guard/return facts describe code, not whether a guard dominates a sink or
whether a path is feasible. That judgment belongs solely to the review rule. Mark
text candidates and ambiguity explicitly. Include every source file used as a hashed
dependency; in git-worktree mode reference its absolute file in the recorded worktree
without copying it. Follow worktree-baseline.md for path containment and freshness.
Only live-hashed mode captures used source dependencies under Session snapshots/.

Result semantics:

- complete: the requested bounded operation ran. coverage.exhaustive is separate;
  graph/text search normally cannot establish exhaustive C++ semantics.
- partial: usable facts plus a gap (cut, ambiguity, missing dependency, stale relation).
- unsupported: no available tool or documented fallback can perform this operation.
- error: tool execution or output validity failed. Keep the diagnostic, not empty success.

A truncated output is partial with truncated=true and an error describing the cut.
Missing symbol text in otherwise successful CLI output is not valid empty JSON: record
partial/missing_dependency and invoke qualification or source fallback. After fallback,
state which portions became exact-source and what remains best-effort.

Persist dependencies, backend/version, raw_refs and method including all fallbacks.
Append query/cache/retry events. Attach query_refs to file/function contexts. Query IDs
remain session-local and unique; retried errors keep old IDs. Multi-query composition
is executed by the agent under these Skills, not a generated Python/Node driver.
