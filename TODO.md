# Libmozok

Libmozok TODO list.

### Todo

- [ ] Add: support for commas in quest object, action, and sub-quest lists.
- [ ] Add: support arrays:
    - [ ] Add: `object arr[N,M,..] : Type` -> `arr_0_0`, `arr_0_1`, ...
    - [ ] Add: `rel(arr[i,j], arr[i+1, j])` -> `rel(arr_0_0, arr_1_0)` for all pairs of `(i,j)` that are consistent with the array size.
- [ ] Add: onNewQuestGoal
- [ ] Add: Error: The same object appears twice or more in the same statement.
- [ ] Add: Warning: Action argument was not used.
- [ ] Add: error codes for `libmozok::Result`
- [ ] In the `World::performPlanning` function, update only the quests that are activated but not yet completed. This can be achieved by creating a set of quests that need to be planned.
- [ ] Narration Engine
    - [ ] Implementation of multiple language support
    - [ ] Typed pattern matching with higher priority for the first match
    - [ ] A tool for verifying the completeness of the narration
- [ ] Migrate to C++17 (`[[nodiscard]]`, `<string_view>` etc.)
- [ ] Use `std::move()` to transfer ownership of resources and reduce unnecessary copy operations where possible. Additionally, use `const T` (together with `std::move()`) instead of `const T&` in function signatures to indicate that data has been copied.
- [ ] Ensure the order of the `status ...` commands in the save file matches the order in which they were triggered during the game-play.
- [ ] Add: tags support for quicker navigation in vim/nvim for `.quest` files.
- [ ] Quest debugging tool `mozok`(`.exe`). The main purpose of this tool is to model all the ways quest worlds can evolve during any possible gameplay:
    - [ ] Command files (`.qcf` quest command file) — files that describe how to properly load the quest world and all possible quest lines.
    - [ ] `world [name]` - create a new quest world
    - [ ] `load [world_name] [file_name]` - load a quest project
    - [ ] `run [command_file_name]` - execute a list of commands from a file
    - [ ] `apply [world].[action]([arg1],...)` - apply an action
    - [ ] `add [world].[rel]([obj1],...)` - add a statement
    - [ ] `rem [world].[rel]([obj1],...)` - remove a statent
    - [ ] `expect [what]` - adds an expectaion, raise error if expectation fails. E.g. `expect quest FAIL [fullname]`, `expect goal_change ...`.
    - [ ] `when_quest [event] [world].[quest_name] [debug_command]: ...` - sets a solver marker with a quest event as the condition (`NEW`, `DONE`, `UNREACHABLE`, `UNKNOWN`, `GOAL [id]`).
    - [ ] `when: [<conditions>] [debug_command]: [<command list>]` - sets a solver marker using a list of statements as the condition
        - [ ] Debug command `branch [name]` - creates a new branch. The debugger will try all branches by solving the quests up to the `[condition]`. Then it will first skip the `[<command list>]`, and in an alternative "timeline", execute the `[<command list>]`. This makes it possible to debug complex non-linear quests with multiple goals.
        - [ ] Debug command `do [name]` - simply executes the command list when conditions are met, without branching

### In Progress

- [ ] Basic documentation
    - [x] Doxygen generated documentation
    - [x] .quest format reference page
    - [ ] First Tutorial
- [ ] Add: other planning algorithms
    - [x] Add: `heuristic` setting to quest definition
    - [x] Implement *HSP* (Heuristic Search Planner) algorithm
    - [ ] Implement *GraphPlan* algorithm

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
    - [x] Add: "omega" parameter for quests
    - [x] Add: support of both A* & DFS search strategies as quest options `strategy ASTAR` and `strategy DFS`.

- [x] Multithreading
    - [x] Introduce basic multithreading in the server by incorporating a worker sub-thread

- [x] Logical consistency of the messages
    - [x] Add: `UNREACHABLE` to the `status` command
    - [x] Remove: the quest goal index from `INACTIVE` `status` command
    - [x] Add: `PARENT [parentQuestName]` to the `status` commands of sub-quests
    - [x] Ensure logical consistency of the status change command messages

- [x] Optimization
    - [x] Optimize `Quest::iterateOverApplicableActions` by organizing all possible actions into a tree structure, using precondition statements as nodes and action subsets as data. A node contains a statement that most effectively splits the set, with additional statements at each level further dividing the set of actions.

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
    - [x] Fix the highlighting for relation definitions with three or more arguments.
