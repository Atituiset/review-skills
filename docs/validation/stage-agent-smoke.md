# Stage 子 Agent 验证（2026-09-14）

执行宿主为当前会话的原生 `spawn_agent`，不是 Claude Code。每个 worker 使用
独立的新上下文，读取对应 Stage Skill，并共享 Session 文件。Main 只传递请求、
检查 handoff 元数据和安排下一阶段，没有代做源码分析或阶段产物。

目标为 `tests/fixtures/cross-file/consumers.cpp`，规则为 CWE-476。
本机 Session：`/tmp/review-stage-subagent-smoke/review-20260914T153138Z-smoke1`。
Codegraph 使用本机现有 CLI；未编写 Python、Node 或 shell 流程驱动。

| Stage | 实际宿主 worker handle | Handoff |
| --- | --- | --- |
| pre-process | /root/stage_pre_process_smoke | handoffs/pre-process-smoke-1.json |
| preload | /root/stage_preload_smoke | handoffs/preload-smoke-1.json |
| dispatch | /root/stage_dispatch_smoke | handoffs/dispatch-smoke-1.json |
| review-execute | /root/stage_review_execute_smoke | handoffs/review-execute-smoke-1.json |
| post-process | /root/stage_post_process_smoke | handoffs/post-process-smoke-1.json |

这些 handle 来自本次实际工具调用；不能仅凭 JSON 自报的身份判定隔离成立。
五阶段均已完成。post-process 独立检查源码/事实/引用及依赖哈希后发布
`output/findings.json`：4 个已审查单元，3 个 confirmed Finding、1 个作用域内
excluded 场景、0 unresolved、0 skipped。不同 sink 未合并。验证方式为
agent-inspection，未声称运行完整的程序化 schema 验证器。

边界：这是受控例子的阶段委派验证，不能替代 Claude Code 中的 `/review` 实跑，
也不证明任意 C/C++ 项目的分析准确率或完整 JSON Schema 验证。历史同 Agent
运行结果与本记录分开。Claude Code 验收应新开会话，确认有五次真实的 Agent
调用，而不是主会话读取五份 Markdown 或输出五个阶段标题。
