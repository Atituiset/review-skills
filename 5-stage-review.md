# 5-Stage Agentic C/C++ Code Review Runtime

> 执行拓扑约束：Main Agent 只负责编排；每个 Stage 必须在独立子 Agent 中加载
> 对应 Skill 执行。阶段间通过持久化 Session 和 handoff 交接，不能在 Main 中
> 读取五份 Skill 后自行完成全部阶段。实现协议为 stage-agent/1。

> 实施范围更新：先完成可安装、全 Skill 驱动的 CWE-476 vertical slice，再套五阶段。
> 当前入口为 [`skills/review/SKILL.md`](skills/review/SKILL.md)，安装时整体复制该目录到
> 使用方的 Skills 目录；子 Skill 和静态 schemas 随包分发，Session 写入目标仓。
> 用户最新约束为全 Skill 指令式执行：不依赖自定义 Python/适配/驱动脚本。
> 因此下文 2.2 节的脚本建议暂不采用，不能宣称具有自动 schema 校验或缓存引擎。
> 0.2.0 已加入五阶段 Skill、七类 Query 工具映射、跨文件补证据和 scenario checkpoint。
> 当前实现与验证边界见
> [`docs/first-slice.md`](docs/first-slice.md)，安装说明见 [`README.md`](README.md)。
> 下文第 26、31 节仍为完整第一版目标；更多 CWE、各目标模式及故障恢复的验收状态
> 以实施对应表和实际验证记录为准，不因已有 Skill 指令就视为全部验收通过。

## 1. Overview

本文定义一个基于 Agent Skills 的 C/C++ Code Review Runtime。

系统目标不是构建一个大型 Code Review Prompt，而是将代码检视拆解为：

* 可组合 Pipeline Skills
* CWE-oriented Review Skills
* Semantic Code Query Primitives
* Persistent Review Context
* Deterministic Scripts
* Evidence-based Findings
* Standardized SARIF Output

核心执行流程：

```text id="twdq89"
Review Target
     │
     ▼
┌──────────────────┐
│ 1. pre-process   │
└────────┬─────────┘
         ▼
┌──────────────────┐
│ 2. preload       │
└────────┬─────────┘
         ▼
┌──────────────────┐
│ 3. dispatch      │
└────────┬─────────┘
         ▼
┌──────────────────┐
│ 4. review-execute│
└────────┬─────────┘
         ▼
┌──────────────────┐
│ 5. post-process  │
└────────┬─────────┘
         ▼
       SARIF
```

五个 Stage 本身均实现为 Skill。

顶层 `review` Skill 负责编排整个 Pipeline。

---

# 2. Design Principles

## 2.1 Query Facts, Don't Make Judgments

Code Intelligence 层只负责获取事实。

例如：

```text id="tyjd8l"
find_definition
find_references
find_callers
find_callees
find_assignments
find_pattern
find_related_symbols
```

查询层不允许返回：

```text id="ebj6h9"
"这里存在空指针漏洞"
```

只允许返回：

```text id="brfbpb"
pointer p assigned from foo()
p dereferenced at line 103
guard exists at line 97
foo() may return nullptr according to definition
```

漏洞判断只能发生在 Review Skill。

---

## 2.2 Separate Deterministic Execution from LLM Reasoning

确定性任务优先使用：

```text id="tfmzjt"
Python
Shell
```

例如：

* 创建 Review Session
* 创建目录
* 计算 hash
* JSON persistence
* cache read/write
* finding merge
* deduplication
* SARIF generation
* schema validation

LLM 负责：

* Scenario matching
* Evidence selection
* Security reasoning
* CWE judgment
* Context expansion decisions

原则：

> If it can be deterministically executed, don't ask the LLM to simulate it.

---

## 2.3 Stable Semantic Query Contract

CWE Skill 不得直接依赖：

```text id="p4mbc2"
codegraph
rg
ast-grep
```

只能依赖 Semantic Query Interface。

即：

```text id="bscs7d"
CWE Skill
     │
     ▼
Semantic Query
     │
     ▼
Atomic Query Skill
     │
 ┌───┼─────────┐
 ▼   ▼         ▼
rg  ast-grep  codegraph
```

