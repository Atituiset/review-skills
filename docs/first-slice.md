# 设计落实对应表：0.2.0

执行拓扑修正（stage-agent/1）：Main Agent 只编排，每个 Stage 由一个新建的独立
子 Agent 加载 Skill 执行。用真实 host 调用和 handoff 验证；旧的同 Agent 验证
不是这个拓扑的验收。详见 skills/review/references/stage-agent.md。

遵守最新用户要求：全 Skill 编排，不生成独立 Python/Node/Shell 驱动；现成
Codegraph、git、文件工具直接使用。原设计 2.2 中“确定性程序”的建议因此变为
“Agent 按契约调用现成工具”；没有自动验证器时明确披露人工式检查。

| 原设计 | 本版对应 | 状态与边界 |
| --- | --- | --- |
| 2、3、30 架构边界 | review → pipeline/review/query Skills | Skill 负责各领域职责；包内嵌套仅为安装分发，不是独立 Runtime |
| 5、6 Target / Session | schemas + pre-process / session-workspace | 0.2.0 支持 file/function/diff/directory 契约；后两者未做端到端行为验收 |
| 7–10 五阶段 | skills/review/skills/pipeline/ | 各阶段输入/输出/状态/失败约定；受控例子执行并保存阶段产物 |
| 11、14 CWE / Lazy Expansion | cwe-476-null-pointer | 跨文件来源、调用参数、guard、重赋值，三种结论状态 |
| 12、13 Query / Atomic | semantic-query / atomic-query | 七类真实工具路径；赋值/guard 是语法观察，不宣称编译器级数据流 |
| 15、16 缓存与一致性 | dependencies + baseline + query_refs | Agent 检查后复用；有 checkpoint 读取与复用演练，不是自动缓存系统 |
| 17–19 Finding / Evidence | finding、review-record schemas | 必须引用事实；支持跨文件 hop、排除结论、缺口和 next_queries |
| 20、21 后处理 | post-process + SARIF reference | 默认 JSON；受控例子已产出 SARIF，尚未运行完整 SARIF schema 验证器 |
| 22–24 状态/trace/partial | pipeline、session、review records | 失败保留已完成结果；没有并发事务或自动进程恢复保证 |
| 26、27 更多 CWE | CWE-476、CWE-416、CWE-401 | 已有三份规则、通用目录/执行/后处理；仍未达到首批 5–10 CWE，跨规则行为验证见 validation |
| 31 Definition of Done | docs/validation | 以实际证据链和行为记录评估，不以文档/Schema 数量宣布完成 |

与之前错误切片的关键区别：不再用单文件索引限制语义上下文；不因后端没有同名
API 就停止查询；不把出现 guard 视为已防护；不把代码查询和文件落盘当作审查效果。

Claude Code 安装包含 review Skill 目录和 agents/review-worker.md 两部分。各 Stage
使用同一 worker 定义的新实例。Git 基线默认使用目标仓 .review/cache 下的 worktree 缓存；新 Session 记录
执行文件哈希，恢复时同时检查源码、索引及指令版本。独立 Skill 的逻辑身份、
输入输出与相对依赖保留在各自 SKILL.md 中。
