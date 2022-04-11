#pragma once

#include <vector>

namespace ogr::utils {
    template <typename T>
    using Row = std::vector<T>;

    template <typename T>
    using Matrix = std::vector<Row<T>>;

    template <typename TDerived, typename TBase>
    std::enable_if_t<std::is_base_of_v<TBase, TDerived>, TDerived*>
    As(const TBase* base_ptr) {
        auto* derived_ptr = dynamic_cast<TDerived*>(base_ptr);
        if (derived_ptr == nullptr) {
            throw std::runtime_error{"Invalid dynamic cast"};
        }

        return derived_ptr;
    }

    template <typename TDerived, typename TBase>
    std::enable_if_t<std::is_base_of_v<TBase, TDerived>, bool>
    Is(const TBase* base_ptr) {
        return dynamic_cast<TDerived*>(base_ptr) != nullptr;
    }
}