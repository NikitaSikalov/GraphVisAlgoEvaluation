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
    auto FilterEmptyPoints(const TStackVector &points) {
        TStackVector result;
        for (const point::PointPtr& point : points) {
            if (point->IsEmpty()) {
                continue;
            }
            result.PushBack(point);
        }

        return result;
    }
}