---
name: review-execute
context: fork
agent: review-worker
description: Execute dispatched CWE reviews with lazy cross-file evidence expansion, per-scenario checkpoints and partial failure isolation.
---

# review-execute

## Stage worker boundary

Execute this Skill only inside this stage's isolated subagent. Main delegates via
the real Agent tool; it must not run these instructions inline. Read
[stage-agent protocol](../../../references/stage-agent.md) first. Use the supplied
PACKAGE, TARGET, SESSION and INVOCATION_ID; load inputs from the Session yourself.
Execute only this stage, persist its status/artifacts and handoff, then return.
Do not execute the next stage or spawn grandchildren. If invoked as plain Markdown
in Main, request proper delegation instead of doing stage work.

## Stage work

Input: dispatch plan, contexts, baseline. Output: reviews/<rule>.json and raw findings.
Read [installed rules and common contract](../../../references/rules.md). Validate
each dispatched skill against this catalog and load its exact Skill path. Execute
each unit × rule sequentially using [semantic-query](../../query/semantic-query/SKILL.md).
Persist each rule in its catalog review artifact; maintain accumulated units when
checkpointing. Match prior queries before new calls and record cross-rule cache hits.
Continue independent pairs after a rule-local error; record missing/failed pairs and
partial coverage. Never mark a unit reviewed before all its assigned rules resolve.

Create pending scenarios for actual potential sinks, then checkpoint expanding state
and next_queries. After every query update evidence references and the specific missing
question. Cross-file evidence is expected: headers, definitions, callsites, assignments,
alternative branches and wrappers are within scope for context acquisition.

Do not run a fixed exhaustive list of queries. Select the next query that can decide
the current hypothesis. On a failed/ambiguous lookup try the atomic fallback, precise
qualification or scoped source inspection. Only then mark unresolved with recorded
attempts. A user budget, repeated non-progress, unavailable external contract or
unresolvable alias/indirect dispatch can be a concrete stop reason; file boundaries
and a broad first search returning nothing are not stop reasons.

CWE outcomes: confirmed -> raw Finding; excluded -> short refutation with supporting
facts; unresolved -> missing facts and next_queries. Complete stage only if all planned
reviews resolve; otherwise partial, retaining successful results. A rule failure does
not suppress post-process or successful units. Persist exact next work for resume.
