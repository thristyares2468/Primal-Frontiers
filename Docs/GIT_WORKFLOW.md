# Git and Unreal assets

## Milestone commits and attribution

For Codex-authored milestone work, include `Milestone N` in the commit subject and add `Co-authored-by: Codex GPT-6 Astra <codex@openai.com>` as a trailer. Retain the repository's configured primary Git author. Commit and push the task's reviewed, verified changes without staging unrelated user work. Describe incomplete gates honestly in checkpoint commits; never label a milestone complete before its playtest gate passes.

Git LFS tracks `*.uasset`, `*.umap`, and the existing `*.psd` rule. Commit `.gitattributes` with the project. GitHub Desktop can commit and push these files once Git LFS is installed; new command-line clones should run `git lfs install` and `git lfs pull` if needed.

Keep source code, configuration and documentation in ordinary Git. Do not version `.vs`, `.vscode`, `Binaries`, `Intermediate`, `Saved`, or `DerivedDataCache`, including plugin-generated directories. These are excluded rather than uploaded through LFS. Removing files from a current commit does not remove their earlier versions from history.

Before pushing, use `git status`, `git lfs status`, and `git lfs fsck`. Push the current branch normally (`git push`). LFS uploads happen through the pre-push hook. Do not disable the hook or force-push to work around a file-size rejection. LFS has separate hosting quotas and per-file limits.

## 2026-09-22 initial-publish repair

The remote had no published branches. A separate repair clone removed generated directories from the unpublished history and migrated Unreal asset revisions to LFS. The original full history was preserved in `C:/UnrealProjects/PrimalFrontier-GitBackup-20260922/PrimalFrontier-before-lfs.bundle`. The original project directory's asset/source files were checked against SHA256 hashes before adopting the repaired history. Local generated files are retained.

Commit IDs changed during migration; older IDs mentioned in milestone documentation refer to the pre-migration history in the backup. The repair preserves commit messages and project changes. Do not push the backup history or use `git push --all`/`--mirror` to publish backup refs.
