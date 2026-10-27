---
name: preload
context: fork
agent: review-worker
description: Persist cheap shared file and function context for review units without eagerly exploring every caller or callee.
---

# preload

## Stage worker boundary

Execute this Skill only inside this stage's isolated subagent. Main delegates via
the real Agent tool; it must not run these instructions inline. Read
[stage-agent protocol](../../../references/stage-agent.md) first. Use the supplied
PACKAGE, TARGET, SESSION and INVOCATION_ID; load inputs from the Session yourself.
Execute only this stage, persist its status/artifacts and handoff, then return.
Do not execute the next stage or spawn grandchildren. If invoked as plain Markdown
in Main, request proper delegation instead of doing stage work.

## Stage work

Input: target, units and baseline. Output: context/files and context/functions records.
Record stage running first. Read selected source and relevant local declarations
through semantic-query. Capture macros, global declarations, typedefs/aliases and
function definitions needed to interpret the target. Record include paths as leads,
not proof that all included code or compile configurations have been loaded.

Each context record cites exact baseline source dependencies and its query_refs. Reuse an
unchanged context, including across rules. Empty macros/globals/typedefs arrays mean
none captured in the stated context, not proof of their global absence; explain gaps.
Initialize function records with the complete body location and file dependency.

Do not preload entire call graphs. Specialized return contracts, callers, aliases
and guards are lazy queries initiated by a review rule. Record partial if necessary
common context could not be captured, and complete when the declared scope is ready.
