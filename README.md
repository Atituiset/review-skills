# C/C++ Review Skills

按 [5-stage-review.md](5-stage-review.md) 实现的 **全 Skill、分阶段子 Agent 编排**：
Main Agent 通过 review Skill 调度；五个 Stage 分别创建独立的 review-worker 实例，加载对应 Skill 后执行，
通过 Session 文件交接。Semantic Query 获取事实，CWE Skill 推理。
无需 Python 驱动、pip 或自定义 Runtime；Codegraph 是直接调用的现成工具。

```text
/review → Main Agent（只编排）
             ├─ 子 Agent 1：pre-process → handoff
             ├─ 子 Agent 2：preload → handoff
             ├─ 子 Agent 3：dispatch → handoff
             ├─ 子 Agent 4：review-execute → handoff
             └─ 子 Agent 5：post-process → 最终报告
```

五个阶段按依赖顺序执行，不并行写 Session。仅在主 Agent 中读取五份 SKILL.md
不算委派；必须有五次真实的子 Agent 调用。若 Agent 工具不可用，明确阻塞，
不会退回主 Agent 执行。协议见 [stage-agent/1](skills/review/references/stage-agent.md)。

## 安装与使用

同时安装 `skills/review/` 和 `agents/review-worker.md`。已有旧版先移到备份位置，
再复制整个目录，避免残留旧脚本与不兼容 schema。

```bash
# Claude Code 项目级；全局使用 ~/.claude/skills 和 ~/.claude/agents
mkdir -p /你的目标仓/.claude/skills /你的目标仓/.claude/agents
cp -R skills/review /你的目标仓/.claude/skills/
cp agents/review-worker.md /你的目标仓/.claude/agents/review-worker.md
```

Claude Code 的 Agent 注册目录是 `.claude/agents/`，不是通用的 `.agent/`。
其他企业宿主需把 worker 定义注册到其原生 Agent 目录；不能假定复制 Claude
Markdown 到 Codex 的 Skills 目录就完成了 Agent 注册。本版安装说明针对 Claude Code。

在 Claude Code 直接输入：

```text
/review 审查 src/example.cpp，Codegraph 使用 /home/atituiset/Projects/codegraph
```

也可以明确函数或 diff 的 base/head。路径只是本机 Codegraph 配置示例，安装包
不依赖该开发路径。入口是 [review/SKILL.md](skills/review/SKILL.md)。

更新已安装的 Skill 后，建议新开 Claude Code 会话再运行，避免旧会话继续沿用
先前读入的指令。验收时查看实际 Agent 工具调用：五个 Stage 应分别有独立 worker；
主会话只出现阶段标题或读取五份 SKILL.md，不能算子 Agent 隔离。

继续已有 Session：

> 使用 review Skill，恢复 .review/sessions/<id>，先检查 baseline 和未完成的 next_queries。

## 如何阅读检视输出

每次检视在目标仓的 `.review/sessions/review-<时间>-<标识>/` 保存完整工作记录。
**先看 `output/findings.json` 的最终结论；想知道为什么，再看 `reviews/`；
需要核对证据时，沿引用查看 `context/queries/`。** 不必先读完全部中间文件。

### 任务信息：检查什么、哪个版本

| 文件 | 内容 | 生成阶段与用途 |
| --- | --- | --- |
| `target.json` | 用户指定的文件、函数、目录或 diff | pre-process 标准化请求，后续阶段据此限制问题范围 |
| `meta.json` | commit、源码 worktree 路径、Codegraph 索引信息 | pre-process 固定基线，后续阶段找到对应代码与索引 |
| `units.json` | 拆出的审查单元、文件行号、状态和上下文引用 | pre-process 建立工作清单，后续逐单元处理 |

新 Session 的 meta 还保存 requested_rules、rule_selection 和 execution_manifest
引用；execution-manifest.sha256 记录实际 Skill/契约内容哈希，恢复时检查规则变化。
例如检查一个目录：target 记录目录，units 列出实际检查的函数，meta 保证它们
来自同一代码版本。审查范围不限制取证范围：可以查询其他函数来理解目标函数。

### 上下文与事实：判断前需要知道什么