未来可以替换为：

```text id="anizql"
clangd
Clang
CodeQL
Joern
Tree-sitter
custom index
```

而不修改 CWE Skill。

---

## 2.4 Persistent Context

Review Context 不应只存在于 LLM Context Window。

需要持久化：

```text id="q6qwtc"
.review/
└── sessions/
```

用于：

* Query reuse
* Skill sharing
* Debugging
* Trace
* Resume
* Replay
* Benchmark
* Future training trajectory

---

# 3. System Architecture

推荐逻辑架构：

```text id="hdhiz7"
                    review
                      │
                      ▼
              Pipeline Runtime
                      │
       ┌──────────────┼───────────────┐
       ▼              ▼               ▼
 Pipeline Skills   Review Skills   Query Skills
       │              │               │
       │              │               ▼
       │              │        Semantic Query API
       │              │               │
       │              │        Atomic Query Skill
       │              │               │
       │              │      ┌────────┼────────┐
       │              │      ▼        ▼        ▼
       │              │  codegraph    rg    ast-grep
       │              │
       └──────────────┼───────────────┘
                      ▼
              Persistent Context
                      │
                      ▼
                   Finding
                      │
                      ▼
                    SARIF
```

---

# 4. Suggested Repository Structure

建议第一版：

```text id="hw9w85"
skills/
│
├── review/
│   └── SKILL.md
│
├── pipeline/
│   ├── pre-process/
│   │   ├── SKILL.md
│   │   └── scripts/
│   │
│   ├── preload/
│   │   ├── SKILL.md
│   │   └── scripts/
│   │
│   ├── dispatch/
│   │   ├── SKILL.md
│   │   └── scripts/
│   │
│   ├── review-execute/
│   │   └── SKILL.md
│   │
│   └── post-process/
│       ├── SKILL.md
│       └── scripts/
│
├── query/
│   ├── semantic-query/
│   │   └── SKILL.md
│   │
│   └── atomic-query/
│       ├── SKILL.md
│       └── scripts/
│
└── review-rules/
    ├── cwe-476-null-pointer/
    │   └── SKILL.md
    │
    ├── cwe-787-out-of-bounds-write/
    │   └── SKILL.md
    │
    ├── cwe-416-use-after-free/
    │   └── SKILL.md
    │
    └── ...
```

Runtime：

```text id="sq0umx"
.review/
└── sessions/
    └── <session-id>/
        ├── session.json
        ├── target.json
        ├── pipeline.json
        │
        ├── context/
        │   ├── files/
        │   └── functions/
        │
        ├── dispatch/
        │   └── plan.json
        │
        ├── findings/
        │   ├── raw/
        │   └── findings.json
        │
        ├── trace/
        │   └── events.jsonl
        │
        └── output/
            └── review.sarif
```

---

# 5. Review Target

所有 Review 入口必须首先标准化为统一 `ReviewTarget`。

支持：

```text id="cq7k3h"
diff
function
file
directory
```

建议 schema：

```json id="04c10f"
{
  "type": "diff",
  "repository": "/workspace/project",
  "base": "main",
  "head": "HEAD",
  "paths": [],
  "functions": []
}
```

单函数：

```json id="fjyn12"
{
  "type": "function",
  "repository": "/workspace/project",
  "path": "src/foo.cpp",
  "function": "Foo::process"
}
```

单文件：

```json id="7gkyh2"
{
  "type": "file",
  "repository": "/workspace/project",
  "path": "src/foo.cpp"
}
```

目录：

```json id="dzw0ln"
{
  "type": "directory",
  "repository": "/workspace/project",
  "path": "src/network"
}
```

Pipeline 后续阶段不关心用户最初如何发起 Review。

---

# 6. Review Session

每次 Review 创建唯一 Session：

```text id="d0vnw8"
review-20260914-221500-a82f
```

`session.json`：

```json id="1ujk25"
{
  "session_id": "review-20260914-221500-a82f",
  "created_at": "2026-09-14T22:15:00+08:00",
  "status": "running",
  "pipeline_version": "0.1.0",
  "schema_version": "0.1.0"
}
```

