#pragma once

#include "internal/flat_hash_map.hpp"

namespace jstd
{
    template <typename K,
              typename V,
              typename H = std::hash<K>,
              typename E = std::equal_to<K>,
              typename A = std::allocator<std::pair<K, V> > >
    using flat_hash_map = ska::flat_hash_map<K, V, H, E, A>;
}
