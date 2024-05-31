# Libmozok

Libmozok TODO list.

### Todo

- [ ] Add: support for commas in quest object, action, and sub-quest lists.
- [ ] Add: support arrays:
    - [ ] Add: `object arr[N,M,..] : Type` -> `arr_0_0`, `arr_0_1`, ...
    - [ ] Add: `rel(arr[i,j], arr[i+1, j])` -> `rel(arr_0_0, arr_1_0)` for all pairs of `(i,j)` that are consistent with the array size.
- [ ] Add: onNewQuestGoal
- [ ] Add: Error: The same object appears twice or more in the same statement.
- [ ] Add: Error: Action argument was not used.
- [ ] A quest can be a sub-quest for only one parent quest at most.
- [ ] Add: error codes for `libmozok::Result`
- [ ] Add: a better heuristic function(s) for the quest planner
- [ ] In the `World::performPlanning` function, update only the quests that are activated but not yet completed. This can be achieved by creating a set of quests that need to be planned.
- [ ] Narration Engine
    - [ ] Implementation of multiple language support
    - [ ] Typed pattern matching with higher priority for the first match
    - [ ] A tool for verifying the completeness of the narration
- [ ] Migrate to C++17 (`[[nodiscard]]`, `<string_view>` etc.)
- [ ] Use `std::move()` to transfer ownership of resources and reduce unnecessary copy operations where possible. Additionally, use `const T` (together with `std::move()`) instead of `const T&` in function signatures to indicate that data has been copied.
- [ ] Logical consistency of the messages
    - [ ] Add: `PARENT [parentQuestName]` to the `status` commands of sub-quests.
    - [ ] Ensure the order of the `status ...` commands in the save file matches the order in which they were triggered during the game-play.
- [ ] Fix the highlighting for relation definitions with three or more arguments.

### In Progress

- [ ] Basic documentation
    - [x] Doxygen generated documentation
    - [ ] First Tutorial

### Done ✓

- [x] Global and local actions
    - [x] Global actions either reference an external object at least once or include at least one 0-arity relation
    - [x] Ensure that local functions affect only the quests that are relevant
    - [x] Quests can only accept local actions in the actions list

- [x] N/A Actions
    - [x] "N/A" actions (actions that are not applicable from the server. These actions are utilized for splitting complex quests)
    - [x] Only N/A actions can trigger a new sub-quest

- [x] Add .quest `options:` section
    - [x] Make search and space limits as options
    - [x] Add "omega" parameter for quests

- [x] Multithreading
    - [x] Introduce basic multithreading in the server by incorporating a worker sub-thread

- [x] Other
    - [x] Check the quality of the hash function used for the StatementSet
    - [x] Add the keyword `status [QuestName] ACTIVE|INACTIVE|DONE [goalIndex]` to save/quest files
    - [x] Add onSearchLimitReached and onSpaceLimitReached messages
    - [x] Do not recalculate the plan if limit was reached and state has not changed
    - [x] During the quest planning process, remove all non-relevant relations from the state, or construct a state containing only relevant relations
    - [x] Refactor the puzzle solver code
    - [x] Refactor the puzzle `.quest` files
    - [x] Complete the quest solver utility
    - [x] Add: MIT license
    - [x] Add: onNewQuestState

