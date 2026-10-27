# Callee dependency change check

The immutable original fixture was left unchanged. A separate three-file view was
created with only provider.cpp changed: its false branch now returns &object.

- consumers.cpp stayed at SHA-256 `5f8a34e2ffef07d1a9b305a711d6716870e984efbab5fdfe7223a85f8e30a3b9`.
- Original provider.cpp: `dc548751653186ca76d519632f4e5a77995fed642a96bef69c53309ac564fa4d`.
- Changed provider.cpp: `3277bf3c6b1f36476f5161f7f7f3a3ba0049751f7ce411f094bdf6f12e7c3dab`.

The executing agent rejected reuse of the old q01 null-return conclusion for this
new view. Codegraph indexed the changed complete fixture and returned the new body
in context/queries/cache-change.raw.txt. The caller hash alone would have missed this
change. This is a Skill-directed invalidation exercise, not an automatic cache-engine
test, and no revised full review report is claimed for the changed view.
