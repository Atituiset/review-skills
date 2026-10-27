# Worktree baseline validation — 2026-09-15

Created a temporary Git repository from the three-file cross-file fixture, committed
it and created a separate detached worktree with native git commands. Existing
Codegraph CLI successfully indexed 3 files (11 nodes, 17 edges) in the worktree;
query lookup returned review_lookup in provider.cpp. Status reported an up-to-date
index. The original temporary repository remained clean; the worktree contained only
the expected untracked .codegraph directory. No Session repository copies were used.

Actual CLI status takes a positional path (or current directory), not --path; query
accepts --path. Stage instructions require checking the installed tool's help.

Parsed meta.schema.json and contracts.schema.json successfully as JSON. This is not
a full JSON Schema validation or a five-stage Claude Code run. Lease concurrency,
dirty-source recovery, cache eviction/recreation and legacy resume remain behavioral
test gaps. Historical stage-agent smoke predates the worktree protocol.