状态建议：

```text id="0gg7an"
created
running
completed
failed
partial
```

---

# 7. Stage 1 — pre-process

## Goal

将外部 Review Request 转换成稳定、可执行、可追踪的 Review Session。

输入：

```text id="26y6n5"
User Request
Repository
Review Mode
```

输出：

```text id="rmxvly"
ReviewTarget
Session
Review Units
```

---

## Responsibilities

### 7.1 Normalize Review Target

例如：

```text id="wdgf0v"
"review current diff"
```

转换：

```json id="u2x6dq"
{
  "type": "diff",
  "base": "HEAD~1",
  "head": "HEAD"
}
```

---

### 7.2 Determine Review Units

例如 Diff：

```text id="tobiqs"
git diff
   ↓
changed files
   ↓
changed ranges
   ↓
affected functions
```

形成：

```json id="82gg5h"
{
  "units": [
    {
      "type": "function",
      "path": "src/foo.cpp",
      "function": "foo",
      "changed_lines": [[120, 135]]
    }
  ]
}
```

File Review 则枚举文件内函数。

Directory Review 则：

```text id="0w57ys"
directory
 ↓
source files
 ↓
functions
```

---

## 7.3 Initialize Workspace

创建：

```text id="imjqnv"
session/
context/
dispatch/
findings/
trace/
output/
```

---

## 7.4 Record Baseline Metadata

建议至少记录：

```json id="16zyz9"
{
  "repository_commit": "...",
  "dirty": false,
  "review_target": {},
  "review_units": [],
  "tool_versions": {},
  "pipeline_version": "0.1.0"
}
```

未来 Benchmark 极其依赖这些信息。

---

# 8. Stage 2 — preload

## Goal

提前加载：

> Cheap + Common + Reusable

的上下文。

不是把整个 Repository 塞给 Agent。

原则：

> **Eager Common Context + Lazy Specialized Context**

---

## Preloaded Context

当前第一版可以加载：

```text id="6lpyyd"
macros
global variables
typedefs
```

以后可以扩展：

```text id="f0bk9e"
struct
enum
class
namespace
file-level symbols
```

---

## File Context

例如：

```text id="50gd5l"
context/files/src__foo.cpp.json
```

```json id="l36wmc"
{
  "path": "src/foo.cpp",
  "file_hash": "sha256:...",
  "macros": [],
  "globals": [],
  "typedefs": [],
  "structs": [],
  "created_at": "...",
  "schema_version": "0.1.0"
}
```

---

## Function Context

此阶段可以只建立基本 Function Record：

```json id="mqvp64"
{
  "path": "src/foo.cpp",
  "function": "Foo::process",
  "function_hash": "sha256:...",
  "definition": {},
  "queries": {}
}
```

注意：

**preload 不应该执行大量 call graph/reference query。**

这些应该 Lazy Load。

---

# 9. Stage 3 — dispatch

## Goal

判断：

> Which review capabilities are relevant to this review unit?

Dispatch 不负责漏洞判断。

只负责：

```text id="10ip6a"
Review Unit
     ↓
Potential Risk Signals
     ↓
Relevant CWE Skills
```

---

## Input

```text id="s8k1fh"
Review Unit
+
Preloaded Context
+
Changed Code
```

---

## Output

`dispatch/plan.json`

```json id="flj6yg"
{
  "units": [
    {
      "unit_id": "src/foo.cpp::Foo::process",
      "skills": [
        {
          "skill": "cwe-476-null-pointer",
          "reason": "pointer dereference introduced"
        },
        {
          "skill": "cwe-787-out-of-bounds-write",
          "reason": "memcpy call modified"
        }
      ]
    }
  ]
}
```

---

## Dispatch Signals

第一版可以依据：

```text id="pbn56k"
changed API
syntax pattern
pointer operation
memory operation
allocation/free
integer arithmetic
string API
locking
filesystem
network
serialization
```

例如：

```cpp id="6w4cxf"
memcpy(dst, src, len);
```

可能 Dispatch：

```text id="x50zrm"
CWE-787
CWE-125
CWE-190
```

