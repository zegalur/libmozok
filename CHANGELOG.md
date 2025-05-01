# Changelog

All significant modifications to this project will be recorded in this file. The structure follows the guidelines of [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.0] - 2025-05-01

### Added

- To `Server` public interface:
    - `getObjects`
    - `getObjectType`
    - `getActions`
    - `getActionType`
- Ability to include objects by their type (in the quest's `objects:` section).

## [1.0.0] - 2025-04-23

### Added

- QSFs (Quest Script Files).
- Quest debugger tool (`mozok.exe`) for debugging non-linear quests:
    - Special QSF commands for describing all possible complex splits and timelines.
    - A "Tutorial" for verifying the debugger (based on tutorial quest from `libmozok-godot`).
    - Ability to generate a visual representation of the simulation process.
- `FileSystem` class to the public interface.
- To `Server` public interface:
    - `loadQuestScriptFile`
    - `getWorlds`
    - `hasObject`
    - `hasMainQuest`
    - `hasSubQuest`
    - `checkAction`
- `onQuestGoal` event to `MessageProcessor`.

### Changed

- Basic recursive descent parser separated from the `project.cpp`.
- Action data and action error codes:
    - `applyAction` now outputs the action error code.
    - `pushAction` now accepts an additional `int` data parameter.
    - `onActionError` now has `data` and `actionError` parameters.

## [0.2.1] - 2025-04-12

### Added

- Search strategies for quests (A\* and DFS).
- `strategy ASTAR` and `strategy DFS` quest options.

### Changed

- Improved quest solver test.

## [0.2.0] - 2025-04-11

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


