---
name: cwe-401-memory-leak
description: Review C/C++ heap ownership paths for allocations lost without release or valid transfer, including early exits, overwrites and RAII cleanup.
---

# CWE-401

Execute inside review-execute. Read [rule contract](../../../references/rules.md)
and [semantic-query](../../query/semantic-query/SKILL.md). Obtain facts through that
interface without selecting backends. Persist reviews/cwe-401.json and referenced
findings using existing schemas.

## Applicability and lazy investigation

1. Locate a heap acquisition or owning pointer whose cleanup path changed. Resolve
   allocation wrappers and success/failure behavior. Borrowed parameters, stack objects
   and merely calling a function named create are not proof of owned heap memory.
2. Trace the acquired object through assignments, aliases, returns, out-parameters,
   containers and ownership-taking calls. Establish who must release it, using actual
   implementations/contracts. An unknown callee may take ownership: expand evidence
   or mark unresolved rather than assuming either a leak or a valid transfer.
3. Inspect a concrete path that loses the last usable ownership handle or exits its
   responsible scope without cleanup/transfer. Include early returns, error branches,
   exceptions and loop iterations. A graph search returning no free is insufficient;
   inspect complete relevant source paths. Find release wrappers and RAII destructors.
4. Account for unique_ptr/shared_ptr destruction, scope guards, custom deleters and
   container ownership. Raw-pointer containers do not automatically delete pointees;
   returning a pointer can be a legitimate transfer. Reachable caches/global roots
   and process-lifetime storage are not automatically CWE-401; establish actual loss
   or violated ownership obligation. File/socket leaks alone are outside this rule.
5. For realloc, a failed nonzero-size operation leaves the old allocation live:
   overwriting its sole owning pointer can leak it. A temporary preserving the original
   handle is different. For throwing new, distinguish exception from null-return paths;
   resolve nothrow/custom allocation behavior before using a null hypothesis.
6. Reuse matching prior factual queries; expand only the next missing ownership or
   cleanup relationship. Hash all used dependencies and reference committed worktree
   source according to the baseline protocol. Checkpoint after each material result.

## Decisions

Confirmed requires successful acquisition, a demonstrated ownership obligation, and
one feasible path losing the resource without release or valid transfer. Cite origin,
ownership and control_flow facts; explain why alternate cleanup cannot run on that
path. A leak need not occur on every path. Anchor the finding at the in-scope overwrite,
loss or leaking exit, with acquisition in the evidence flow.

Excluded cites guaranteed cleanup or established transfer for the specific hypothesis.
Unresolved records missing owner/destructor/callee contracts and concrete next_queries.
Do not manufacture negative evidence by searching only the reviewed file. Findings
outside the selected sink scope remain leads. Severity follows actual trigger/impact,
not a blanket critical rating for every allocation. Emit no SARIF or next-stage work.
