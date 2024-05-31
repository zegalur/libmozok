// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>
#include <libmozok/result.hpp>

namespace mozok {

class World;

/// @brief Adds a project to the world.
/// Project is a collections of types, objects, relations, relation lists,
/// actions and quests, which collectively form a cohesive and logical whole.
/// WARNING! This operation does not function as a transaction. An error during 
/// the addition process could result in an incomplete game world.
/// @param world The world to which definitions will be added.
/// @param projectFileName The project file name. 
///         Utilized only for error handling purposes.
/// @param projectSrc Project source code in the .quest format.
/// @return Returns the status of the operation.
mozok::Result addFromProjectSRC(
        mozok::World* world, 
        const mozok::Str& projectFileName,
        const mozok::Str& projectSrc
        ) noexcept;

}