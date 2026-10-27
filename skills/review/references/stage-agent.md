# Stage subagent protocol — stage-agent/1

Execution topology: Main Agent -> five distinct Stage subagents in sequence. Each
worker loads exactly its owning Stage Skill and any supporting Query/CWE/Workspace
Skills needed for that stage. Supporting Skills run within that worker; they are not
additional mandatory agents. Do not require children to spawn grandchildren.

This topology corrects the earlier same-agent implementation; existing 0.2.0 code
fact schemas remain compatible. Old runs are historical and do not demonstrate
stage isolation. `stage-agent/1` adds handoffs, not a custom executable runtime.

## Main's dispatch prompt

Use the host's real Agent tool with `subagent_type: review-worker`, creating a fresh
instance for each stage. The installed review-worker agent defines shared execution
boundaries; the assigned Stage Skill defines the stage procedure.
Send the values below explicitly. Do not assume workers inherit conversation history
or auto-load the Skill merely because a path/name is mentioned:

```text
You are the isolated worker for stage <STAGE>, not the Main orchestrator.
Read <ABSOLUTE_PACKAGE>/references/stage-agent.md and
<ABSOLUTE_PACKAGE>/skills/pipeline/<STAGE>/SKILL.md, then execute ONLY that stage.
PACKAGE: <absolute package root>
TARGET: <absolute target repository>
SESSION: <null for initial pre-process; otherwise absolute Session path>
INVOCATION_ID: <unique main-assigned label, e.g. preload-attempt-1>
MODE: <run | resume | recover>
USER_REQUEST: <verbatim request including selected target/revisions/output>
CONFIGURATION: <Codegraph path/tool, requested limits and output format>
PREVIOUS_HANDOFFS: <absolute handoff paths, empty initially>
UPSTREAM_FAILURES: <actual tool/worker errors, if any>
CONSTRAINTS: preserve source; use existing tools, no custom runtime/driver;
            no execution of other pipeline stages; no grandchild delegation.
Write stage status and a handoff under SESSION/handoffs/.
Return only the structured handoff and a brief scheduling summary, not raw code.
```

Invocation labels are not host agent IDs. Main retains the actual returned agent
handle separately when exposed. Don't invent it to fill an output field. A handoff
claiming isolated execution is not proof by itself: the host conversation must also
contain the actual delegation call.

## Worker bootstrap and ownership

Read target repository guidance in your own context. Load the stage Skill and
[contracts](contracts.md). Use input artifact paths, not copied conversation code.
Pre-process creates the Session, initializes all five states and returns its path.
Other workers start by reading session/pipeline metadata and their documented inputs.
Refuse to run with a missing/wrong Session or mismatched stage/target. Do not quietly
create a replacement Session in a downstream stage.

Write your own stage's running status, actual start time and attempt before its work.
Write only your owned stage artifacts and shared query/context/trace records necessary
for that work. Workers run sequentially: no concurrent pipeline.json writers. Finish
owned outputs before setting terminal state. Do not mark another stage completed.
An initial pre-process worker may initialize all pending states; recovery/pre-process
may invalidate affected downstream states explicitly when checking a changed baseline.
post-process owns final report and final session status. Main does not publish them.

Write handoffs/<INVOCATION_ID>.json using [handoff schema](../schemas/stage-handoff.schema.json).
Return the same object to Main. `outputs` are Session-relative artifacts. Summaries
contain status, counts, warnings and missing input, not code excerpts or reasoning
traces. Findings and evidence remain in the Session; Main can read the final report.
Use actual times. Record worker_started / worker_finished trace events with stage and
invocation label. Leave host_agent_id null when the tool does not expose it to you.

Do not run another stage or decide its code-review conclusions. A review-execute
worker may load several CWE Skills in its own context; a query Skill is a capability,
not a new Main Agent. If you discover you were asked to run a stage inline in the main
conversation, request a real worker invocation instead of performing its workload.

## Scheduling, resume and failures

Main schedules only after the prior worker has returned usable inputs. Workers set
`can_continue` based on readiness, not merely “no exception”. Empty dispatch is valid:
review-execute must still be a worker, persisting a skipped/no-applicable-work record;
post-process must also be a worker, producing an honest scoped report.

For partial output with usable inputs, pass limitations onward. If prerequisites are
unusable, Main must not run dependent work or do it itself. Dispatch post-process with
MODE=recover and nonempty UPSTREAM_FAILURES when a Session exists (the failure-reporting
route); it records unreachable stages as skipped
with explicit upstream-failure reasons (the only terminal-stage ownership exception).
If pre-process cannot create a Session, return a handoff in the tool response with
session_path=null; Main reports the concrete blocking error.

A worker needing clarification returns needs_input with its question; Main relays it
and waits, then resumes that stage. A crashed worker/invalid handoff is an orchestration
failure, not an empty successful review. Retry the same stage with a new invocation
only after a corrected input/configuration or a justified transient-failure retry;
otherwise surface the block. Never retry indefinitely or fabricate completed states.

On resume, first delegate a pre-process baseline/checkpoint check. It can validate
previously completed outputs without recreating them, and returns recommended_next
for the earliest invalid/incomplete stage. Main resumes that stage in its own worker
and never reuses a worker for another stage. Preserve error and invocation history.
No subagent tool => explicit orchestration block, no inline fallback.

## Claude Code routing

Install both the review Skill directory and the distribution's agents/review-worker.md
in `.claude/skills/review/` and `.claude/agents/review-worker.md` respectively (or both
at user scope). Main invokes Agent(review-worker), passing the absolute installed
Skill package path. Nested Stage Skills remain package resources. A missing worker
registration is an installation block, not permission to use general-purpose.

Stage files carry `context: fork` and `agent: review-worker` for hosts that register
and invoke them as Skills. Reading that YAML as text does not fork anything; in the
default route the Agent tool creates isolation BEFORE the file is read. The top-level
review Skill deliberately has no fork flag, because Main is the orchestrator.

Official references: [Skills in subagents](https://code.claude.com/docs/en/skills#run-skills-in-a-subagent)
and [subagent types](https://code.claude.com/docs/en/sub-agents). Tool availability is
controlled by the user's host/permissions; Skill prose is not a security enforcement
mechanism. Observe actual Agent calls to verify topology.
