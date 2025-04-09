// Copyright 2024 Pavlo Savchuk. Subject to the MIT license.

#pragma once

#include <libmozok/public_types.hpp>

#include <utility>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mozok {

// ============================== Basic Types =============================== //

// IMPORTANT! ID type must support negative values!
using ID = int;

using SIZE_T = std::size_t;

// =============================== Containers =============================== //

template<typename F, typename S>
using Pair = std::pair<F,S>;

template<typename T>
using Queue = std::queue<T>;

template<typename K, typename V, typename H, typename E> 
using HashMap = std::unordered_map<K, V, H, E>;

template<typename T, typename H, typename E> 
using HashSet = std::unordered_set<T, H, E>;

template<typename K, typename V> 
using UnorderedMap = std::unordered_map<K, V>;

template<typename T> 
using UnorderedSet = std::unordered_set<T>;

template<typename T, typename Comp> using PriorityQueue = 
        std::priority_queue<T, Vector<T>, Comp>;

// ============================= Smart Pointers ============================= //

template<typename T> 
using UniquePtr = std::unique_ptr<T>;

template<typename T> 
using SharedPtr = std::shared_ptr<T>;

template<typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
UniquePtr<T> makeShared(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// ============================= Multithreading ============================= //

using Thread = std::thread;
using Mutex = std::mutex;
using LockGuard = std::lock_guard<Mutex>;
using UniqueLock = std::unique_lock<Mutex>;
using ConditionVariable = std::condition_variable;

template<typename T> 
using Atomic = std::atomic<T>;

// ================================= Chrono ================================= //

/// @brief One quest server tick is (1/24) sec or 40 milliseconds.
constexpr auto ONE_QUEST_TICK = std::chrono::milliseconds(40); 


}
