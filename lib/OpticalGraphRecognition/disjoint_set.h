#pragma once

namespace NOgr::NUtils {
    class DisjointSet {
    public:
        using SetId = size_t;
    public:
        explicit DisjointSet(SetId id) : id_(id), parent_(this) {}

        template <typename Iterable>
        friend void MergeSets(Iterable sets);

        SetId GetId() {
            if (parent_ == this) {
                return id_;
            }
            return parent_->GetId();
        }

        DisjointSet* GetSetSource() {
            if (parent_ == this) {
                return this;
            }

            parent_ = parent_->GetSetSource();
            return parent_;
        }

        void Merge(DisjointSet* set) {
            DisjointSet* set_source = GetSetSource();
            set_source->parent_ = set->GetSetSource();
        }
    private:
        const SetId id_;
        DisjointSet* parent_{nullptr};
    };

    template <typename Iterable>
    void MergeSets(Iterable sets) {
        DisjointSet* common_parent = nullptr;
        for (DisjointSet* set : sets) {
            if (common_parent == nullptr) {
                common_parent = set;
                continue;
            }
            set->Merge(common_parent);
            assert(set->GetId() == common_parent->GetId());
        }
    }
}