但 Dispatch 只表示：

> Relevant.

不是：

> Vulnerable.

---

# 10. Stage 4 — review-execute

这是整个系统最核心的 Reasoning Stage。

## Goal

针对每个：

```text id="0xt1dd"
Review Unit × Review Skill
```

执行 Evidence-based Review。

---

# 11. CWE Review Skill Contract

每个 CWE Skill 应遵循统一结构。

例如：

```text id="7c7ekg"
CWE-476 Null Pointer Dereference
```

建议包含：

```text id="ftq66x"
Identity
Applicability
Risk Model
Required Evidence
Available Queries
Reasoning Policy
Positive Indicators
Negative Indicators
Stop Conditions
Finding Contract
```

---

## Example

```text id="jsyhf6"
Goal

Determine whether a pointer dereference may occur
when the pointer can be null.
```

Applicability：

```text id="w44jqt"
pointer dereference
member access through pointer
indirect call
pointer-returning API
```

Required Evidence：

```text id="gbj0n7"
pointer origin
assignments
guards
callee return semantics
control-flow context
```

---

# 12. Semantic Query Interface

Review Skill 只能调用：

```text id="rv98ur"
find_definition
find_references
find_callers
find_callees
find_assignments
find_pattern
find_related_symbols
```

例如：

```text id="v8txjz"
find_assignments(
    symbol="ptr",
    scope="Foo::process"
)
```

返回：

```json id="3oj2o3"
{
  "query": "find_assignments",
  "symbol": "ptr",
  "results": [
    {
      "path": "src/foo.cpp",
      "line": 91,
      "code": "ptr = get_object();"
    }
  ]
}
```

不得返回：

```json id="it66pl"
{
  "is_vulnerable": true
}
```

---

# 13. Atomic Query Skill

Semantic Query 是稳定 API。

Atomic Query Skill 负责决定：

```text id="60e4ps"
find_references
      ↓
codegraph?
rg?
ast-grep?
combination?
```

例如：

```text id="j9b3yg"
find_definition
    → codegraph

find_pattern
    → ast-grep

fallback text search
    → rg
```

CWE Skill 完全不知道这一层。

---

# 14. Lazy Context Expansion

Review Skill 不应一次加载所有 Context。

例如：

```text id="92k64d"
看到：

ptr->field
```

第一步：

```text id="5wn2so"
find_assignments(ptr)
```

得到：

```text id="os0kvx"
ptr = lookup(id)
```

然后：

```text id="e6nvxu"
find_definition(lookup)
```

如果发现：

```cpp id="t4cmtw"
return nullptr;
```

再：

```text id="pzrg0p"
find_pattern(null-check of ptr)
```

这形成：

```text id="fs4qhw"
Observation
    ↓
Query
    ↓
Evidence
    ↓
Decision
    ↓
Next Query
```

而不是固定执行 20 个 Query。

---

# 15. Query Cache

每次 Semantic Query 优先查询 Function Context：

```json id="umk6cx"
{
  "queries": {
    "find_assignments:ptr": {
      "status": "complete",
      "results": [],
      "created_at": "..."
    }
  }
}
```

流程：

```text id="ogm1un"
Semantic Query
      ↓
Cache Hit?
  ┌───┴───┐
 Yes      No
  │        │
return    Atomic Query
           ↓
        persist
           ↓
         return
```

多个 CWE Skill 可以共享结果。

---

# 16. Cache Consistency

第一版至少记录：

```text id="38ex4c"
file_hash
function_hash
schema_version
tool_version
pipeline_version
created_at
```

注意：

```text id="tub4bi"
function_hash unchanged
```

并不意味着：

```text id="ow1yhu"
semantic context unchanged
```

因为：

```text id="y5ar6h"
macro
global
typedef
callee
```

都可能变化。

第一版不用解决完整 dependency invalidation。

但数据结构必须为未来留下空间。

---

# 17. Finding Model

不要让 CWE Skill 直接输出 SARIF。

先统一内部 Finding。

推荐：

