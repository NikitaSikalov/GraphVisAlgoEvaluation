#pragma once

#include <ogr_components/matrix.h>

#include <opencv2/opencv.hpp>

#include <sstream>
#include <memory>

namespace ogr::crawler {
    struct IStep;
}

namespace ogr::debug {
    cv::Mat DebugDumpGrm2CvMat(const matrix::Grm& grm);

    std::string DebugDump(const point::Point& point);
    std::string DebugDump(const Vertex& vertex);
    std::string DebugDump(const crawler::IStep& step);
}