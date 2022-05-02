#pragma once

#include <map/composite_map.h>

#include <unordered_set>

namespace ogr::map {
    template <size_t ResetThreshold, typename Value, typename... Keys>
    class ResetMapDecorator : public IMap<Value, Keys...> {
        using KeyType = typename IMap<Value, Keys...>::KeyType;

    public:
        explicit ResetMapDecorator(IMap<Value, Keys...>& map) : map_(map) {}

        bool Contains(Keys... key) const override {
            return map_.Contains(key...);
        }

        Value& operator()(Keys... key) override {
            return map_(key...);
        }

        void Clear() override {
            map_.Clear();
        }

        void TryReset() {
            if (++counter_ == ResetThreshold) {
                for (auto& [key, value] : map_) {
                    if (!used_.contains(key)) {
                        value = Value();
                    }
                }
                counter_ = 0;
            }
        }

        typename std::unordered_map<KeyType, Value>::iterator begin() override {
            return map_.begin();
        }

        typename std::unordered_map<KeyType, Value>::iterator end() override {
            return map_.end();
        }

    private:
        IMap<Value, Keys...>& map_;
        std::unordered_set<KeyType> used_;
        uint64_t counter_{0};
    };
}
