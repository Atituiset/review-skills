---
name: dispatch
context: fork
agent: review-worker
description: Map review units and observed pointer signals to applicable CWE skills without deciding whether a vulnerability exists.
---

# dispatch

## Stage worker boundary

Execute this Skill only inside this stage's isolated subagent. Main delegates via
the real Agent tool; it must not run these instructions inline. Read
[stage-agent protocol](../../../references/stage-agent.md) first. Use the supplied
PACKAGE, TARGET, SESSION and INVOCATION_ID; load inputs from the Session yourself.
Execute only this stage, persist its status/artifacts and handoff, then return.
Do not execute the next stage or spawn grandchildren. If invoked as plain Markdown
in Main, request proper delegation instead of doing stage work.

## Stage work

Input: units, preloaded contexts, changed code when applicable. Output: dispatch/plan.json.
Start stage running. Read [installed rules](../../../references/rules.md) and honor
the user's explicit CWE selection; otherwise consider all installed rules. Inspect
facts for nullable-pointer use (476), lifetime end and later alias use (416), and
owned allocation with cleanup/transfer obligations (401). Persist reason and fact
references for each applicable unit × rule. One unit may receive multiple rules.

Signals only establish applicability. Distinguish pointer types from iterators,
references and overloaded operators using declarations; dispatch uncertain cases
with an explicit type question rather than claiming risk is confirmed. For functions
returning nullable pointers, the applicable review question can include how callers
use that return, while final findings remain constrained to the requested sink scope.

Persist one entry per unit. A unit with no relevant signal has an empty skills list
and a trace reason. Only catalog rules may run. Unknown requested rules are explicit
coverage errors and make dispatch partial; never silently drop the request. Dispatch
every applicable unit before marking this stage completed.
