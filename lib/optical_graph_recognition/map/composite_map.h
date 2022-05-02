#pragma once

#include <unordered_map>
#include <tuple>

namespace std {
    // Keys hash definition
    template <typename... Args>
    struct hash<std::tuple<Args...>> {
        size_t operator()(const std::tuple<Args...>& key) const {
            return std::apply([](auto... v) { return (... ^ std::hash<decltype(v)>{}(v)); }, key);
        }
    };
}

namespace ogr::map {
    template <typename... Keys>
    inline std::tuple<Keys...> MakeCompositeKey(Keys... keys);

    template <typename Value, typename... Keys>
    class IMap {
    public:
        using KeyType = std::tuple<Keys...>;

    public:
        virtual bool Contains(Keys... keys) const = 0;
        virtual bool Contains(const KeyType& key) const = 0;
        virtual Value& operator()(Keys... keys) = 0;
        virtual Value& operator[](const KeyType& key) = 0;
        virtual void Clear() = 0;

        virtual typename std::unordered_map<KeyType, Value>::iterator begin() = 0;
        virtual typename std::unordered_map<KeyType, Value>::iterator end() = 0;
    };

    template <typename Value, typename... Keys>
    class CompositeMap : public IMap<Value, Keys...> {
        using KeyType = typename IMap<Value, Keys...>::KeyType;

    public:
        bool Contains(Keys... keys) const override {
            auto key = MakeCompositeKey(keys...);
            return map_.contains(key);
        }

        bool Contains(const KeyType& key) const override {
            return map_.contains(key);
        }

        Value& operator()(Keys... keys) override {
            auto key = MakeCompositeKey(keys...);
            return map_[key];
        }

        Value& operator[](const KeyType& key) override {
            return map_[key];
        }

        void Clear() override {
            map_.clear();
        }

        typename std::unordered_map<KeyType, Value>::iterator begin() override {
            return map_.begin();
        }

        typename std::unordered_map<KeyType, Value>::iterator end() override {
            return map_.end();
        }

    private:
        std::unordered_map<KeyType, Value> map_;
    };
}
