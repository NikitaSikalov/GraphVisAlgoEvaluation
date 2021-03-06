#pragma once

#include <ogr_components/matrix.h>

#include <opencv2/opencv.hpp>

#include <sstream>
#include <memory>
#include <string>
#include <optional>
#include <tuple>

namespace ogr::crawler {
    struct IStep;
    struct IStepTreeNode;
    struct IEdgeCrawler;
}

namespace ogr::debug {
    extern std::string DevDirPath;
    extern bool DumpIntermediateResults;

    void DebugDump(const matrix::Grm& grm, std::optional<VertexId> vertex_filter = std::nullopt);
    std::string DebugDump(const point::Point&);
    std::string DebugDump(const Vertex&);
    std::string DebugDump(const crawler::IStep&);
    std::string DebugDump(const crawler::IStepTreeNode&);
    std::string DebugDump(const crawler::IEdgeCrawler&);

    template <typename... TupleArgs>
    inline std::string DebugDump(const std::tuple<TupleArgs...>& data) {
        std::stringstream ss;
        std::apply([&ss](auto... v){
            ss << "(";
            (..., (ss << " " << v));
            ss << " )";
        }, data);

        return ss.str();
    }
}