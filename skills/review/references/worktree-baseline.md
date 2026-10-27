# Git worktree baseline

The pre-process review-worker owns preparation; later workers only consume the
recorded view. Use existing git/Codegraph/file tools, never a custom driver.

## Committed source (default)

Resolve the requested revision to a full commit SHA and the repository's canonical
`git rev-parse --git-common-dir`. For a working-tree request, inspect tracked and
untracked changes first; do not silently substitute HEAD for dirty source.

Default CACHE_ROOT to `TARGET/.review/cache/`, alongside `.review/sessions/` so all
review data is managed inside the target project. TARGET is the resolved reviewed
repository root, not the Skill repository or an arbitrary shell working directory.
Record actual absolute paths. A configured external cache remains supported. On
resume keep the recorded worktree path, including legacy external caches; do not
move registered worktrees, rewrite historical evidence paths or create duplicates
merely to adopt the new default.

Before creating an in-project cache, ensure TARGET's source scanner explicitly
excludes the entire `.review/` subtree. Inspect the actual installed Codegraph build's
supported ignore configuration: when supported, merge `.review/` into root
codegraph.json's exclude array while preserving all existing fields. Otherwise use
its documented supported ignore mechanism (for example root .gitignore), preserving
existing contents. Record the exact configuration change and verify exclusion using
supported scanner/status tooling; do not assume dot-directories or Git worktree
boundaries prevent recursive indexing. Configuration edits obey host write permissions
and are disclosed as review setup changes, never silently committed. If exclusion
cannot be established, block cache creation with the concrete reason rather than
silently use an external cache. Do not refresh the target's existing index: it may
already be stale or contain old snapshot symbols, and is never used for this review.
Source-fallback searches in TARGET also exclude .review explicitly.

Use `<CACHE_ROOT>/<full-commit>/` for the complete detached worktree. When absent,
run `git -C TARGET worktree add --detach WORKTREE COMMIT`. When present, verify its
registered worktree identity, common Git directory, HEAD and tracked cleanliness.
Unexpected untracked source invalidates reuse; never reset, clean or overwrite it.
Generated `.codegraph/` metadata is allowed, but must not enter source scanning.
Do not checkout a different commit in a cached worktree.

Serialize use conservatively: before creating, checking or indexing the worktree,
acquire an exclusive sibling `<commit>.lease/` with ordinary atomic mkdir and write
owner metadata identifying the Session and invocation. Keep this lease for the whole
review, across sequential workers. Another Session must wait or return a bounded
blocked result, not update/read an index being written. On interruption retain the
lease. Resume only the recorded owning Session after checking no worker is active;
never steal a lease merely because it is old. post-process releases the lease only
after outputs are published and all workers have finished; remove only its own owner
file and empty directory. If no Session could be established, pre-process releases
its own lease before returning. Cache cleanup is a separate explicit action; never
remove an in-use worktree or automatically prune other Sessions' data.

## Index binding

PROJECT is WORKTREE for every Codegraph call, including MCP project selection.
Git worktree creation does not fetch or copy an index. Use WORKTREE/.codegraph/.
Reuse only after checking the actual installed Codegraph status/help, tool version,
index configuration/exclusions, complete source coverage and no pending changes.
Record the index binding in sibling `<commit>.index.json`: commit, tool_version,
configuration fingerprint, project path and verification time. Session meta.json
records the same binding. A missing/mismatching binding requires validation and
rebuild/update of this cache's index before any query, under the lease. Do not copy
or symlink TARGET/.codegraph, refresh the target index, or accept an ancestor index
from another worktree. Exclude .review, .review-cache and generated index data using
the installed tool's supported configuration; record actual settings, not assumed
flags. If reliable indexing is unavailable, use source fallback and mark degraded
coverage. Git submodules, LFS and generated headers need explicit coverage handling;
a clean checkout alone does not prove those dependencies are available.

## Session metadata and evidence

Write meta.json using [meta schema](../schemas/meta.schema.json). Set session.baseline.source_mode=git-worktree,
commit to the full SHA, dirty=false and repository_view to the absolute worktree.
Session holds queries, facts, findings and reports, not another checkout or index.
Do not create snapshots/ or codegraph-project/ for this mode.
Also bind the execution-manifest.sha256 required by session-workspace. Code/index
identity alone does not validate conclusions after a rule or orchestration change.

For schema 0.2.0 compatibility, dependency.snapshot_path remains the field name:
in git-worktree mode store the absolute WORKTREE/path there, with path repository-
relative and sha256 computed from the actual file. Resolve canonically and ensure
it remains inside the recorded worktree; no traversal/symlink escape. Other modes
retain their Session-relative snapshot paths. Raw tool outputs remain Session-local.
Before query reuse and final publication verify source hashes, HEAD and index binding.

If a cache was removed, pre-process may recreate the same registered baseline at
the recorded path (or explicitly rebind metadata/dependency paths after checking all
hashes), then rebuild the index. `git show COMMIT:PATH` can recover committed evidence
for hash/content checks, but does not restore a graph index. Missing Git objects are
a recovery block, not permission to use current HEAD. Do not claim metadata alone is
a self-contained archive.

## Dirty source and legacy Sessions

For an explicit committed revision, unrelated target edits stay outside scope and
are recorded as exclusions. For requested uncommitted content use live-hashed mode:
record HEAD, dirty=true, TARGET as repository_view, captured tracked binary diff and
the hashes/content of included untracked files and every queried dependency. Preserve
only those necessary files under snapshots/; never copy the whole repository. Use
source fallback rather than mutating the user's index; invalidate graph-wide negative
results and dependent evidence when source changes. Non-Git targets use the same
bounded live-hashed approach with commit=null. Record this fallback in meta.json.

Legacy git-snapshot/filesystem-snapshot Sessions remain readable. Resume their
recorded paths and semantics; do not rename/delete their directories or automatically
migrate evidence. The new meta.json is required for new Sessions, not retroactively
for historical artifacts.
