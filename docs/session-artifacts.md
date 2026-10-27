# Review Session 输出件说明

本文解释一次 `review-<时间>-<标识>/` 审查目录里的全部产物：谁生成、存什么、谁消费，以及为什么单独存在。

实际样本：

```text
/home/atituiset/Projects/AetherStack/.review/sessions/review-20260914-161800-492bbb2ca4b2/
```

该 Session 审查 `core::BsNode::request_handover`，基线 commit 为
`ed126a8ed87fefd2698affe928c875e52b663d0d`。检查时已完成：1 个审查单元、4 个排除场景、0 个确认问题、0 个未解决场景。
以下数量描述该样本，不代表所有运行都会生成相同数量的文件。

后续三规则版本增加了以下字段，不修改这个历史样本：meta 的 requested_rules、
rule_selection、execution_manifest；Session 的 execution-manifest.sha256；各规则
review 的 unit_results；最终报告 coverage 的 partial_units、unavailable_rules。
这些用于规则选择/指令变更后恢复，以及区分逐单元逐规则的成功、未解决与失败。
新规则分别写 reviews/cwe-416.json、reviews/cwe-401.json，文件职责与下文 476 相同。

## 目录总览

```text
review-<时间>-<标识>/
├─ meta.json
├─ target.json
├─ session.json
├─ pipeline.json
├─ units.json
├─ context/
│  ├─ files/
│  ├─ functions/
│  ├─ queries/
│  └─ raw/
├─ dispatch/
│  └─ plan.json
├─ reviews/
│  └─ cwe-476.json
├─ findings/
│  └─ raw/                     本次没有问题文件
├─ output/
│  └─ findings.json
├─ handoffs/
│  └─ <stage>-attempt-<n>.json
└─ trace/
   └─ events.jsonl
```

已提交 Git 版本的源码和 Codegraph 索引位于 Session 外部的 worktree 缓存，Session 通过元数据引用它们，不复制完整仓库。
当前默认位置是目标仓 `.review/cache/<commit>/`，与 `.review/sessions/` 并列。
本文历史样本曾使用目标仓外缓存，恢复时仍沿用 meta.json 的实际路径，不自动搬迁。
原仓源码索引需排除整个 `.review/`；查询显式指向缓存 worktree 自己的索引。

## 1. meta.json：代码版本与索引位置

**生成者：pre-process。消费者：需要读取源码或查询索引的 Stage。**

记录原仓路径、commit、基线模式、worktree 路径、Codegraph 版本和索引配置、缓存 lease 路径，以及排除内容。

它回答“这次审查依据哪一版代码，去哪里查询”。路径记录不是独立的源码备份；恢复仍依赖 Git 对象或可用 worktree。lease_path 是锁的位置记录，不代表审查结束后该锁仍存在。

## 2. target.json：用户要求审查什么

**生成者：pre-process。消费者：后续全部 Stage。**

把用户请求标准化为文件、函数、目录或 diff 目标。本次内容是函数目标：

```text
文件：stack/core/src/bs_node.cpp
函数：core::BsNode::request_handover
```

它限制最终问题的范围。其他文件和函数可以作为证据被查询，但不会因此自动变成审查目标。

## 3. session.json：整个任务的总状态

**生成者：pre-process 初始化，按阶段更新，post-process 收尾。消费者：Main 和 Stage。**

记录 Session ID、创建和更新时间、整体状态、target 引用、基线摘要、工具能力、验证方式和错误。

它回答“这次审查整体完成了吗，在什么条件下完成”。当前与 meta.json 重复保存了一部分基线字段，是可以进一步合并的实现选择；现有读取方仍按各自契约使用它们。

## 4. pipeline.json：五阶段执行进度

**生成者：pre-process 初始化，各 Stage 更新自己的记录。消费者：Main 和恢复流程。**

每个阶段记录状态、执行次数、起止时间、输入、输出和错误。用于决定下一步及定位未完成阶段。

session.json 记录整体状态，pipeline.json 记录阶段状态。不能仅凭阶段名存在就认定阶段完成，也不能仅凭文件中的 worker 身份证明实际发生了子 Agent 调用。

## 5. units.json：实际审查工作清单

**生成者：pre-process 拆分，后续更新处理状态。消费者：preload、dispatch、review-execute、post-process。**

每个单元包含 ID、类型、文件和行号、变更范围、状态及 context_ref。本次只有一个函数单元，范围为 bs_node.cpp 的 1258–1351 行。

target.json 表达用户要求，units.json 表达实际拆出的工作清单。文件或目录目标可以拆成多个单元。

## 6. context/files/：文件级基础上下文

**本次生成者：preload。消费者：dispatch、review-execute。**

本次文件为 `file-core-bsnode-h.json`。它整理相关头文件声明、类型、依赖与查询引用，为后续阶段提供可复用的基础信息。

它不是整个源文件的替代品，也不是漏洞判定报告。

## 7. context/functions/：函数级基础上下文

**本次生成者：preload。消费者：dispatch、review-execute。**

本次文件为 `fn-core-bsnode-request-handover.json`。围绕目标函数组织相关类型、宏、全局变量、依赖和查询引用。

preload 准备基础内容，后续阶段可以按需补充；context 是 Session 共享材料，不是 preload 独占的输出空间。

## 8. context/queries/：查询请求与标准化事实

