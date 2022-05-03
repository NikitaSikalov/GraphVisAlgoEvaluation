#pragma once

#include <map/composite_map.h>

#include <unordered_set>

namespace ogr::map {
    template <size_t ResetThreshold, typename Value, typename... Keys>
    class ResetMapDecorator : public IMap<Value, Keys...> {
        using KeyType = typename IMap<Value, Keys...>::KeyType;

    public:
        explicit ResetMapDecorator(CompositeMap<Value, Keys...>&& map) : map_(std::move(map)) {}

        bool Contains(Keys... key) const override {
            return map_.Contains(key...);
        }

        bool Contains(const KeyType& key) const override {
            return map_.Contains(key);
        }

        Value& operator()(Keys... key) override {
            used_(key...) = true;
            return map_(key...);
        }

        Value& operator[](const KeyType& key) override {
            return map_[key];
        }

        void Clear() override {
            map_.Clear();
        }

        void TryReset() {
            if (++counter_ == ResetThreshold) {
                for (auto& [key, value] : map_) {
                    if (!used_.Contains(key)) {
                        value = Value();
                    }
                }
                counter_ = 0;
                used_.Clear();
            }
        }

        typename std::unordered_map<KeyType, Value>::iterator begin() override {
            return map_.begin();
        }

        typename std::unordered_map<KeyType, Value>::iterator end() override {
            return map_.end();
        }

    private:
        CompositeMap<Value, Keys...> map_;
        CompositeMap<bool, Keys...> used_;
        uint64_t counter_{0};
    };
}
