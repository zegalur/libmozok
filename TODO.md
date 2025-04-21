# Libmozok

Libmozok TODO list.

### Todo

- [ ] Add: support for commas in quest object, action, and sub-quest lists.
- [ ] Add: support arrays:
    - [ ] Add: `object arr[N,M,..] : Type` -> `arr_0_0`, `arr_0_1`, ...
    - [ ] Add: `rel(arr[i,j], arr[i+1, j])` -> `rel(arr_0_0, arr_1_0)` for all pairs of `(i,j)` that are consistent with the array size.
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
    
### In Progress

- [ ] Basic documentation
    - [x] Doxygen generated documentation
    - [x] .quest format reference page
    - [ ] First Tutorial
- [ ] Add: other planning algorithms
    - [x] Add: `heuristic` setting to quest definition
    - [x] Implement *HSP* (Heuristic Search Planner) algorithm
    - [ ] Implement *GraphPlan* algorithm
- [ ] Add: Quest debugging tool `mozok` (`.exe`):
    - [x] Add: `FileSystem` class as a part of public interface
    - [x] Add: `Result Server::loadQuestScriptFile(script, FileSystem&)` for parsing the "loading" part of script files (excluding debugging parts)
    - [ ] Auto-generate a help Markdown document into `docs`
    - [ ] Generate a tree representation in a popular format: 
        - [ ] Include information on how long planning took for each node
        - [ ] Connect nodes with identical hash values (states)
- [ ] `.qsf` quest script file:
    - [ ] Add: `generateQSFSaveFile` to `Server`
    - [ ] Add: Syntax highlight for the `.qsf` files
        - [ ] Vim / NeoVim
        - [ ] VSCode
    - [x] Commands: `info`, `exit`, `pause` and `continue`
    - [x] Blocks: `worlds:`, `projects:`, `init:`, `debug:`
    - [ ] `std_world name` - sets the standard world name
    - [ ] `load [world_name] file_name` - loads a quest project
    - [ ] `run [command_file_name]` - executes a list of commands from a `qsf` file
    - [x] `push [world] action(arg1,...)` - pushes an action into the action queue
    - [x] `expect UNREACHABLE`
    - [ ] `expect DONE <goal>`
    - [ ] `assert [what]` - asserts a condition and raises an error if it fails.
    - [x] Event handlers: `ACT`, `ACT_IF`, `ALWAYS`, `SPLIT`
    - [ ] Add: `SPLIT_IF`, `ALWAYS_IF`, `ACT_IF_NOT`, `IF_NOT`, `SPLIT_IF_NOT`
    - [x] Placeholder parameters `_`
    - [ ] Options:
        - [ ] Max wait time
        - [ ] Color output
    - [ ] Events:
        - [x] `onInit`
        - [ ] `onCheck`:
            - [ ] `pushCheck` to `Server` (with data)
            - [ ] `onCheck` event for `MessageProcessor` (with check status `true,false,error` and data)
        - [x] `onAction`
        - [ ] `onActionError`
        - [x] `onNewMainQuest`
        - [x] `onNewSubQuest`
        - [ ] `onNewQuestState`
        - [x] `onNewQuestStatus`
        - [ ] `onNewQuestPlan`
        - [x] `onSearchLimitReached`
        - [x] `onSpaceLimitReached`

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
    - [x] Add: `onNewQuestGoal(newGoal,oldGoal)` event
