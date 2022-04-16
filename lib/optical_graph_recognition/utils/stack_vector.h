#pragma once

#include <array>
#include <initializer_list>
#include <vector>
#include <exception>

namespace ogr::utils {
    // Stack vector for the poor
    // FIXME: https://chromium.googlesource.com/chromium/chromium/+/master/base/stack_container.h
    template <class TItem, size_t MaxSize>
    class StackVector {
    public:
        static const size_t kMaxSize = MaxSize;
        typedef TItem ItemType;
    public:
        class Iterator;
    public:
        StackVector() = default;
        StackVector(std::initializer_list<TItem> args) {
            for (auto&& arg : args) {
                array_[size_++] = std::move(arg);
            }
        }
        explicit StackVector(const std::vector<TItem>& items) {
            if (items.size() > MaxSize) {
                throw std::runtime_error{"StackVector constructor failed due to items size overflow"};
            }

            for (const auto& item : items) {
                array_[size_++] = item;
            }
        }

        void PushBack(const TItem& item) {
            assert(size_ < MaxSize);

            array_[size_] = item;
            size_++;
        }

        TItem& Back() {
            assert(size_ > 0);

            return array_[size_ - 1];
        }

        const TItem& Back() const {
            assert(size_ > 0);

            return array_[size_ - 1];
        }

        TItem& Front() {
            assert(size_ > 0);

            return array_[0];
        }

        const TItem& Front() const {
            assert(size_ > 0);

            return array_[0];
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

        const TItem& operator[](size_t index) const {
            return array_[index];
        }

        TItem& operator[](size_t index) {
            return array_[index];
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
