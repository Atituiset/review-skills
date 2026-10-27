# Contracts and package boundaries (0.2.0)

Execution topology is defined by [stage-agent/1](stage-agent.md): Main only orchestrates;
each of five stages executes in a different real subagent. Same-agent historical runs
do not validate this requirement. Workers own business artifacts and state updates;
Main only passes handoffs and scheduling metadata.

This is a distributable directory of Skills, not an application. Install the entire
review directory plus register agents/review-worker.md in the host's Agent directory.
Nested Skills are loaded explicitly; their logical identities
match the design (pipeline stages, semantic query, atomic query and CWE rules).
No global lookup by a short nested Skill name is required. Future packaging may
expose individual Skills without changing their contracts.

## Artifacts

All schemas live in ../schemas/. contracts.schema.json owns shared $defs; individual
schema files reference them. JSON Schema references resolve locally. No network
access to the illustrative schema $id is needed. Paths are relative to the source
repository or Session as specified; never absolute machine paths in source locations.

| Artifact | Contract | Owner |
| --- | --- | --- |
| target.json | review-target | pre-process |
| units.json | review-unit (collection) | pre-process creates identity/location; review-execute updates status |
| session.json | session | session-workspace |
| pipeline.json | pipeline | each Stage worker owns its entry; pre-process initializes |
| handoffs/<invocation>.json | stage-handoff | each Stage worker |
| context/files/<id>.json; context/functions/<id>.json | context | preload / semantic-query |
| dispatch/plan.json | dispatch-plan | dispatch |
| context/queries/<id>.request.json | query-request | semantic-query |
| context/queries/<id>.json | query-result | semantic-query |
| reviews/cwe-<number>.json | review-record | owning catalog rule |
| findings/raw/<id>.json | finding | owning catalog rule |
| output/findings.json | report | post-process |
| trace/events.jsonl | timestamp, event, stage/skill/query/unit, details | each owner |

Raw tool outputs are stored separately, unmodified. Facts contain exact multi-line
source content at location {path,start_line,end_line,symbol?}; start/end are inclusive.
A fact's raw_ref points to a saved result or snapshotted source supporting that fact.
A dependency records repository-relative path and SHA-256 (hex only).
The historical snapshot_path field is Session-relative in legacy/live-hashed modes;
in git-worktree mode it is an absolute file inside the recorded worktree. Validate
canonical containment, HEAD and hashes using [worktree baseline](worktree-baseline.md).
New Sessions also write meta.json using [meta schema](../schemas/meta.schema.json).
This additive protocol retains legacy 0.2.0 artifacts; older readers that do not
support git-worktree must not consume new Sessions.
A finding references facts by query_id + fact_id and never invents inline evidence.

## Enforced by the executing agent

Schemas define structure. The owning Stage worker (post-process for final outputs)
must also check invariants schemas cannot
express: range order; exact source content; unique IDs; reference resolution; a
query's id matches its request id; five pipeline stages appear once in order;
errors and truncation agree with status; counts match scenario records; confirmed
scenarios reference findings; excluded/unresolved scenarios don't; partial coverage
has reasons. Query unsupported/error => no asserted facts and nonempty errors;
truncated => partial + truncated error. A successful bounded query may still have
coverage.exhaustive=false (e.g. graph resolution is best-effort).

Read [installed rules](rules.md) for rule-specific evidence and output ownership.
CWE-476 findings require origin or callee_behavior, dereference and control_flow
roles, plus evidence for each material cross-file hop. The sink location must match
cited dereference facts. Findings need feasible null-source + sink connectivity,
not merely unvalidated pointer arguments. References/iterators/empty std::function
are not automatically nullable raw pointers. Confidence bands follow the design.

Only existing external validators, if actually available and run, justify
validation.mode=external-validator. Otherwise use agent-inspection and say that
JSON Schema, reference integrity and lifecycle are not programmatically enforced.
No custom validation program should be generated to imitate a runtime.

## Scope and provenance

A ReviewTarget supports file, function, directory or diff. File/function review
normally expands to function units; file units are allowed where extraction fails,
with a recorded limitation. Directory and diff expansion may be partial, never silent.
Diff uses explicit base/head: don't guess HEAD~1. Ask only if the intended revisions
cannot be inferred. Live working-tree diffs must be named as such in session.baseline.

Use a complete repository view for graph indexing, never just the review target.
Committed Git source uses a cached detached worktree with a matching local index,
not a Session repository copy. Dirty/non-Git source uses bounded live-hashed captures;
legacy snapshots remain recoverable. Record excluded files/build products and
configuration. Hash headers and newly visited callers alongside the reviewed file.
Do not claim full preprocessing/build configuration
coverage from a tree-sitter index.

## Failure, persistence and review outcome

Stages: pending -> running -> completed/partial/failed; skipped only with a reason.
Increment attempt on retry. On interruption leave running, with the latest checkpoint.
Resume checks input hashes and last persisted state; it doesn't rerun completed stages
with identical inputs. Changed baselines require a new session or invalidate affected
contexts, queries and conclusions explicitly. Never carry stale findings forward.

Scenario states: pending -> expanding -> confirmed/excluded/unresolved. Keep missing
questions and concrete next_queries. Missing evidence triggers targeted expansion,
not immediate partial. Excluded means the stated scenario is refuted, not global safety.
Unresolved requires an actual missing fact and attempted query/fallback recorded.

One failed rule/unit does not erase others. post-process still publishes completed
work, errors and unresolved coverage. Set session partial if any required unit/scenario
is unresolved; failed if no review could execute. Never mark interrupted work complete.
