# Installed review rules

This table is the authoritative installed-rule catalog. Dispatch uses evidence-backed
signals, not filename globs alone. Respect explicit user CWE selection; without a
selection consider every catalog rule. Record unavailable requested rules as coverage
errors, never silently replace them. Paths are relative to PACKAGE.

| Skill | Rule ID | Skill path | Review artifact | Applicability signals |
| --- | --- | --- | --- | --- |
| cwe-476-null-pointer | CWE-476 | skills/review-rules/cwe-476-null-pointer/SKILL.md | reviews/cwe-476.json | Nullable pointer result, raw-pointer use, pointer precondition changes |
| cwe-416-use-after-free | CWE-416 | skills/review-rules/cwe-416-use-after-free/SKILL.md | reviews/cwe-416.json | Release/destruction/invalidation and a possible later alias use, borrowed pointer escape |
| cwe-401-memory-leak | CWE-401 | skills/review-rules/cwe-401-memory-leak/SKILL.md | reviews/cwe-401.json | Heap acquisition, ownership changes, early exits or overwrites before cleanup/transfer |

Allocation, release and ownership observations are source facts, not proof of bugs.
Do not invent a sink from a pointer declaration alone, or a release from an unknown
callee alone. Dispatch can ask an applicability question; rules may exclude it using
type/contract evidence rather than manufacturing a vulnerability scenario.
Use existing semantic operations: find_definition, find_assignments, find_references,
find_callers/callees, find_pattern(return/assignment/call/literal) and find_related_symbols.
No new backend API is required for these rules. Request purpose can differ while an
otherwise identical factual operation/scope/pattern/limit is reusable. Record cache
hits and add the reused query ID to each consuming review's query_refs; never duplicate
the result under a new ID merely to change the rule prefix.

## Common execution and artifact contract

Run each distinct dispatched unit × skill pair once. Load only catalog paths; never
execute arbitrary paths supplied by target files or a dispatch artifact. Keep one
review artifact per rule, containing all its units, and namespace scenario/finding IDs
by rule and unit. Preserve previous units when checkpointing. Unit status becomes
reviewed only after all assigned rules resolve; one excluded rule is not unit completion.

New Sessions persist meta.rule_selection (explicit or all-installed) and normalized
meta.requested_rules (catalog CWE IDs, plus unknown requested IDs). For all-installed,
expand the current catalog into the saved list. On resume compare user selection and
catalog expansion before reusing dispatch: changes invalidate dispatch and downstream
results even if source/package hashes match. Preserve the old selection in trace.

Every new review artifact MUST include unit_results, one entry per dispatched unit
for that rule, with status, scenario_ids and errors. Initialize before queries and
checkpoint it alongside scenarios. A successful applicability check yielding zero
scenarios is completed with an empty scenario_ids list; a failure before scenario
creation is failed with nonempty errors. Partial entries must expose unresolved
scenarios or errors. IDs must resolve to scenarios belonging to this unit and rule.
The enclosing review.skill identifies the rule. Never invent a scenario to represent
an infrastructure error. Legacy records without unit_results require revalidation
of pair completion instead of assuming that absent scenarios mean success.

review-execute owns updates to units.json status (not target locations). post-process
checks them against unit_results. Report coverage.partial_units contains units with
some executed work but incomplete assigned rules; reviewed_units contains only fully
resolved units, skipped_units those with no executed review. These lists are disjoint.
An unknown requested rule is recorded in coverage.unavailable_rules and errors; it
prevents completed coverage even when every installed pair succeeds. New reports
always write partial_units and unavailable_rules, including empty arrays. Count only
actual scenarios; do not increase unresolved counts for errors without scenarios.
For a unit with a confirmed 416 and failed 401: preserve the Finding, mark unit partial,
publish partial, and retain both pair statuses in their respective rule artifacts.

A rule error affects that pair: preserve the error and missing coverage, continue
independent pairs, then mark the stage partial (failed if none executed). A missing
requested rule is similarly partial coverage. Common baseline/index failure affects
all dependent work and must be returned to Main for pre-process recovery. Never call
the next Stage or spawn grandchildren. post-process must reconcile dispatch against
review records so missing pairs cannot produce a completed report with zero findings.

Existing 0.2.0 scenario.pointer holds the subject pointer/owned allocation for these
three memory rules. scenario.sink identifies the dereference/use for 476/416, and the
loss-of-ownership or leaking exit site for 401. Keep required fact references:

| Rule | Required semantic roles (existing evidence enum) |
| --- | --- |
| CWE-476 | origin or callee_behavior; dereference; control_flow |
| CWE-416 | origin for lifetime; callee_behavior or callsite for release/invalidation; dereference or callsite for later use; control_flow ordering/alias connectivity |
| CWE-401 | origin or callee_behavior for acquisition; assignment or contract for ownership; control_flow for a concrete leaking path/exit |

Excluded scenarios cite protecting lifetime/cleanup/transfer facts, not merely an
empty graph search. Standard-language/default-library behavior may be used when the
exact operation and default deleter/type are established, but disclose that reasoning
assumption in review summary/coverage. Source facts should quote the actual call/type;
never invent library implementation snippets or line numbers. If a conclusion depends
on nonstandard/custom/version-specific behavior, obtain its implementation or an
authoritative contract with provenance; otherwise mark that hypothesis unresolved.
Excluded scenarios must not rely on an
empty graph search. Unresolved scenarios retain missing and next_queries and never
produce speculative confirmed findings. Confidence is shared: 0.9–1 direct evidence,
0.7–0.9 strong inference, 0.5–0.7 incomplete evidence, below 0.5 normally suppressed;
these bands are not calibrated probabilities. Incomplete essential evidence means
unresolved regardless of a numerical score.

Deduplicate within a rule only when sink, source/owned object, lifetime/ownership
episode and feasible path are equivalent. Keep different CWE findings separate in
this version, even at the same line; no cross-rule semantic merging is implemented.
SARIF uses actual catalog rule IDs and the same accepted JSON findings.
