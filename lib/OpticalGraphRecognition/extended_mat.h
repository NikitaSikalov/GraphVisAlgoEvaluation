#pragma once

#include "stack_vector.h"

#include <opencv2/opencv.hpp>

#define NOT(type) GetNotPixelTypeFilter(type)

// Optical graph recognition namespace
namespace ogr {
    using namespace utils;

    enum class PixelType {
        EMPTY,
        UNKNOWN,
        VERTEX,
        EDGE,
        CROSSING,

        MAX
    };

    auto GetNotPixelTypeFilter(PixelType type);

    class OgrMat : public cv::Mat {
    public:
        using cv::Mat::Mat;

        PixelType GetPixelType(const cv::Point& point) const;
        StackVector<cv::Point, 8> Get8Neighbourhood(const cv::Point&) const;
        StackVector<cv::Point, 4> Get4Neighbourhood(const cv::Point&) const;
        StackVector<cv::Point, 24> Get24Neighbourhood(const cv::Point&) const;

        template<class Iterable, class ...Filters>
        Iterable FilterPoints(const Iterable& points, const Filters&... filters) const {
            Iterable result;

            for (const cv::Point& point : points) {
                bool addPoint = true;
                for (auto& filter : {filters...}) {
                    if (!filter(*this, point)) {
                        addPoint = false;
                        break;
                    }
                }

                if (addPoint) {
                    result.PushBack(point);
                }
            }

            return result;
        }
    };

}