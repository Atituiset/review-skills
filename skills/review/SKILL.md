---
name: review
description: Orchestrate C/C++ review by delegating each of five pipeline stages to its own isolated subagent. Main Agent only schedules workers, passes session artifacts and summarizes the published report.
---

# Review — Main Agent orchestrator

You are the Main Agent. **Do not execute stage work in this conversation.** Reading a
stage SKILL.md or saying “entering stage” does not create a subagent. Every stage must
have a real host Agent/Task/spawn-agent invocation, a separate worker context and a
returned handoff. This requirement is mandatory, not conditional on task complexity.

Keep this entry in the main context: do not add `context: fork` here. Forking `/review`
once would not isolate the five stages and can prevent the worker from delegating.

## Main Agent responsibilities

1. Read [stage-agent protocol](references/stage-agent.md). Resolve this package's
   absolute path and pass the user's request, target path, constraints and configured
   Codegraph path to a **fresh pre-process subagent**. If the user provides only
   `/review`, let that worker inspect repository state and return a clarification
   request when the target cannot be inferred; do not inspect source yourself.
2. Wait for its handoff. Accept only a real worker return with matching invocation
   and stage, artifact paths and status. Retain the returned session path. Read only
   handoffs/session/pipeline metadata to make scheduling decisions.
3. Spawn a **new subagent for each next stage**, in dependency order below. Pass the
   same Session path, previous handoff paths and any explicit user constraints. Each
   worker reads its own stage Skill and required schemas/resources. Never reuse a
   finished stage's agent as the worker for a different stage.
4. On partial/failure follow the protocol. Resume/retry is delegated to the owning
   stage; Main does not repair its output or perform its missing code analysis.
5. After post-process returns, relay its summary and published JSON/SARIF links.
   You may read the published final report to present it accurately. Do not invent
   additional findings, reconsider source facts or rewrite the worker's conclusions.

| Order | Worker task label | Skill the worker must read |
| --- | --- | --- |
| 1 | review/pre-process | skills/pipeline/pre-process/SKILL.md |
| 2 | review/preload | skills/pipeline/preload/SKILL.md |
| 3 | review/dispatch | skills/pipeline/dispatch/SKILL.md |
| 4 | review/review-execute | skills/pipeline/review-execute/SKILL.md |
| 5 | review/post-process | skills/pipeline/post-process/SKILL.md |

Use Claude Code's **Agent** tool with `subagent_type: review-worker`, registered from
`.claude/agents/review-worker.md` or `~/.claude/agents/review-worker.md`.
Create a fresh instance of that definition for each stage. If it is not registered,
report the missing agent installation; do not silently substitute general-purpose.
Other hosts must explicitly register an equivalent worker using their native format.
Supply the protocol's self-contained task prompt and the absolute Skill
path. Use the main model by default; no hardcoded model, bypass permission mode or
worktree isolation. Workers need shared access to the Session and appropriate tools.

Never inline a stage because it looks small, because a worker is slow, or because
nested stage Skills are not discoverable by slash name. They are package resources:
explicit Agent creation + worker reading the path is the supported default route.
If a host actually registers a stage Skill with `context: fork`, that invocation is
also acceptable, but do not combine both routes and double-run it.

Main must NOT query Codegraph/rg, read target source/diffs, build indexes, create a
review baseline, preload context, dispatch CWE signals, reason about null paths,
validate evidence or publish stage outputs. Those are worker responsibilities.
Main may relay a worker's missing-input question or actual host permission block.

If no capable subagent tool is available, state that required stage isolation cannot
run and stop this workflow. **Do not fall back to main-agent execution.** Retain actual
worker handles from tool results when exposed; never fabricate identities or claim
that old same-agent reports validate this topology. Show concise start/finish updates
for each worker so the user can distinguish real delegation from narrated stages.