| 路径 | 内容 | 生成者与消费者 |
| --- | --- | --- |
| `context/files/` | 文件级类型、声明、依赖和查询引用 | preload 建立，dispatch 和 review-execute 复用 |
| `context/functions/` | 函数相关的类型、宏、全局变量及事实引用 | preload 建立，后续按需补充和消费 |
| `context/queries/*.request.json` | 查什么、范围、参数和目的 | 执行查询的 Stage 通过 Query Skill 保存 |
| `context/queries/*.json`（不含 request 文件） | 标准化事实、源码位置、依赖哈希、来源与限制 | Query Skill 整理，规则与后处理消费 |
| `context/raw/` | Codegraph、源码搜索等工具原文及 stderr | 执行查询的 Stage 保存，供事实提取和必要时回查 |

context 是整个 Session 的共享材料，不只由 preload 写入。正常数据流是：

```text
查询请求 → 工具原始输出（raw）→ 标准化事实（queries）→ 规则判断
```

例如 Query 查出“find_ue 某个分支返回 nullptr”，这只是事实；调用方是否存在
空指针问题，要由规则结合使用点、保护条件和赋值关系判断。raw 不只是过程日志，
但常规规则检视主要消费标准化事实，无需反复解析工具原文。

### 分派、检视与最终报告

| 路径 | 内容 | 生成阶段与用途 |
| --- | --- | --- |
| `dispatch/plan.json` | 每个单元要执行的 CWE Skill、适用信号和依据 | dispatch 生成，review-execute 按计划检视；分派不等于发现漏洞 |
| `reviews/cwe-476.json` | 检查过的场景、证据引用、判断理由、缺失信息及下一步查询 | review-execute 生成，用于解释结论、恢复检视和后处理 |
| `findings/raw/` | 已确认、尚待后处理的问题记录 | review-execute 生成，post-process 检查、去重和汇总；无确认问题时可为空 |
| `output/findings.json` | 最终问题列表、覆盖范围、场景数量、限制与验证信息 | post-process 发布，用户和下游系统直接消费 |

reviews 中的场景分为三种状态：`confirmed` 表示有证据支持问题成立，`excluded`
表示有依据排除此场景，`unresolved` 表示证据不足、暂时不能判断。
**最终 findings 为空不代表没有检查：排除依据仍保存在 reviews。**

每个规则写自己的 reviews/cwe-<编号>.json，并在 unit_results 中记录每个已分派
单元的执行结果。零场景完成与执行失败分别记录，不能混为“无问题”。最终报告
用 partial_units 表示规则未全部完成的单元，用 unavailable_rules 列出不支持的
请求规则；一条规则失败不会丢弃其他规则的成功结果。

例如，“find_ue 可能返回空，但 ue->imsi 位于 if (ue) 保护内”，可以形成一个
排除场景，不生成 Finding。它只排除记录的这个场景，不证明整个项目安全。
`findings/raw` 的 raw 意为“尚未后处理”，与 `context/raw` 的“工具原文”不同。
按需生成 SARIF 时，它也存放在 output 中。

### 执行管理：各个 Agent 做到了哪里

| 路径 | 内容 | 谁使用 |
| --- | --- | --- |
| `session.json` | 整体状态、时间、基线摘要、工具能力和错误 | Main 与 Stage 了解任务状态；pre-process 初始化，post-process 收尾 |
| `pipeline.json` | 五阶段各自的状态、次数、起止时间、输入输出和错误 | 各 Stage 更新自己的记录，Main 调度和恢复时读取 |
| `handoffs/<stage>-attempt-<n>.json` | 一次 worker 调用的交接单：结果路径、能否继续、下一阶段建议或阻塞问题 | Stage 返回给 Main，避免 Main 加载完整分析材料 |
| `trace/events.jsonl` | 阶段开始结束、查询、索引核验等事件，每行一条 | 各阶段追加，供排查和追溯 |

这些文件主要服务于编排和恢复，正常阅读结果时不必逐一打开。pipeline 是进度
总表，handoff 是一次 worker 调用的返回结果，trace 是事件历史。

```text
output/findings.json        看最终结论
         ↓
reviews/cwe-476.json        看场景判断与依据
         ↓
context/queries/           核对引用的代码事实
         ↓
context/raw/               必要时回查工具原文
```

源码与索引位于 Session 外的 worktree 缓存；新建的已提交版本审查不复制完整
仓库到 Session。未提交内容可能产生必要的 snapshots；旧 Session 保留原目录。

