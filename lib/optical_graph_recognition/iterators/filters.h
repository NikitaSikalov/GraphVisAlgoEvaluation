#pragma once

#include <utils/stack_vector.h>
#include <ogr_components/point.h>

namespace ogr::iterator::filter {
    template<typename TStackVector>
    TStackVector FilterMarkedPoints(const TStackVector &points) {
        TStackVector result;
        for (const point::PointPtr& point : points) {
            if (utils::As<point::FilledPoint>(point.get())->IsMarked()) {
                continue;
            }
            result.PushBack(point);
        }

        return result;
    }

    template<typename TStackVector>
    TStackVector FilterEmptyPoints(const TStackVector &points) {
        TStackVector result;
        for (const point::PointPtr& point : points) {
            if (point->IsEmpty()) {
                continue;
            }
            result.PushBack(point);
        }

        return result;
    }

    template <typename TStackVector>
    TStackVector FilterVertexPoints(const TStackVector& points) {
        TStackVector result;
        for (const point::PointPtr& point : points) {
            if (utils::Is<point::VertexPoint>(point.get())) {
                continue;
            }
            result.PushBack(point);
        }

        return result;
    }
}