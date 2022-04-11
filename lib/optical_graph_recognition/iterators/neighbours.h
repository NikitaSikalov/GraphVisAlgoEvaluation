#pragma once

#include <utility>
#include <type_traits>

#include <ogr_components/matrix.h>
#include <utils/stack_vector.h>
#include <utils/matrix_utils.h>
#include <iterators/filters.h>

namespace ogr::iterator {
    namespace detail {
        template<class TStackVector>
        TStackVector PreFilterNeighbours(const TStackVector &candidates, const size_t max_rows, const size_t max_cols) {
            TStackVector result;
            for (const auto &candidate: candidates) {
                const int row = candidate.first;
                const int column = candidate.second;
                if (row < 0 || column < 0) {
                    continue;
                }
                if (row >= max_rows || column >= max_cols) {
                    continue;
                }

                result.PushBack({row, column});
            }

            return result;
        }
    }

    struct Neighbourhood8 {
        utils::StackVector<point::PointPtr, 8> operator()(const point::PointPtr &point, const matrix::Grm &grm) const {
            const int row = static_cast<int>(point->row);
            const int column = static_cast<int>(point->column);

            const utils::StackVector<std::pair<int, int>, 8> candidates{
                    {row - 1, column},
                    {row,     column - 1},
                    {row,     column + 1},
                    {row + 1, column},
                    {row - 1, column - 1},
                    {row + 1, column + 1},
                    {row + 1, column - 1},
                    {row - 1, column + 1}
            };

            const auto prefiltered_candidates = detail::PreFilterNeighbours(candidates, matrix::Rows(grm),
                                                                            matrix::Columns(grm));
            const auto points = utils::MapMatrixItems(prefiltered_candidates, grm);
            return filter::FilterEmptyPoints(points);
        }
    };

    struct Neighbourhood4 {
        utils::StackVector<point::PointPtr, 4> operator()(const point::PointPtr &point, const matrix::Grm &grm) const {
            const int row = static_cast<int>(point->row);
            const int column = static_cast<int>(point->column);

            const utils::StackVector<std::pair<int, int>, 4> candidates{
                    {row - 1, column},
                    {row + 1, column},
                    {row, column - 1},
                    {row, column + 1}
            };

            const auto prefiltered_candidates = detail::PreFilterNeighbours(candidates, matrix::Rows(grm),
                                                                            matrix::Columns(grm));

            const auto points = utils::MapMatrixItems(prefiltered_candidates, grm);
            return filter::FilterEmptyPoints(points);
        }
    };

    template <class TStackVector>
    std::enable_if_t<std::is_same_v<typename TStackVector::ItemType, point::PointPtr>, bool>
    ContainsPoint(const TStackVector& points, const point::PointPtr point) {
        for (const point::PointPtr p : points) {
            if (p.get() == point.get()) {
                return true;
            }
        }

        return false;
    }
}