文件拆分属于当前实现选择，不是五阶段架构的硬性要求。meta/session 的基线字段、
pipeline/handoff 的部分状态有重复；请求、结果、stdout、stderr 也可以在更新
契约后合并组织。详细实例见 [Session 输出件说明](docs/session-artifacts.md)。

## 已实现的 Skill 协议

- 五阶段：pre-process → preload → dispatch → review-execute → post-process。
- file/function/diff/directory 目标契约；目标限制 Finding 范围，不限制证据搜索范围。
- 七类查询映射：定义、引用、调用方、被调函数、赋值、语法/字面模式、相关符号。
  Codegraph + 源码回退；明确歧义、截断、错误和查询精度。
- 按 commit 复用 Git worktree 和本地索引、跨文件依赖哈希、共享上下文、查询复用、scenario checkpoint。
- 三个规则：CWE-476 空指针、CWE-416 释放后使用、CWE-401 内存泄漏；
  统一 confirmed/excluded/unresolved 和证据契约，按需共享事实查询。
- Finding JSON、证据检查、保守去重、部分失败报告；按需 SARIF 映射说明。

查询和生命周期由 **Agent 执行 Skill 指令**，不是程序强制的类型系统、编译器级
数据流分析或事务引擎。未运行外部验证器时，会明确记录 agent-inspection。
已安装规则见 [规则目录](skills/review/references/rules.md)。明确指定 CWE 时只考虑
指定规则，否则考虑全部已安装规则；不支持的请求会报告覆盖缺口。
规则失败按 unit × rule 隔离，成功结果保留；不同 CWE 暂不合并为同一 Finding。
新 Session 记录执行文件哈希，规则更新后恢复时重新验证旧结论。

入口不设置 `context: fork`：否则只是把整个流程放进一个子 Agent。Main 使用
Claude Code 的 Agent(review-worker) 为每个 Stage 创建独立实例，再让 worker
读取包内相对路径对应的 Skill。Stage 的 fork frontmatter 用于它被宿主直接注册
调用的情况；单纯读取 frontmatter 不会生成子 Agent。通用执行边界集中在
[review-worker 定义](agents/review-worker.md)，五个实例共享定义但不共享上下文。
界面仍可能显示 Agent 调用；自定义 worker 不会把 Agent 工具变成 Skill 工具。

## 验收与实现边界

[Session 输出件说明](docs/session-artifacts.md) 逐项解释实际审查目录中的文件、生成阶段、消费者和数据流。

新 Git 审查默认使用目标仓内的 `.review/cache/<commit>/` detached
worktree，Session 的 `meta.json` 记录版本、路径和索引绑定，不再创建完整的
`snapshots/repository-view/` 或 `codegraph-project/`。同一缓存通过 Session 独占
lease 串行使用，结束后保留 worktree/index 供复用。Git 不自动传递 Codegraph
索引；首次建立、后续验证复用。未提交内容采用 live-hashed 和必要文件捕获。
原仓 Codegraph 必须明确排除整个 `.review/`，避免将缓存源码重复索引；
pre-process 检查实际工具支持的配置并保留已有设置。旧 Session（包括外部缓存）
继续使用已记录路径，不自动迁移。目录关系如下：

```text
目标仓/.review/
├─ cache/
│  ├─ <commit>/             # Git worktree，内部 .codegraph/ 为其独立索引
│  ├─ <commit>.index.json   # 索引绑定记录
│  └─ <commit>.lease/       # 使用期间的锁
└─ sessions/
   └─ review-…/            # meta.json 引用 cache 中的实际 worktree
```

详见
[基线协议](skills/review/references/worktree-baseline.md)。

[实施对应表](docs/first-slice.md) 对应原设计章节。
[验证记录](docs/validation/README.md) 区分真实工具执行、受控场景、未验证能力。
受控跨文件例子和完整产物保存在 [cross-file-v02](docs/validation/cross-file-v02/)。

历史 0.1 脚本版的“13 tests passed”不适用于本版。0.2.0 与其 Query/Finding schema
不兼容，旧 Session 保留回放，不直接迁移或混用。

早期 0.2.0 验证也是同 Agent 执行，只能验证证据链，不能证明 Stage 隔离。
当前执行拓扑使用 stage-agent/1，保留 0.2.0 事实 schema 并新增 handoff schema。