```json id="eckfbv"
{
  "id": "finding-001",
  "rule_id": "CWE-476",
  "title": "Potential null pointer dereference",
  "severity": "high",
  "confidence": 0.87,

  "location": {
    "path": "src/foo.cpp",
    "line": 103,
    "function": "Foo::process"
  },

  "claim": "ptr may be null when dereferenced",

  "evidence": [
    {
      "type": "assignment",
      "path": "src/foo.cpp",
      "line": 91,
      "content": "ptr = lookup(id);"
    },
    {
      "type": "callee_behavior",
      "path": "src/object.cpp",
      "line": 52,
      "content": "return nullptr;"
    }
  ],

  "reasoning_summary":
    "lookup may return nullptr and no guard was found before dereference.",

  "suggestion":
    "Validate ptr before dereference or establish a non-null invariant.",

  "context_refs": [
    "function:src/foo.cpp::Foo::process",
    "query:find_assignments:ptr"
  ]
}
```

---

# 18. Evidence-oriented Review

每个 Finding 必须尽可能回答：

```text id="87eubg"
WHERE?
WHAT?
WHY?
EVIDENCE?
CONFIDENCE?
HOW TO FIX?
```

尤其：

> Finding 的 Evidence 应该引用 Query Layer 获取的事实。

不要只输出：

```text id="7y5io7"
LLM believes this is dangerous.
```

---

# 19. Confidence

第一版可以简单定义：

```text id="n4t1uw"
0.9–1.0  strong direct evidence
0.7–0.9  strong inference
0.5–0.7  incomplete evidence
<0.5     normally suppress
```

以后再建立正式 Calibration。

不要让每个 CWE Skill 自己发明 confidence 语义。

---

# 20. Stage 5 — post-process

## Goal

将多个 Review Skills 的输出转换成稳定最终结果。

流程：

```text id="4cm0gv"
Raw Findings
     ↓
Schema Validation
     ↓
Normalize
     ↓
Merge
     ↓
Deduplicate
     ↓
Filter
     ↓
Sort
     ↓
SARIF
```

---

## Deduplication

例如：

```text id="5ahk9c"
CWE-787:
buffer write may overflow

CWE-120:
unsafe copy may overflow buffer
```

可能指向同一：

```text id="mqp6hf"
sink + location + evidence chain
```

Post-process 可以根据：

```text id="mxaxbc"
location
sink
source
evidence
rule
```

进行合并。

第一版可以 conservative：

> 宁可保留少量重复，也不要错误合并不同问题。

---

# 21. SARIF Adapter

只有 Post-process 知道 SARIF。

即：

```text id="x4od4c"
CWE Skill
   ↓
Finding
   ↓
Post-process
   ↓
SARIF Adapter
   ↓
review.sarif
```

这样未来可以同时支持：

```text id="u58jyx"
SARIF
JSON
Markdown
GitHub PR Comment
IDE Diagnostic
Web UI
```

而不用修改 Review Skill。

---

# 22. Pipeline State

`pipeline.json`：

```json id="23lf12"
{
  "stages": {
    "pre-process": {
      "status": "completed",
      "started_at": "...",
      "finished_at": "..."
    },

    "preload": {
      "status": "completed"
    },

    "dispatch": {
      "status": "completed"
    },

    "review-execute": {
      "status": "running"
    },

    "post-process": {
      "status": "pending"
    }
  }
}
```

推荐 Stage 状态：

```text id="w8epnz"
pending
running
completed
failed
skipped
```

这为以后：

```text id="5v6u3y"
resume
retry
debug
benchmark
```

打基础。

---

# 23. Trace

建议从第一版就保留：

```text id="7isvl4"
trace/events.jsonl
```

例如：

```json id="eud17u"
{
  "timestamp": "...",
  "event": "skill_started",
  "skill": "cwe-476",
  "unit": "src/foo.cpp::Foo::process"
}
```

```json id="h43hmo"
{
  "timestamp": "...",
  "event": "query",
  "query": "find_assignments",
  "symbol": "ptr",
  "cache_hit": true
}
```

```json id="0u0lmn"
{
  "timestamp": "...",
  "event": "finding_created",
  "finding_id": "finding-001"
}
```

第一版不用设计成复杂 Observability Platform。

但：

