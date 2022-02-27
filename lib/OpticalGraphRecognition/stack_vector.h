#pragma once

#include <array>
#include <initializer_list>

namespace NOgr::NUtils {
    // Stack vector for the poor
    // FIXME: https://chromium.googlesource.com/chromium/chromium/+/master/base/stack_container.h
    template <class TItem, size_t MaxSize>
    class StackVector {
    public:
        class Iterator;
    public:
        StackVector() = default;
        StackVector(std::initializer_list<TItem> args) {
            for (auto&& arg : args) {
                array_[size_++] = std::move(arg);
            }
        }

        void PushBack(const TItem& item) {
            assert(size_ < MaxSize);

            array_[size_] = item;
            size_++;
        }

        Iterator begin() const {
            // FIXME: Create ConstIterator
            TItem* front = const_cast<TItem*>(&array_[0]);
            return Iterator(front);
        }

        Iterator end() const {
            // FIXME: Create ConstIterator
            TItem* front = const_cast<TItem*>(&array_[0]);
            return Iterator(front + size_);
        }

        bool Contains(const TItem& other) const {
            for (const auto& item : *this) {
                if (item == other) {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]] size_t Size() const {
            return size_;
        }

        [[nodiscard]] bool Empty() const {
            return !size_;
        }

    public:
        class Iterator {
        public:
            explicit Iterator(TItem* pitem) : pitem_(pitem) {}

            Iterator& operator++() {
                pitem_++;
                return *this;
            }

            bool operator!=(const Iterator& rhs) {
                return pitem_ != rhs.pitem_;
            }

            TItem& operator*() {
                return *pitem_;
            }

        private:
            TItem* pitem_;
        };

    private:
        size_t size_{0};
        std::array<TItem, MaxSize> array_;
    };
}
