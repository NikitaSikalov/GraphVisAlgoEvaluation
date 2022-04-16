#include "edges_detector.h"

#include <crawler/step_tree_node.h>
#include <crawler/edge_crawler.h>
#include <utils/debug.h>

#include <plog/Log.h>

#include <queue>

namespace ogr::crawler {
    namespace {
        std::vector<StepPtr> FilterSteps(const std::vector<StepPtr>& steps) {
            std::vector<StepPtr> result;
            for (const auto& step : steps) {
                if (!step->IsExhausted() && !step->IsPort()) {
                    continue;
                }
                result.push_back(step);
            }

            return result;
        }
    }

    std::vector<EdgePtr> FindEdges(const Vertex& source, matrix::Grm& grm, size_t& edge_id_counter) {
        LOG_DEBUG << "Try to find edges from vertex: " << debug::DebugDump(source);

        // Configurable parameters
        constexpr size_t kSubPathStepsSize = 7;
        constexpr size_t kStepSize = 10;
        constexpr double kAngleDiffThreshold = 25.0;

        using StepTreeNodeImpl = StepTreeNode<kSubPathStepsSize>;
        using EdgeCrawlerImpl = EdgeCrawler<kStepSize, kSubPathStepsSize>;

        std::vector<EdgePtr> edges;

        utils::StackVector<point::FilledPointPtr, 64> port_points;
        for (auto port_point : source.port_points) {
            port_points.PushBack(port_point.lock());
        }

        std::priority_queue<EdgeCrawlerPtr, std::vector<EdgeCrawlerPtr>, crawler::Comparator> crawlers;
        StepTreeNodePtr paths_tree = StepTreeNodeImpl::MakeRoot();
        auto initial_steps = MakeSteps<kStepSize>(port_points, grm);


        for (StepPtr step : initial_steps) {
            // Step consists only of 1 port point
            if (!step->IsExhausted()) {
                continue;
            }

            LOG_DEBUG << "Add crawler with initial step: " << debug::DebugDump(*step);

            StepTreeNodePtr next_path_node = paths_tree->MakeChild(step);
            crawlers.push(std::make_shared<EdgeCrawlerImpl>(grm, next_path_node));
        }

        while (!crawlers.empty()) {
            debug::DebugDump(grm, false, source.id);

            EdgeCrawlerPtr crawler = crawlers.top();
            crawlers.pop();

            LOG_DEBUG << "Run crawler: " << debug::DebugDump(*crawler);

            // Prepare next steps
            auto steps = FilterSteps(crawler->NextSteps());
            if (steps.empty()) {
                LOG_DEBUG << "Next steps empty, skip crawler: " << debug::DebugDump(*crawler);
                continue;
            }

            // Add crawlers for other steps
            for (StepPtr step : steps) {
                auto next_crawler = std::make_shared<EdgeCrawlerImpl>(dynamic_cast<EdgeCrawlerImpl&>(*crawler));
                next_crawler->Commit(step);

                if (next_crawler->IsComplete()) {
                    LOG_DEBUG << "Materialize edge for crawler: " << debug::DebugDump(*next_crawler);
                    EdgePtr new_edge = std::move(*next_crawler).Materialize(source.id, edge_id_counter++, grm);
                    edges.push_back(new_edge);
                    continue;
                }

                if (next_crawler->CheckEdge(kAngleDiffThreshold)) {
                    LOG_DEBUG << "Push crawler to queue: " << debug::DebugDump(*next_crawler);
                    crawlers.push(next_crawler);
                    continue;
                }

                LOG_DEBUG << "Skip crawler: " << debug::DebugDump(*next_crawler);
            }
        }

        return edges;
    }
}