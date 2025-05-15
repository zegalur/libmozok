# 1. Getting Started

In this tutorial, we’ll walk through creating and running your first quest using LibMozok. You’ll learn how to define a simple world, describe quests, and simulate the game using the command-line app.

All files used in this tutorial are available in the `docs/code` directory.

## Table of Contents

1. [Before We Start](#before-we-start)
2. [Quest Description](#quest-description)
3. [Main QSF](#main-qsf)
4. [Quest Worlds](#quest-worlds)
5. [Quest Projects](#quest-projects)
6. [Game Model](#game-model)
    1. [Objects & Types](#objects--types)
    2. [Relations](#relations)
    3. [Actions](#actions)
    4. [Quests](#quests)
    5. [Initialization](#initialization)
7. [Final Project File](#final-project-file)

## Before we start

Download and install the LibMozok as described in the [README.md](../README.md#installation). You’ll need the mozok executable available at `<repo>/install/bin`. Throughout this tutorial, we’ll refer to it as `<mozok>`.

For better development experience, it’s recommended to install syntax highlighting plugins for (Neo)Vim or VSCode.

LibMozok is an *experimental* quest engine—you're entering uncharted territory with no established best practices. Expect to encounter unusual and challenging problems :)

## Quest Description

Let's start with a very simple quest:
```
You wake up in a dark room.  
Turn ON the room light.
```
For our first quest, we won’t make any assumptions beyond this basic setup. It will be as minimal as possible - just enough to get everything working.

## Main QSF

Create a directory where you’ll store all files for this tutorial. Let’s assume it’s `<tut>/01/`. Inside that directory, create a `main.qsf` file with the following content:

```nim
# main.qsf
version 1 0
script main

worlds: 
    # The list of quest worlds (first letter is a lowercase):
    #example_world
projects:
    # The list of `.quest` project files:
    #[example_world] relative/path/to/example.quest
init:
    # The list of init-actions:
    #[example_world] ExampleInitAction(arg1,arg2)
```

This `.qsf` file (**Q**uest **S**cript **F**ile) defines the overall structure of your game and how it should be initialized. It acts as the main entry point for the quest engine.

In real games, you’d integrate LibMozok into your codebase. For now, we’ll simulate the game using the mozok command-line app. Run the mozok app:
```
<mozok> main.qsf -v -P OK
() INFO: =============== NEW TIMELINE ===============
() INFO: All quest expectations are met.
OK
```
The -v flag enables verbose output, and -P OK tells mozok to print OK if the simulation completes successfully.

## Quest worlds

A *quest world* contains all objects, relations, actions, and quests. Each world is independent, and a game may contain multiple worlds. In our case, we only need one:

```nim
...
worlds:
    tut
...
```
Run the mozok app again to ensure everything works:
```
<mozok> main.qsf -v -P OK
() INFO: =============== NEW TIMELINE ===============
() INFO: All quest expectations are met.
OK
```

## Quest projects
A *quest project* defines a part of a quest world. Projects are written in .quest files. Our simple game only needs one project, but complex games often have many.

Create `room_light.quest` with the following:

```nim
# room_light.quest
version 1 0
project room_light

# ...
```
Then include this file in the `projects:` section of `main.qsf`:

```nim
...
projects:
    [tut] room_light.quest
...
```
## Game model

The world is built using types, objects, relations, and actions.

### Objects & Types

**Objects** are things in the game. Every object exists as a single instance and has a unique name. The name should start with a lowercase letter. It's not mandatory, but by convention, we end object names with an underscore (_). **Types** categorize objects and may inherit from other types. Type has a unique name (start with uppercase letter):

```nim
type Type1
type Type2
type Type3 : Type1, Type2

object obj_1_ : Type2, Type1
object obj_2_ : Type3
```
In the example above, the objects `obj_1_` and `obj_2_` have different typesets, though they share some types in common. A **typeset** is the complete set of type names associated with an object. For `obj_1_`, the typeset is `{Type1, Type2}`, and for `obj_2_`, it's `{Type1, Type2, Type3}`. You can think of a typeset as the union of all types assigned to the object.

The specific types and objects you'll need depend entirely on how you choose to model your game. In our case, let's define:
```nim
type Room
object living_room_ : Room
```

### Relations

We have objects, which might suggest we're dealing with OOP (Object-Oriented Programming), but that's **not** the case - objects in LibMozok do not have any internal state. Instead, the game state is represented as a list of **relations** between objects. These relations are dynamic: they can appear and disappear over time. A list of relations defines the current world state and can be used as a precondition or as a goal in a quest.

Each relation has a unique name (starting with an uppercase letter) and a list of argument types:
```
rel RelationName(ArgType1, ArgType2)
```

In our case, we only need one relation, but for demonstration purposes, let's define two:
```nim
rel LightOff(Room)
rel LightOn(Room)
```

When a relation is applied to specific objects in the game state, it becomes a statement. For example, `LightOn(living_room_)` is a statement.

### Actions

**Actions** describe how the game state can change. Each action has a name, a list of arguments, and three sections:
* `pre` : preconditions
* `rem` : statements to remove
* `add` : statements to add
Here’s the one we need:
```nim
action TurnLightOn:
    room : Room
    pre LightOff(room)
    rem LightOff(room)
    add LightOn(room)
```
When invoked, this action checks if `LightOff(living_room_)` is present, removes it, and adds `LightOn(living_room_)`.

All action parameters will be unique objects. You cannot use the same object twice in one action call like `Action(obj_, obj_)`.

The `pre`, `rem`, and `add` sections are always required, even if your action doesn't use them. In that case, simply leave them empty or include a `# none` comment. For example:
```nim
action Test:
    obj : Type
    pre # none
    rem # none
    add Pre1(obj)
        Pre2(obj)
```

### Quests

Quests define goals in the game. A quest has:

- a unique name (starts with uppercase letter)
- preconditions (can be empty)
- a list of goals
- allowed actions (vertical list of action names and action group names)
- allowed objects (vertical list of object names or type names)
- subquests (can be empty)

There are two types:
- **Main quests**: triggered when preconditions are met
- **Subquests**: triggered by `N/A` (non-applicable) actions inside already activated quest (will be covered later)

For now, we’ll just use one main quest:
```nim
main_quest TurnLivingRoomLightOn:
    preconditions:
        LightOff(living_room_)
    goal:
        LightOn(living_room_)
    actions:
        TurnLightOn
    objects:
        living_room_
    subquests:
        # none
```

### Initialization

We also need to define an initialization action to set the starting state:

```nim
action Init_RoomLight:
    pre # none
    rem # none
    add LightOff(living_room_)
```
This is a global action, because it uses a global object name (`living_room_`) instead of argument name. Global actions cannot be listed in a quest’s `actions:` section. Additionally, for theoretical reasons, any action that uses a 0-arity relation (a relation with no arguments) is also considered global.

### Final project file

Here’s the full content of `room_light.quest`:
```nim
# room_light.quest
version 1 0
project room_light

type Room

object living_room_ : Room

rel LightOff(Room)
rel LightOn(Room)


action TurnLightOn:
    room : Room
    pre LightOff(room)
    rem LightOff(room)
    add LightOn(room)


main_quest TurnLivingRoomLightOn:
    preconditions:
        LightOff(living_room_)
    goal:
        LightOn(living_room_)
    actions:
        TurnLightOn
    objects:
        living_room_
    subquests:
        # none


action Init_RoomLight:
    pre # none
    rem # none
    add LightOff(living_room_)

```

Update `main.qsf` to include the initialization action:

```nim
...
init:
    [tut] Init_RoomLight()
...
```
Now run the app:
```
<mozok> main.qsf -V -P OK
() INFO: =============== NEW TIMELINE ===============
() INFO: EVENT: onNewMainQuest [tut] TurnLivingRoomLightOn
() INFO: EVENT: onNewQuestStatus [tut] TurnLivingRoomLightOn REACHABLE
() INFO: EVENT: onNewQuestPlan [tut] TurnLivingRoomLightOn
() INFO:        New plan accepted for [tut] TurnLivingRoomLightOn:
() INFO:        - TurnLightOn
() INFO: pushAction [tut] TurnLightOn(living_room_)
() INFO: EVENT: onNewQuestState [tut] TurnLivingRoomLightOn
() INFO: EVENT: onNewQuestStatus [tut] TurnLivingRoomLightOn DONE
() INFO: EVENT: onNewQuestPlan [tut] TurnLivingRoomLightOn
() INFO: All quest expectations are met.
OK
```

Success! We’ve created and completed our first quest. As you can see, the quest was activated, a valid plan was found, and it was completed by applying the `TurnLightOn(living_room_)` action.

<hr>

**Next Tutorial:** [Goals & Subquests](02-goals_and_subquests.md)
