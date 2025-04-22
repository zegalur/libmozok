# LibMozok

<p align="center">
    <picture>
      <img alt="LibMozok Logo" src="./docs/imgs/libmozok_logo.svg">
    </picture>
</p>

**LibMozok** is a universal quest engine for games. This library implements a system heavily inspired by ["Hierarchical generation of dynamic and nondeterministic quests in games"](https://www.researchgate.net/publication/286454232_Hierarchical_Generation_of_Dynamic_and_Nondeterministic_Quests_in_Games).

*Mozok, derived from the Ukrainian word "мозок" meaning "brain."*

# Table of contents

- [Introduction](#introduction)
    - [Showcase](#showcase) 
    - [Example #1](#example-1)
    - [Example #2](#example-2)
    - [Example #3](#example-3)
- [Further Reading](#further-reading)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [References](#references)
- [Licenses](#licenses)

# Introduction

**LibMozok** is an experimental quest system for games. It is a universal library, capable of handling quests of various kinds and genres—whether they are linear or non-linear, big or small, connected or independent.

Quests are modeled as a set of preconditions, goals, and actions. Players start in an initial state and, through actions applied to the world, work to achieve (or fail) the quest goals. Depending on the current state of the world, the library determines if a quest goal is achievable, builds a quest plan (a list of actions to achieve the goal), and, if necessary, automatically triggers sub-quests.

To assist with world-building tasks, **LibMozok** introduces a specially designed `.quest` and `.qsf` formats for quest projects and scripts, very simple API to handle quest-related problems, and advanced quest debugging tool.

## Showcase

LibMozok generates a plan for a quest in real-time and detects when the quest becomes unsolvable:
![zegalur/libmozok-godot/docs/imgs/demo-01.gif](https://github.com/zegalur/libmozok-godot/blob/139a335af3f4eeff1eae35585b3aa9f9f33acbf9/docs/imgs/demo-01.gif)

From: https://github.com/zegalur/libmozok-godot

## Example #1

> While journeying to visit a distant relative, the princess has been captured by trolls in the forest. You are a brave knight who must find and save the princess. You are in the town now. You need to go to the castle, take your sword, travel to the forest, free the princess, and guide her back to the castle.

The first example is a simple quest with one goal and no sub-quests. Here is how the `.quest` file for this quest could look like (some parts of the file are omitted; for the full file, see [save_princess.quest](./src/libmozok/tests/quests/save_princess.quest)):

```nim
# ...

object knight : Knight
object princess : Princess

# ...

# Quest initial state.
rlist Initial:
    # All characters are alive.
    Alive(knight)
    Alive(princess)
    # ...

    # Roads between the places:
    # [ Forest ] <-=-> [ Town ] <-=-> [ Castle ]
    Road(forest, town)
    Road(town, forest)
    Road(town, castle)
    Road(castle, town)
    # ...

    # Initial locations.
    At(knight, town)
    At(princess, forest)
    # ...


# Travel from place A to place B by the road.
action TravelTo:
    player : Player
    location_A : Location
    location_B : Location
    pre At(player, location_A)
        Road(location_A, location_B)
        Alive(player)
    rem At(player, location_A)
    add At(player, location_B)

# Player takes a sword.
action TakeSword:
    player : Player
    sword : Sword
    location : Location
    pre At(player, location)
        At(sword, location)
        Alive(player)
    rem At(sword, location)
    add Has(player, sword)

#...

# One main quest.
main_quest SaveThePrincess:
    preconditions:
        # none
    goal:
        At(princess, castle)
    actions:
        KillEnemy
        TravelTo
        Guide
        Free_3
        TakeSword
    objects:
        knight
        bigSword
        princess
        troll_1
        troll_2
        troll_3
        castle
        town
        forest
    subquests:
        # none
```

And this is how a plan for the `SaveThePrincess` quest at the initial state may look like:

```nim
TravelTo ( knight, town, castle )
TakeSword ( knight, bigSword, castle )
TravelTo ( knight, castle, town )
TravelTo ( knight, town, forest )
KillEnemy ( knight, bigSword, troll_1, forest )
KillEnemy ( knight, bigSword, troll_2, forest )
KillEnemy ( knight, bigSword, troll_3, forest )
Free_3 ( knight, princess, troll_1, troll_2, troll_3, forest )
Guide ( knight, princess, forest, town )
Guide ( knight, princess, town, castle )
```

The player is allowed to perform any possible actions, not just those from the plan. Each action will change the quest world state, triggering the replanning of the quest. For example, if the player travels to the forest first, the planner will add a new action to the front of the plan:

```nim
TravelTo ( knight, forest, town )
...
```

When the player reaches the goal of the quest (either by following the plan or through some other combination of actions), the library will mark the quest as `DONE`. If the player's actions make it impossible to reach the goal, the library will mark the quest as `UNREACHABLE`, etc.

## Example #2

> "A farmer with a wolf, a goat, and a cabbage must cross a river by boat. The boat can carry only the farmer and a single item. If left unattended together, the wolf would eat the goat, or the goat would eat the cabbage. How can they cross the river without anything being eaten?" [[Wikipedia Link]](https://en.wikipedia.org/wiki/Wolf,_goat_and_cabbage_problem#The_story)

LibMozok can handle more complex "puzzle-like" quests. An example of this is the famous [Wolf-Goat-Cabbage Problem](https://en.wikipedia.org/wiki/Wolf,_goat_and_cabbage_problem). You can find a working [wolf_goat_cabbage.quest](./src/libmozok/tests/puzzles/wolf_goat_cabbage.quest) file in the `src\libmozok\tests\puzzles` directory, along with some other examples.

The possible winning plan for this quest at the initial state may look like this:

```nim
MoveLoad1 ( human, left, right, goat, wolf, cabbage )
MoveFree1 ( human, right, left, goat, wolf, cabbage )
MoveLoad2 ( human, left, right, wolf, cabbage, goat )
MoveLoad2 ( human, right, left, goat, wolf, cabbage )
MoveLoad2 ( human, left, right, cabbage, goat, wolf )
MoveFree2 ( human, right, left, wolf, cabbage, goat )
MoveLoad3 ( human, left, right, goat, wolf, cabbage )
```

**Warning!** Libmozok was designed for planning quests in real-time for games, and was not intended to be a universal puzzle-solving library. That said, it is powerful enough to solve many hard puzzles, although it can take some time and memory to find a solution. An example of a complex puzzle is [game_of_fifteen.quest](./src/libmozok/tests/puzzles/game_of_fifteen.quest).

## Example #3

The third example demonstrates the power of `.qsf` scripts and the quest debugging tool (mozok). With this tool, you can generate a complete visualization of your non-linear gameplay — clearly revealing bottlenecks, slowdowns, unreachable states, and how your storylines split and evolve over time:

```ini
# ...

# ======================== Header section ========================
# Defines which worlds to create, which projects to load 
# (and in what order), and how to properly initialize the game.

worlds:
    tut 
projects:
    [tut] tutorial_utils.quest
    [tut] tutorial_controls.quest
    [tut] tutorial_fighting.quest
    [tut] tutorial_key.quest
    [tut] tutorial_puzzle.quest
    [tut] tutorial_main.quest
init:
    [tut] InitTutorials()

# ...

# ======================== Debug section ========================
# Describes how to simulate and test all non-linear gameplay.

onInit:
ACT ON_INIT:
    print ------------- Hello, world! ------------- 

onNewMainQuest [tut] FinishAllTutorials:
ACT TUTORIAL_STARTED:
    print Tutorial quest started, enjoy!

onNewQuestStatus [tut] PuzzleTutorial_GetHeart UNREACHABLE:
ACT_IF BLOCK_EXIT:
    push [tut] PTut_Cancel(pt_cell_00, puzzleTutorial, puzzleTutorial_GetHeart)

onNewSubQuest [tut] PuzzleTutorial_GetHeart _ _:
SPLIT BLOCK_EXIT:
    expect UNREACHABLE [tut] PuzzleTutorial_GetHeart
    push [tut] PTut_BlockExit()

onAction [tut] ApplyTutorialAction(pickUpKeyAction):
ACT ON_TST_ACTION:
    print The key was picked up!

# ...
```

Resulting SVG file:

<picture>
<img height="750px" alt="Simulation Graph" src="./docs/imgs/simulation_example.svg">
</picture>

# Further Reading

- Getting Started:
    - *Under Construction*
- Demo Projects:
    - https://github.com/zegalur/libmozok-godot
- Manual:
    - [`.quest` Format Reference](docs/quest-format-reference.md)
    - [`.qsf` Format Reference](docs/qsf-format-reference.md)
    - [How to use quest debugging tool](docs/debugger.md)
    - Doxygen auto-generated reference
- Editor support:
    - [Vim/NeoVim Support Manual](.vim/README.md)
- Other:
    - [Hierarchical generation of dynamic and nondeterministic quests in games](https://www.researchgate.net/publication/286454232_Hierarchical_Generation_of_Dynamic_and_Nondeterministic_Quests_in_Games)
    - [STRIPS](https://en.wikipedia.org/wiki/Stanford_Research_Institute_Problem_Solver)

# Prerequisites

LibMozok is written in C++, using CMake as a build tool, and VSCode as the default development environment. To set up the development environment, make sure you have the following prerequisites installed:

- Core API Requirements:
    - C++11 compatible compiler
    - CMake (version 3.29 or higher)
    - Doxygen
- Highlight support for `.quest` files:
    - VSCode (version 1.89 or higher)
- Graph PDF and SVG representation:
    - [`Graphviz`](https://graphviz.org/)

# Installation

How to build and install **libmozok** (Release):

1. Navigate to the build directory:<br /> 
`cd [WorkDir]/libmozok/build`

2. Run CMake to configure the project:<br /> 
`cmake ../src`
3. Build the project:<br /> 
`cmake --build . --config Release`
4. Install the project:<br /> 
`cmake --install . --config Release`
5. You should now have the non-empty `lib/` and `include/libmozok/` directories inside the `install` directory.

# References

* Soares de Lima, Edirlei & Feijó, Bruno & Furtado, Antonio. (2014). *Hierarchical Generation of Dynamic and Nondeterministic Quests in Games.* ACM International Conference Proceeding Series. 2014. 10.1145/2663806.2663833. Web. https://www.researchgate.net/publication/286454232_Hierarchical_Generation_of_Dynamic_and_Nondeterministic_Quests_in_Games

* Lacan, Olivier. *Keep a Changelog*. 5 Mar. 2023. Web. https://keepachangelog.com/en/1.1.0/

* Preston-Werner, Tom. Semantic Versioning. Web. https://semver.org/spec/v2.0.0.html

* Kolpackov, Boris. *P1204R0: Canonical Project Structure*, 8 Oct. 2018. Web. https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1204r0.html

* Gasser, Ralph Udo. Harnessing *Computational Resources for Efficient Exhaustive Search*. PhD Thesis, ETH Zürich, 1995

* Brüngger, Adrian; Marzetta, Ambros; Fukuda, Komei; Nievergelt, Jurg. *The parallel search bench ZRAM and its applications*. Ann. Oper. Res. **90** (1999), 45-63.

# Licenses

All files on this repository are subject to the MIT license. Please read the `LICENSE` file at the root of the project.