> **不要丢执行轨迹。**

它以后对 Debug、Benchmark，以及 Engineering Trajectory Dataset 都有价值。

---

# 24. Error Model

必须允许 Partial Review。

例如：

```text id="j0lsng"
CWE-476 success
CWE-787 success
CWE-416 query backend failed
```

不应该整个 Review：

```text id="0b12qd"
FAILED
```

而应该：

```json id="h53r9e"
{
  "status": "partial",
  "errors": [
    {
      "skill": "cwe-416",
      "type": "query_failure",
      "message": "..."
    }
  ]
}
```

最终 SARIF / Summary 应明确：

```text id="grnj9c"
Review completed with partial coverage.
```

不能悄悄吞掉失败。

---

# 25. Three Core Contracts

整个 Runtime 第一版最应该稳定的不是 Skill 数量，而是三个 Contract。

## Query Contract

```text id="fhixxs"
Review Skill
    ↓
How do I obtain code facts?
```

包括：

```text id="e1x2av"
input schema
output schema
error semantics
empty result semantics
truncation
cache semantics
version
```

---

## Review Contract

```text id="ebcf40"
Facts
 ↓
How does a Review Skill produce a Finding?
```

包括：

```text id="2vwm2i"
applicability
evidence
reasoning
confidence
finding schema
stop conditions
```

---

## Pipeline Contract

```text id="e55fbw"
How does a Review Session execute?
```

包括：

```text id="6z9i0i"
stage lifecycle
state
retry
failure
partial completion
artifacts
trace
output
```

这三个 Contract 比“先写 100 个 CWE Skill”重要得多。

---

# 26. First Implementation Scope

第一版不要做大。

建议只实现：

```text id="4u12b7"
Review Target:
    diff
    function
    file

Pipeline:
    complete 5 stages

Semantic Queries:
    find_definition
    find_references
    find_callers
    find_callees
    find_assignments
    find_pattern

Backends:
    codegraph
    rg
    ast-grep

Review Skills:
    5–10 CWE

Persistence:
    JSON

Output:
    Finding JSON
    SARIF

Trace:
    JSONL
```

暂时不要做：

```text id="0hnr4n"
distributed execution
database
vector DB
complex scheduler
web UI
multi-repository index
full dependency-aware cache invalidation
100+ CWE
```

---

# 27. Recommended Initial CWE Set

为了覆盖不同 Reasoning 类型，可以选择：

```text id="7nrj5w"
CWE-476  Null Pointer Dereference
CWE-416  Use After Free
CWE-787  Out-of-bounds Write
CWE-125  Out-of-bounds Read
CWE-190  Integer Overflow
CWE-401  Memory Leak
CWE-362  Race Condition
CWE-78   OS Command Injection
```

目的不是 CWE 覆盖率。

而是验证：

```text id="m93d8i"
Semantic Query
+
Evidence
+
Skill Reasoning
+
Context Cache
```

是否能支持不同类型问题。

---

# 28. Recommended Implementation Order

不要按照五个 Stage 顺序机械开发。

推荐：

```text id="y4sja5"
Step 1
定义 JSON Schema
    │
    ├── ReviewTarget
    ├── ReviewUnit
    ├── QueryResult
    ├── Finding
    ├── DispatchPlan
    └── Session
```

然后：

```text id="nhgynp"
Step 2
实现 Semantic Query Contract
        ↓
Atomic Query Backend
```

然后：

```text id="4zt6zo"
Step 3
实现一个 CWE-476 Skill
```

跑通：

```text id="ilc7g3"
Code
 ↓
Query
 ↓
Evidence
 ↓
Finding
```

再：

```text id="52qklb"
Step 4
Persistent Context + Cache
```

然后：

```text id="ydg12p"
Step 5
五阶段 Pipeline
```

最后：

```text id="jjfnye"
Step 6
SARIF + Trace
```

再逐步增加 CWE Skills。

---

# 29. End-to-End Example

用户：

```text id="r8ogoq"
Review current diff.
```

系统：

