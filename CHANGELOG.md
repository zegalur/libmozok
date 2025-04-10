# Changelog

All significant modifications to this project will be recorded in this file. The structure follows the guidelines of [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2024-04-11

### Added

- `HSP` heuristic function and action trees support  
- `heuristic` and `use_atree` options for quests
- Overall increase in planning speed

## [0.1.1] - 2024-06-05

### Added

- `status <questName> UNREACHABLE`
- `status ... PARENT <parentQuestName> <parentQuestGoal>` for subquests

### Changed

- Removed goal index from the `status <questName> INACTIVE` command
- Improved the `generateSaveFile()` function to ensure that a subquest's `status` command always after its parent quest's `status` command
- Improved logical consistency of the `onNewQuestStatus` message

## [0.1.0] - 2024-05-31

### Added

- Core API.
- Puzzle solver test.
- Basic quest solver test (will be enhanced in the future).
- Logo, LICENSE, README.md, TODO.md, and CHANGELOG.md.


