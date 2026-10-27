# Optional SARIF adapter

When requested, write SARIF 2.1.0 directly with file tools. Set version "2.1.0",
$schema "https://json.schemastore.org/sarif-2.1.0.json", runs[0].tool.driver.name
"review-skills" and semanticVersion "0.2.0". List distinct rules in driver.rules.
For each accepted finding set ruleId, message.text (claim), level (critical/high ->
error; medium -> warning; low -> note), locations[0].physicalLocation.artifactLocation.uri
(repository-relative, percent-encode URI-special characters), region.startLine/endLine.
Include confidence, finding ID and reasoning_summary in result.properties. Evidence
may appear in relatedLocations, with exact snapshot path/line facts resolved by
post-process. Do not fabricate codeFlows if control flow is not established.

Set invocation.executionSuccessful false for infrastructure failure; partial scope
may still execute successfully but MUST expose limitations in toolExecutionNotifications
and run.properties.reviewStatus. Keep suppressed/unresolved scenarios in the JSON
review records. A report is not called schema-validated unless an actual SARIF validator
was run. Agent inspection alone must be disclosed. JSON remains the source for adapters.
