---
name: cwe-416-use-after-free
description: Review C/C++ object lifetime and alias paths for a use after release or invalidation, with evidence-backed confirmed, excluded or unresolved scenarios.
---

# CWE-416

Execute inside review-execute. Read [rule contract](../../../references/rules.md)
and [semantic-query](../../query/semantic-query/SKILL.md). All facts come through
semantic-query; the rule does not select backend tools. Persist reviews/cwe-416.json
and findings/raw/<rule-unit-id>.json using existing schemas.

## Applicability and lazy investigation

1. Identify an actual object and possible lifetime boundary: free/delete, owner
   destruction/reset, realloc, container erasure/reallocation, or documented release
   wrapper. A function name alone does not establish release semantics. Start from
   an observed release/use or escaped borrowed pointer, not every pointer in the repo.
2. Resolve allocation/owner declarations, release implementation and alias assignment
   through definitions, assignments and references. Track the same object instance,
   not just the same variable spelling. Follow interprocedural argument/return mapping
   only as needed. Preserve participating file hashes using the baseline protocol;
   reference worktree files without copying a second snapshot.
3. Locate a subsequent use that requires a live object (dereference or call with a
   demonstrated lifetime precondition). A mere pointer copy/comparison does not by
   itself demonstrate such a use. Establish feasible control-flow ordering, including
   branches, early returns, exceptions and loops. A post-free null guard does not make
   a non-null dangling alias valid. Reassignment to a new live object may exclude use
   of the old object; do not conflate allocation episodes.
4. Check ownership details before concluding: shared_ptr aliases can retain lifetime;
   weak_ptr.lock must succeed; unique_ptr.get creates a borrow, not ownership;
   moving an owner does not itself destroy its pointee. Vector/iterator invalidation
   requires the operation and relevant capacity/lifetime conditions, not its name.
   Successful realloc may invalidate the old pointer, failure preserves the original
   allocation; unknown size-zero/platform behavior is unresolved unless established.
5. Select only queries needed to settle the next missing relationship. Reuse prior
   rules' factual queries when baseline, request and dependency checks pass. For
   callbacks/threads, require a feasible destruction-before-use schedule plus ownership
   and synchronization evidence; unknown scheduling is not automatic confirmation.

## Decisions

Confirmed requires an established lifetime end, a later live-object use of that same
object through a connected alias, and a feasible path between them. Cite source,
release, use and control-flow roles from the rule contract. Finding location is the
in-scope invalid use; release outside target scope can be evidence. Double-free without
such a use belongs to another rule and is only a scope lead here.

Excluded requires a concrete reason such as use before destruction, a surviving
owner, or reassignment to a different live object. Unresolved preserves unknown
release contracts, alias identity or scheduling with attempted queries and next work.
Never infer exclusion from absence of callers in the index.

Checkpoint after each query/scenario. Use concise evidence-based reasoning_summary,
feasible trigger and actionable lifetime/ownership correction for findings. Do not
emit SARIF, run another stage, or add speculative findings for excluded/unresolved cases.
