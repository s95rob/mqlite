#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <set>

template <typename T>
using weak_ptr_set = std::set<std::weak_ptr<T>, std::owner_less<std::weak_ptr<T>>>;