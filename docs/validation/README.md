# 0.2.0 验证记录

**历史记录说明：下列运行发生在 Stage 子 Agent 拓扑修正之前。** 它们是同一个
Agent 的证据链验证，不能作为“Main 只编排、五个独立 worker”验收。当前入口
已要求 stage-agent/1，Stage 隔离必须另看真实 host 子 Agent 调用与 handoff。

本次原生宿主的独立 worker 验证见 [Stage 子 Agent 记录](stage-agent-smoke.md)。
后续 worktree 基线与索引工具验证见 [Worktree 记录](worktree-baseline.md)。
三规则受控行为检查见 [Memory rules forward test](memory-rules-forward-test.md)：
独立 worker 检查六个所有权案例，确认一处 UAF、一处泄漏，外部转移契约未知时
保留 unresolved。该测试未创建完整 Session，不代表跨规则持久缓存或五阶段验收。

执行者：本次宿主 Agent。全程为 Skill 指令 + 现成 Codegraph/git/文件工具；没有
Python 驱动或随包分发的自定义执行程序。以下验证不是独立 Agent 盲测，也不证明
对任意 C/C++ 项目具有编译器级精度。

## 已执行

| 场景 | 实际结果 | 证据 |
| --- | --- | --- |
| 跨文件 nullable return → 未判空 sink | confirmed，available=false | cross-file-v02/output/findings.json 的 f1 |
| 同一指针正确判空 | excluded | cross-file-v02/reviews/cwe-476.json 的 s2 |
| 检查错误的指针 q | confirmed，p 仍可为空 | f3 |
| 判空后重新赋值 | confirmed，第二次 lookup(false) 返回空 | f4 |
| Codegraph callers/callees | 实际获取到四个调用方、提供者定义 | q02.raw.json / q03.raw.json |
| 赋值、引用、模式、类型关系 | 原始工具/源码 + Agent 标准化，覆盖七类 Query | q01–q08.json |
| checkpoint / 查询复用 | 保存 expanding 场景后重新读取，核对三个依赖哈希，复用 q01 | checkpoint-before-resume.json / trace/events.jsonl |
| callee 改变但 caller 不变 | 依赖哈希改变，拒绝沿用旧 q01 的 null-return 结论；新索引返回非空分支 | cache-change.raw.txt / cache-invalidation.md |
| 外部定义缺失 | 图查询为空后继续源码搜索，只有 extern 声明与调用；unresolved，零 Finding | missing-definition-v02/output/findings.json |
| JSON / SARIF 后处理 | 三个不同 sink 保留，无误合并；生成两种产物 | cross-file-v02/output/ |
| AetherStack 实际跨文件审查 | NasBs 可空返回 → BsNode 使用点 → 本地保护；同时进行类型消歧 | 目标仓 .review/sessions/review-aether-v02/ |

受控跨文件例子拥有五阶段最终状态、共享 context、dispatch plan、checkpoint、
原始查询、标准化 facts、三个 Finding 及报告。查询工具原始输出在建 Session 前的
能力探测中取得，之后在阶段中由 Agent 检查和标准化；没有伪装成每个查询都由
自动 Pipeline 调度。恢复演练是同一 Agent 的显式重读，不是进程崩溃后的自动恢复。

缺失定义例子只验证规则与 partial 契约，不宣称它执行了完整五阶段。

## AetherStack

基线：`ed126a8ed87fefd2698affe928c875e52b663d0d` 的完整 tracked Git 快照，
Codegraph 1.6.0 索引 142 个文件。原仓未跟踪的运行时目录未纳入快照。

实际目标：`stack/core/src/bs_node.cpp` 的 `core::BsNode::request_handover`。
读取 `stack/nas/src/nas_bs.cpp` 的 `NasBs::find_ue` 和 BsNode 头文件声明消歧。
原始图中还出现不相关的同名 `end`/`send` 候选，未作为精确调用关系使用。

六个已记录场景均排除：三个 raw-pointer 访问具有局部 guard，另三个是 iterator、
std::function 或 std::optional 的类型适用性排除。结论仅适用于记录的顺序路径与
CWE-476，不是整个函数在所有方面安全，也不覆盖并发修改/悬空引用问题。
阶段记录是本次实际操作的回顾性产物，不是程序自动生成的逐事件状态机。

## 产物检查与未验收项

本次借助宿主工具解析了 73 份 JSON 文档，比较 45 段事实与源快照，解析 70 条
证据引用，未发现错位或断链。此数值不包括随后产出的 SARIF；没有宣称运行了完整
JSON Schema/SARIF 验证器。Skill 保持 `validation.mode=agent-inspection`。

尚未行为验收：diff/目录全流程、真实复杂别名/虚调用、多个 CWE 混合故障、进程崩溃
恢复、并发写入、跨版本 Session 迁移、SARIF schema 完整验证、独立 Agent 准确率。
这些不能因为已经有指令/Schema 就算“全部通过”。
