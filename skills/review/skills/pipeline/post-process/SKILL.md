---
name: post-process
context: fork
agent: review-worker
description: Inspect evidence-backed findings, normalize and conservatively deduplicate them, and publish review JSON with honest coverage and optional SARIF.
---

# post-process

## Stage worker boundary

Execute this Skill only inside this stage's isolated subagent. Main delegates via
the real Agent tool; it must not run these instructions inline. Read
[stage-agent protocol](../../../references/stage-agent.md) first. Use the supplied
PACKAGE, TARGET, SESSION and INVOCATION_ID; load inputs from the Session yourself.
Execute only this stage, persist its status/artifacts and handoff, then return.
Do not execute the next stage or spawn grandchildren. If invoked as plain Markdown
in Main, request proper delegation instead of doing stage work.

## Stage work

When MODE=recover and UPSTREAM_FAILURES is nonempty, run the failure-reporting branch.
Missing upstream artifacts are expected, not new prerequisites to invent. Preserve
any failed stage, prior attempts and usable raw findings/reviews. Mark only dependent
stages that could not run as skipped, with the upstream reason; never overwrite a
failed stage as completed/skipped. Check only findings whose required evidence exists;
omit unsupported claims and report unavailable coverage/errors explicitly. Publish
partial if useful review work survived, failed if none executed. Do not fabricate
empty successful review records to satisfy normal inputs. If the Session cannot be
read or written, return a failed handoff without claiming a published report.

Read [worktree baseline](../../../references/worktree-baseline.md). Resolve dependency
paths according to source_mode; verify worktree HEAD, hashes and index binding before
publication. Release only this Session's cache lease after publishing terminal outputs
and handoff, including failure reports; keep the cached worktree/index for reuse.

Input: raw findings, review records, query facts, dependency source files. Output:
output/findings.json and optionally output/review.sarif. Follow
[contracts](../../../references/contracts.md) and [session-workspace](../../session-workspace/SKILL.md).

Start running even when earlier review stages are partial. Inspect each finding's
schema fields, confidence, rule, sink location and referenced facts. Check each
cross-file evidence hop against the saved source; compare dependency hashes before
publication. Reject stale or unsupported claims, record validation errors and preserve
the raw candidate for debugging. Do not modify facts to make a finding pass.

Read [rule catalog and evidence requirements](../../../references/rules.md). Reconcile
every dispatched unit × rule with its recorded result. A missing/failed pair or an
unavailable requested rule is incomplete coverage, not a clean empty result. Preserve
successful rules' findings even if another rule failed. Validate semantic evidence
roles per rule; do not require a null origin for lifetime or leak findings.

Normalize paths, IDs and severity. Deduplicate only when rule + same sink + same
object/ownership episode + equivalent feasible path agree. Keep different CWEs
separate in this version. Retain separate origins/paths and uncertain duplicates.
Filter unsubstantiated claims; unresolved scenarios stay visible
in coverage and reviews even when they produce no finding. Sort by path, start line,
rule and ID. Record every merge/filter decision in trace.

Report reviewed and skipped units, confirmed/excluded/unresolved counts, limitations,
errors and review_refs. A required unresolved/skipped unit yields partial; no executed
review yields failed. Empty findings never imply whole-project safety. Counts must
match retained review records; excluded is scoped to its documented hypothesis.

Use an existing validator if available without introducing a custom runtime. Otherwise
record agent-inspection and its limits. Finalize report, then stage and session status.
Return output links and meaningful scope limits, not a blanket success claim.

Only this stage knows SARIF. If requested, follow [SARIF mapping](../../../references/sarif.md)
using the same accepted Finding records. Never ask a CWE rule to emit SARIF. JSON is
the default output for this implementation; optional adapters don't change rule inputs.