```text id="38bt2f"
review
  │
  ▼
pre-process
  │
  ├── create session
  ├── normalize target
  └── extract affected functions
  │
  ▼
preload
  │
  ├── macros
  ├── globals
  └── typedefs
  │
  ▼
dispatch
  │
  ├── Foo::process → CWE-476
  └── copy_data → CWE-787 / CWE-190
  │
  ▼
review-execute
  │
  ├── CWE-476
  │     ├── find_assignments(ptr)
  │     ├── find_definition(lookup)
  │     ├── find_pattern(null guard)
  │     └── Finding
  │
  └── CWE-787
        ├── inspect memcpy
        ├── trace len
        ├── trace dst size
        └── Finding
  │
  ▼
post-process
  │
  ├── validate
  ├── deduplicate
  ├── normalize
  └── SARIF
  │
  ▼
review.sarif
```

---

# 30. Architectural Boundary

第一版必须坚持下面这个边界：

```text id="ht70p9"
                    REVIEW DOMAIN
                         │
                    CWE Skills
                         │
                  Reasoning / Judgment
                         │
─────────────────────────┼────────────────────────
                         │
                SEMANTIC CODE QUERY
                         │
                   Facts Only
                         │
─────────────────────────┼────────────────────────
                         │
                    TOOL LAYER
                         │
              codegraph / rg / ast-grep
```

以及：

```text id="9qz9lm"
LLM
 │
 ├── reasoning
 ├── dispatch
 ├── evidence interpretation
 └── query planning

Scripts
 │
 ├── persistence
 ├── hashing
 ├── normalization
 ├── schema validation
 ├── deduplication
 └── SARIF
```

这两个边界不要破。

---

# 31. Definition of Done

第一版完成的标准不是：

> 支持多少 CWE。

而应该是：

### Pipeline

```text id="u9vc19"
pre-process
preload
dispatch
review-execute
post-process
```

可以完整执行。

### Query

CWE Skill 完全不感知具体查询 Backend。

### Evidence

Finding 可以追溯到代码事实。

### Context

相同 Query 可以跨 Skill 复用。

### Persistence

Review 中间状态不依赖当前 LLM Context Window。

### Failure

单 Skill 失败不会摧毁整个 Review。

### Output

Finding 可以稳定转换成 SARIF。

### Trace

可以回答：

```text id="h96ph6"
Why was this CWE dispatched?

What evidence was queried?

Was the query cached?

Why was this finding produced?

Which Skill produced it?

Which code location supports it?
```

如果这些问题都能回答，第一版 Runtime 就已经成立。

---

# 32. Future Evolution

第一版完成以后，再逐步向：

```text id="f7nkeg"
                    Code Review Runtime
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
       More Review Skills        More Query Backends
              │                         │
              ▼                         ▼
       Security / Quality      Clang / CodeQL / Joern
              │
              └────────────┬────────────┘
                           ▼
                    Better Evidence
                           │
                           ▼
                     Review Benchmark
                           │
                           ▼
               Embedded Coding Verification
                           │
                           ▼
                  Generate ↔ Verify
```

演进。

但这些不属于第一版实现范围。

---

# Summary

这个系统的核心不是：

> 使用 LLM 做 C/C++ Code Review。

而是建立：

> **一个以 Skill 为领域能力单元、Semantic Query 为事实接口、Persistent Context 为共享状态、Pipeline 为执行骨架、Evidence 为判断基础、SARIF 为标准输出的 Agent-native C/C++ Code Review Runtime。**

核心原则可以浓缩为：

```text id="y6kg7p"
能力原子化
接口标准化
流程可组合
状态可持久化
事实与判断分离
确定性与推理分离
Finding 可证据追溯
执行过程可观测
```

最终 Pipeline：

```text id="qvcdrh"
Review Target
      ↓
pre-process
      ↓
preload
      ↓
dispatch
      ↓
review-execute
      ↓
post-process
      ↓
SARIF
```

而整个架构最重要的抽象边界是：

```text id="dkgl8x"
Review Skill
     ↓
Semantic Query Contract
     ↓
Atomic Query Skill
     ↓
Code Intelligence Backend
```

只要这个边界稳定，Review Skill 和底层 Code Intelligence 都可以独立演进。
