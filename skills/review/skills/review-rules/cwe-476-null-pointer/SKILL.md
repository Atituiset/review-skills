---
name: cwe-476-null-pointer
description: Determine whether a C/C++ pointer can be null at an executable dereference by lazily tracing assignments, guards, callers and callee return paths across repository files.
---

# CWE-476

Input: dispatched units and shared contexts. Output: persistent scenario decisions
and evidence-backed Findings. All facts come through
[semantic-query](../../query/semantic-query/SKILL.md). Tool selection stays in atomic-query.
Read [contracts](../../../references/contracts.md) and the shared
[rule contract](../../../references/rules.md) for structure and confidence bands.

## Applicability and model

Candidate sinks include *p, pointer member/index access and indirect calls. Identify
the type first: an iterator, reference, overloaded arrow, smart pointer or callable
object has different validity semantics. Invalid iterators, dangling pointers and
empty std::function alone are not proof of CWE-476. A memory API can use a pointer
without visible `*`; establish its relevant preconditions before treating it as a sink.

Each scenario needs three connected facts: a possible null origin, an executable
sink, and a feasible path carrying the same pointer value to that sink. Preserve
branch conditions, assignment order, argument mapping and receiver identity. The
review rule, not the query layer, interprets those facts.

## Lazy expansion loop

1. Obtain the entire containing function and declaration/type. Record sink and a
   pending scenario. Identify the first missing fact, not a fixed query checklist.
2. For a local pointer, find_assignments in that scope; trace the initializer and
   later writes. For a returned value, find_definition of the exact callee, then
   find_pattern(return) and the conditions for each return. Follow wrappers and
   same-value aliases. For a parameter, find_callers + find_references and inspect
   actual argument origins and interface preconditions. Do not assume callers obey
   an undocumented contract or assume all pointer parameters can be null in practice.
3. find_pattern(null_guard) on the complete source region and inspect early returns,
   short-circuiting, exception exits, loops, enclosing branches and post-guard writes.
   A guard on q doesn't protect p; a check before reassignment doesn't protect the
   new value. A checked first lookup doesn't necessarily protect a second lookup.
4. Expand find_callees/find_related_symbols as needed for writes, aliases, type
   declarations, macro conditions or invariants. Evidence may come from any file in
   the repository, even for a one-function target. Hash each dependency; reference
   worktree source directly in git-worktree mode, capturing files only in live-hashed mode.
5. Update the scenario's evidence, missing questions and concrete next_queries. Once
   a path is established or refuted, stop that scenario; don't expand unrelated code.

On ambiguous names inspect receiver declarations and all plausible definitions. On
an empty graph use scoped source/reference fallback. On a missing implementation
look for headers/inline bodies, alternate source paths, wrappers and documented
contracts. Record attempts. Only unavailable external evidence, unresolved alias/
indirect dispatch, configuration gaps or a real user limit justify unresolved state.
Do not stop just because the first query or current file lacks the answer.

## Decisions and reporting

Confirmed: provide feasible triggering conditions, sink location and evidence for
origin, dereference, control_flow and every material cross-file hop; cite guards too
when explaining why they don't protect the sink. Don't require runtime execution if
the source path is direct and unambiguous. Don't execute unsafe examples merely to
manufacture confirmation. Findings outside the requested sink scope require a scope
note; return them as context leads unless the user expands the review target.

Excluded: cite a concrete protecting condition or non-null origin and its relationship
to this sink. This is a refutation of one scenario, not a project safety guarantee.
Unresolved: name the remaining factual question, attempted queries/fallbacks and the
next useful query. No speculative high-confidence Finding accompanies it.

Use confidence 0.9–1.0 for direct evidence, 0.7–0.9 for strong inference, 0.5–0.7
for incomplete evidence; normally suppress below 0.5. These are qualitative bands,
not calibrated probabilities. Severity reflects demonstrated impact/triggerability;
do not automatically classify every dereference as critical.

Persist reviews/cwe-476.json with scenario state, evidence and raw Findings. Write a
short reasoning_summary (essential evidence and path, not private chain-of-thought)
and an actionable suggestion. Do not emit SARIF. post-process owns validation,
deduplication and final output. Agent inspection is not automated schema enforcement.