**生成者：执行查询的 Stage，通过 Query Skill 生成。消费者：规则、上下文构建和后处理。**

本次有 13 组查询：8 组来自 preload，5 组来自 review-execute。每组通常包含：

| 文件 | 内容 |
| --- | --- |
| `<query-id>.request.json` | 操作、范围、参数和查询目的 |
| `<query-id>.json` | 标准化事实、源码位置、依赖哈希、来源、覆盖限制和错误 |

例如 `q-exec-findue-defn` 查询 find_ue 的实现，`q-exec-guards` 记录保护条件。规则使用 `query_id + fact_id` 引用证据。

Query 提供事实，例如“此处存在 if (ue)”，不负责判定该条件是否足以排除漏洞；后者属于 CWE 规则。

## 9. context/raw/：工具原始输出

**生成者：执行查询的 Stage。消费者：Query Skill，以及必要的追溯和核验流程。**

保存 Codegraph 原始 JSON、源码搜索文本和 stderr。本次同时存在 pre-process、preload 和 review-execute 的材料，文件前缀分别包括 `preprocess-`、`preload-` 和 `exec-`。

正常流向为：工具输出 → raw → Query Skill 提取事实 → queries → 审查规则。

它不是纯日志：标准化时会消费，遇到歧义、截断或核验证据时也可能回查。常规规则检视应消费标准化事实，避免绕过 Query 契约直接从原始文本得出问题结论。

单独保存每条 stdout/stderr 是当前实现选择；特别是空 stderr，并非必须独立成文件。

## 10. dispatch/plan.json：单元与规则的分派计划

**生成者：dispatch。消费者：review-execute。**

记录各 unit 要执行哪些 CWE Skill、适用信号、分派理由、事实引用及待确认的类型问题。

本次分派到 `cwe-476-null-pointer`。分派表示需要按该规则检查，不代表已经确认问题。样本中 dispatch 复用了 preload 上下文，没有新增查询。

## 11. reviews/cwe-476.json：规则检视记录

**生成者：review-execute。消费者：恢复流程和 post-process，也供人工解释结论。**

记录检查过的场景，包括指针和使用点、证据引用、确认/排除/未解决状态、结论摘要、缺失信息、下一步查询及对应 Finding ID。

本次四个场景都被排除，因此没有 Finding，但保留了排除依据。它是解释“为什么没有报问题”的主要产物，也支持保存尚未完成场景的 checkpoint。

## 12. findings/raw/：后处理前的问题记录

**生成者：review-execute。消费者：post-process。**

规则确认问题后写入此处，后处理再检查证据、去重和汇总。待调查或已排除的场景保留在 reviews，不应当伪装成已确认的问题。

本次该目录没有文件。这里的 raw 表示“尚未后处理的 Finding”，与 context/raw 的“工具原文”含义不同。

## 13. output/findings.json：最终对外报告

**生成者：post-process。消费者：用户、下游系统，以及 Main 的结果展示。**

包含最终 Findings、覆盖范围、场景数量、限制、错误、验证信息及 review 引用。

本次 findings 为空，计数为 confirmed=0、excluded=4、unresolved=0。这只表示在本次函数范围与 CWE-476 规则下没有确认问题，不表示全仓安全。

本次没有生成 SARIF；按需输出 SARIF 时，它也属于 output 下的发布产物。

## 14. handoffs/：Stage 给 Main 的交接结果

**生成者：各 Stage worker。消费者：Main 编排器。**

本次五份文件分别为：

```text
pre-process-attempt-1.json
preload-attempt-1.json
dispatch-attempt-1.json
review-execute-attempt-1.json
post-process-attempt-1.json
```

每份描述一次调用的阶段、身份信息、状态、输出路径、能否继续、下一阶段建议及阻塞问题。Main 依靠这些短结果编排，不必加载源码和完整检视过程。

pipeline.json 是阶段进度总表，handoff 是某次 worker 调用的返回结果。调用身份最终还需要与宿主真实 Agent 调用记录核对。

## 15. trace/events.jsonl：执行事件记录

**生成者：各阶段追加。消费者：过程排查和恢复诊断。**

每行一个事件，记录阶段开始结束、worktree 创建、索引核验、查询执行、上下文写入等。它记录发生过什么，不是规则判断漏洞的主要事实接口，也不应保存模型的隐藏思维链。

## 数据流与可简化的部分

```text
target + meta
      ↓
    units
      ↓
context ← 查询请求 → 工具 raw → 标准化 facts
      ↓
dispatch/plan
      ↓
reviews + findings/raw
      ↓
output/findings.json

session / pipeline / handoffs：管理任务状态与阶段交接
trace：记录执行事件
```

这些职责值得区分，但不必一一对应独立物理文件。当前 meta/session 的基线重复、pipeline/handoffs 的部分状态重复，以及查询请求、结果、stdout、stderr 分开存储，均属于实现选择，不是五阶段 Skill 架构的硬性要求。若调整文件组织，需要同时更新生产者、消费者和 schema，不能直接删除仍被引用的产物。

本样本使用 worktree 基线，没有 Session 内的完整 snapshots/repository-view 或 codegraph-project。未提交内容采用 live-hashed 时可能保存必要的 snapshots；历史 Session 的双副本目录保留原语义。参见 [worktree 基线协议](../skills/review/references/worktree-baseline.md)。
