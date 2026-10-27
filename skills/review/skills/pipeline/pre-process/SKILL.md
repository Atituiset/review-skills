---
name: pre-process
context: fork
agent: review-worker
description: Normalize C/C++ review targets into a repository baseline, persistent session and explicit review units before the other pipeline skills run.
---

# pre-process

## Stage worker boundary

Execute this Skill only inside this stage's isolated subagent. Main delegates via
the real Agent tool; it must not run these instructions inline. Read
[stage-agent protocol](../../../references/stage-agent.md) first. Use the supplied
PACKAGE, TARGET, SESSION and INVOCATION_ID; load inputs from the Session yourself.
Execute only this stage, persist its status/artifacts and handoff, then return.
Do not execute the next stage or spawn grandchildren. If invoked as plain Markdown
in Main, request proper delegation instead of doing stage work.

## Stage work

Input: user target and repository. Output: target.json, session.json, pipeline.json,
units.json and meta.json. Read [session-workspace](../../session-workspace/SKILL.md)
and [contracts](../../../references/contracts.md).

Read the [rule selection contract](../../../references/rules.md) and persist normalized
selection in meta.json. On resume compare selection as well as execution-manifest and
baseline; a changed requested rule set invalidates dispatch and downstream results.

Inspect target repository instructions and dirty/untracked state. Resolve the real
repository root; prevent paths escaping it. Normalize file/function/directory/diff
per schema. Do not equate review scope with allowed evidence scope. On resume follow
session-workspace before creating anything new.

Follow [worktree baseline](../../../references/worktree-baseline.md): acquire the cache
lease, prepare/reuse a detached worktree at the commit and validate its local index.
For dirty/non-Git requests establish bounded live-hashed mode as documented there.
Do not create full repository copies in Session or refresh the target's index. Do not mutate
an unrelated Codegraph repository or rebuild its tool binary. If unavailable, record
capability degradation and use atomic-query's source fallbacks.

For file/function: use semantic find_definition and Codegraph file symbols/source
through atomic-query to enumerate functions and overloads. Include source locations;
qualified name alone is not a unique overload identity. For directory: enumerate
C/C++ sources with documented exclusions, then extract units. For diff: obtain named
base/head changed ranges, map intersecting functions, retain removed-code context,
and include callers only as evidence unless they too are in the requested target.
Extraction gaps become explicit file units or skipped units with reasons.

For a new Session only, create all five pending stage records and transition this
stage to running. On resume/recover, preserve completed outputs and attempt history;
update only this stage's check attempt and explicitly invalidated downstream states.
Do not reset all stages to pending. Return recommended_next for the earliest stage
whose inputs/outputs need work after baseline validation. Record
baseline, versions, created time and unit IDs. On success complete this stage; partial
unit extraction records partial. Infrastructure failure records failed; still provide
an error artifact. No vulnerability classification occurs here.
