---
name: session-workspace
description: Maintain persistent multi-file review sessions, baselines, stage checkpoints and query evidence using native file tools under the host agent's control.
---

# Session Workspace

This capability is executed inside an owning Stage subagent, never by Main. Follow
[stage-agent protocol](../../references/stage-agent.md) for stage ownership and handoffs.

Use [contracts](../../references/contracts.md) and the schemas. Write artifacts
with native file tools; use ordinary mkdir/cp/git/date/sha256sum as necessary.
No custom persistence script. A temporary staging directory is allowed when the
host cannot write TARGET yet; record its location and copy final artifacts once
permitted, checking the destination. Never claim writes before they succeed.

Create TARGET/.review/sessions/review-<UTC timestamp>-<unique suffix>/ containing:
context/{files,functions,queries}, dispatch/, reviews/, findings/raw/,
trace/, output/, handoffs/. Assign IDs that remain stable within the session. A hash in a
filename can distinguish same-named symbols; original path stays in the record.
Record real UTC times, commit, dirty state, tool versions and validation mode.

For new Sessions save execution-manifest.sha256 using actual sha256sum output for
the installed root/Stage/Query/Workspace/rule Skills, catalog, references and schemas,
plus the registered review-worker definition when readable. Use package-relative
paths and a separately identified worker path; record any unavailable host definition.
Set meta.execution_manifest to that Session-relative filename. Do not synthesize hash
values. On resume pre-process compares content hashes before reusing conclusions.
If any package/worker content changed, conservatively invalidate affected execution
(all stages when dependency impact is uncertain). Preserve old attempts and artifacts;
use fresh query IDs for replacement facts. Legacy missing manifests mean old results
have unverified instruction versions and require revalidation before reuse. An ongoing
worker seeing package drift returns a block rather than mixing instruction versions.

Baseline preparation follows [worktree baseline](../../references/worktree-baseline.md).
Write meta.json; committed Git reviews use a cached detached worktree and its local
Codegraph index. Session contains no full repository copy. Baseline modes:

- git-worktree: default for committed source, with exclusive Session cache lease.
- git-snapshot/filesystem-snapshot: legacy Session recovery only; preserve old paths.
- live-hashed: capture each fact's exact file bytes and hash, and record graph
  freshness. Check dependencies before reuse/publication; source changes require
  invalidation or a new session. A graph lookup can be incomplete even if files match.

In git-worktree mode dependency.snapshot_path points to the absolute cached source
file; do not copy it into Session. In live-hashed mode preserve only used dependencies
under snapshots/<repository-relative-path>. Hash the actual bytes. Never invent function_hash or
pretend an excerpt hash includes macros/callees. A file hash plus additional named
dependencies is sufficient until a function-level hash is actually calculated.

After each consequential tool call or scenario decision append a trace event with
its actual timestamp. Preserve raw outputs, query request/result IDs and status.
Record summaries of decisions, not hidden chain-of-thought. Checkpoint reviews/
with remaining questions before moving to the next scenario or pausing.

On resume: read session, pipeline, current review records and referenced queries.
Check schema version and baseline; verify dependencies of reusable facts. Resume
from persisted next_queries, not a fresh prompt-only reconstruction. When retrying
failed queries, allocate a new query ID and retain the old error for replay.

Before publishing inspect references and dependency hashes per contracts. With no
actual validator, label validation agent-inspection. Do not claim atomic multi-file
updates or a deterministic cache engine. Write outputs first, then final stage and
session status. Concurrent writers to one session are not supported.
