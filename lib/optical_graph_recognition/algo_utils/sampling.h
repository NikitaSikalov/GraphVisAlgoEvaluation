#pragma once

#include <map/composite_map.h>

#include <set>
#include <utility>

namespace ogr::algo {
    template <typename Iterable>
    inline std::set<std::pair<typename Iterable::value_type, typename Iterable::value_type>>
    GetUniquePairs(const Iterable& iterable) {
        using Value = typename Iterable::value_type;
        map::CompositeMap<bool, Value, Value> used;
        std::set<std::pair<Value, Value>> result;

        for (const Value& val1 : iterable) {
            for (const Value& val2 : iterable) {
                if (used.Contains(val1, val2)) {
                    continue;
                }

                result.emplace(std::make_pair(val1, val2));
            }
        }

        return result;
    }
}
