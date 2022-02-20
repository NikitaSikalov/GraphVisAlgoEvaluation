#pragma once

#include "stack_vector.h"
#include "inc.h"

#define NOT(type) GetNotPixelTypeFilter(type)

// Optical graph recognition namespace
namespace NOgr {
    using namespace NUtils;

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

        static const cv::Vec3b kColorsPalette[];

        PixelType GetPixelType(const cv::Point& point) const;
        void SetPixelType(const cv::Point& point, PixelType);
        StackVector<cv::Point, 8> Get8Neighbourhood(const cv::Point&) const;
        StackVector<cv::Point, 4> Get4Neighbourhood(const cv::Point&) const;
        cv::Mat GetColoredImage() const;

        template<class Iterable, class ...Filters>
        Iterable FilterPoints(const Iterable& points, const Filters&... filters) const {
            Iterable result;

            for (const cv::Point& point : points) {
                if (point.x < 0 || point.x >= cols) {
                    continue;
                }

                if (point.y < 0 || point.y >= rows) {
                    continue;
                }

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

        static OgrMat MakeOgrMatFromBinary(const cv::Mat& binary_image);
    };
}