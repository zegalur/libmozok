# Quest format reference (v.0.1.1)

To assist with world-building tasks, **LibMozok** introduces a specially designed `.quest` format for quest projects. This format is a type-safe, hierarchical, and quest-oriented version of [STRIPS](https://en.wikipedia.org/wiki/Stanford_Research_Institute_Problem_Solver).

## Example .quest project

```nim
# Single-line comments start with the # symbol.

# The first command is always `version X Y`.
version 1 0

# The second command is always `project <project_name>`, where `<project_name>` 
# is a unique project name (without spaces).
project my_project


# ======== TYPES ======== #

# Defines new types.
# Type names always start with an uppercase letter.
type Location
type Cursed

# Defines subtypes. Multiple supertypes are supported.
type CursedLocation : Location, Cursed


# IMPORTANT: You can only use names that were previously defined.


# ======== OBJECTS ======== #

# Defines new objects. Objects are indestructible and non-copyable entities.
# Object names always start with a lowercase letter.
object town : Location
object heaven : Location
object forest : Location

# This object has the types: CursedLocation, Location, Cursed
object cave : CursedLocation

# But this object has the types: Location, Cursed
# These two objects are not identical in their type!
object cave_X : Location, Cursed


# ======== RELATIONS ======== #

# Defines new relations.
# Relation names always start with an uppercase letter.
rel At(Location)
rel Road(Location, Location) # from, to
rel CursedRoad(CursedLocation, Location) # from, to


# ======== RELATION LIST ======== #

# Relation lists are macro-commands; they are substituted during loading.
# Relation list names always start with an uppercase letter.
rlist InitialState:
    # Arguments:
    initialLocation : Location

    # Substitution is a list of previously defined relations and rlists:
    At(initialLocation)

    # You can refer to objects other than arguments:
    # [cave] <== [forest] <=> [town]
    Road(town, forest)
    Road(forest, town)
    Road(forest, cave)
    CursedRoad(cave, forest)


# ======== ACTIONS ======== #

# Defines a new action group (name starts from a lower case letter).
agroup movement

# Actions define how we can interact with the world.
# Action names always start with an uppercase letter.
# In this case action also will be included into `movement` action group.
action MoveTo {movement}:
    # Arguments:
    prevLocation : Location
    newLocation : Location

    # Action preconditions.
    # In order to be applicable, action preconditions must hold.
    pre Road(prevLocation, newLocation)
        At(prevLocation)

    # Statements after `rem` will be removed.
    rem At(prevLocation)

    # Statements after `add` will be added.
    add At(newLocation)


# Another example:
action RemoveCurse:
    cursedLocation : CursedLocation
    otherLocation : Location
    pre At(cursedLocation)
        CursedRoad(cursedLocation, otherLocation)
    rem CursedRoad(cursedLocation, otherLocation)
    add Road(cursedLocation, otherLocation)


# This action can be used for initializing the world:
action Init:
    pre # none
    rem # none
    # Actions that refer to objects
    # other than arguments are called `global`.
    # This action is therefore a global action.
    add InitialState(town)


# ======== QUESTS ======== #

# Defines a new subquest. Quest names start with an uppercase letter.
# You can only use objects and actions that are explicitly listed.
quest MoveToCursedCave:
    preconditions:
        # Quest preconditions are here.
    goal:
        # Quest goals are here.
        At(cave)
    actions:
        # The (vertical) list of allowed actions:
        # In this case we use an action group to include movement actions.
        movement
    objects:
        # The (vertical) list of allowed objects:
        town
        forest
        cave
        # To include every object of some type, just write a type name. E.g.
        #Location
    subquests:
        # none


# In order to use a subquest, you must also define a special action,
# marked as N/A (Not Applicable). This action will represent the subquest.
# N/A actions can't be applied, but they can appear in a quest plan and
# they can trigger a subquest.
action N/A MoveToCursedLocation:
    cursedLocation : CursedLocation
    pre # none
    rem # none
    add At(cursedLocation)


# The main quest will automatically open after an action is applied,
# once the preconditions are met.
main_quest FinishYourMission:
    options:
        # Quests can have various additional options:
        searchLimit 100 # Sets a search limit (positive integer)
        spaceLimit 100 # Sets a space limit (positive integer)
        omega 1 # Sets omega value to a given positive integer value
        heuristic SIMPLE # Sets the search heuristic to `SIMPLE`
    preconditions:
        # none
    goal:
        # The first quest goal is clearly UNREACHABLE.
        # The planner will switch to the second goal.
        At(heaven)
    goal:
        # This goal is REACHABLE.
        # The planner will build a new plan for this goal.
        # The goal is to open the road from the cave back to the forest.
        Road(cave, forest)
    actions:
        MoveToCursedLocation
        RemoveCurse
    objects:
        cave
        forest
        heaven
    subquests:
        MoveToCursedCave
```

This .quest file is available at [cursed_cave.quest](../src/libmozok/tests/quests/cursed_cave.quest). It is also added as a test solver test. The possible outcome is:

```
> New main quest: FinishYourMission
> New quest status: FinishYourMission = MOZOK_QUEST_STATUS_REACHABLE
> New subquest: MoveToCursedCave. Parent quest: FinishYourMission
> New quest status: MoveToCursedCave = MOZOK_QUEST_STATUS_REACHABLE
1 : MoveTo ( town, forest )
2 : MoveTo ( forest, cave )
3 : RemoveCurse ( cave, forest )
> New quest status: MoveToCursedCave = MOZOK_QUEST_STATUS_DONE
> New quest status: FinishYourMission = MOZOK_QUEST_STATUS_DONE
```

## Language

### Names (Identifiers)

- Only `a-z`, `A-Z`, `0-9`, and `_` are allowed in names.
- No spaces in the names.
- **Type** names always start with an **uppercase** letter.
- **Object** names always start with a **lowercase** letter.
- **Relation** names always start with an **uppercase** letter.
- **Relation list** names always start with an **uppercase** letter.
- **Action** names always start with an **uppercase** letter. 
- **Quest** names start with an **uppercase** letter

### Comments

```nim
# This is a single-line comment.
# Single-line comments start with the # symbol.
```

### Keywords

Keywords are reserved words and must not be used in naming.

| Keyword | Description |
| ------- | ----------- |
| `version` | Specifies the version of the .quest file format.
| `project` | Sets the project name.
| `type` | Defines a new type.
| `object` | Defines a new object.
| `include` | *Reserved but not functional yet.*
| `rel` | Defines a new relation.
| `rlist` | Defines a new relation list.
| `agroup` | Defines a new action group.
| `action` | Defines a new action.
| `N/A` | Marks an action as *Not Applicable*.
| `pre` | Marks the beginning of an action's preconditions block.
| `rem` | Marks the beginning of an action's removal block.
| `add` | Marks the beginning of an action's addition block.
| `quest` | Defines a new subquest.
| `main_quest` | Defines a new main quest.
| `preconditions` | Quest's preconditions block.
| `goal` | Quest goal block.
| `actions` | Quest allowed actions list.
| `objects` | Quest allowed objects list.
| `subquests` | Quest subquests list.
| `status` | Changes the current status of a quest.
| `ACTIVE` | Used in the `status` command.
| `INACTIVE` | Used in the `status` command.
| `DONE` | Used in the `status` command.
| `options` | Quest options block.
| `searchLimit` | Sets the search limit.
| `spaceLimit` | Sets the space limit.
| `omega` | Sets the omega value of the `SIMPLE` heuristic.
| `heuristic` | This quest option sets the quest heuristic function (default `SIMPLE`)
| `SIMPLE` | Simple heuristic (used in `heuristic`).
| `HSP` | Heuristic from HSP algorithm (used in `heuristic`).
| `use_atree` | This quest option forces to use action tree structure to boost the performance.
| `strategy` | This quest option sets the search strategy (default `ASTAR`).
| `ASTAR` | Search the plan using A\*.
| `DFS` | Search in depth. For the cases when plan is long but straighforward.

### Statement

A relation with objects as arguments is called a *statement*:
```nim
At(cursedLocation)
CursedRoad(cursedLocation, otherLocation)
Example() # An example of a 0-arity statement
```

### State

A state of a quest world is just a set of statements.

### Global Statements / Global Actions

Any 0-arity statement, and any statement that refers to any object other than the current action argument, or allowed quest object of the current quest, is called  *global*. Any action with a global statement is also *global*. **Global statements and actions are not allowed in a quest definition!**

### Status change commands

An action can be equipped with a list of status change commands. These commands can alter the current status of quests. Typically, you can find them in the generated `Load` actions:

```nim
# An example of a `Load` action with status change commands.
action Load:
    # Status change commands:
    # status <QuestName> <STATUS> <GoalIndex>
    status Quest_1 INACTIVE # set a quest as inactive
    status Quest_2 ACTIVE 0 # set a main quest as active (with current goal 0)
    status Quest_3 DONE 2 # set a main quest as finished (goal 2 finished)
    status Quest_4 UNREACHABLE # set a main quest as unreachable
    # Set a subquest as active with goal 3 
    # (parent quest is Quest_2 with the corresponding goal 0)
    status SubQuest_1 ACTIVE 3 PARENT Quest_2 0
    pre # none
    rem # none
    add # Current state statements list
        # ...
```

# References

The structure of this reference page is similar to the official [GDScript Reference page](https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_basics.html).
