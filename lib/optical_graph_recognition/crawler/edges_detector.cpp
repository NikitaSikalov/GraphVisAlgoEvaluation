#include "edges_detector.h"

#include <crawler/step_tree_node.h>
#include <crawler/edge_crawler.h>
#include <utils/debug.h>

#include <plog/Log.h>

namespace ogr::crawler {
    matrix::Grm FindEdges(const Vertex& source, matrix::Grm& work_grm) {
        LOG_DEBUG << "Try to find edges from vertex";
        LOG_DEBUG << debug::DebugDump(source);

        constexpr size_t kSubPathStepsSize = 10;
        constexpr size_t kStepSize = 10;
        using StepTreeNodeImpl = StepTreeNode<kSubPathStepsSize>;
        using EdgeCrawlerImpl = EdgeCrawler<kStepSize, kSubPathStepsSize>;
        EdgeId  edge_id_counter = 0;

        // TODO: Think about correct copying grm object with other components (vertexes, edges, etc..)
        // matrix::Grm work_grm = matrix::CopyFromSample(sample);

        utils::StackVector<point::FilledPointPtr, 64> port_points;
        for (auto port_point : source.port_points) {
            port_points.PushBack(port_point.lock());
        }

        std::vector<EdgeCrawlerPtr> crawlers;
        StepTreeNodePtr paths_tree = StepTreeNodeImpl::MakeRoot();
        auto initial_steps = MakeSteps<kStepSize>(port_points, work_grm);


        for (StepPtr step : initial_steps) {
            // Step consists only of 1 port point
            if (!step->IsExhausted()) {
                continue;
            }

            LOG_DEBUG << "Add crawler with initial step";
            LOG_DEBUG << debug::DebugDump(*step);

            StepTreeNodePtr next_path_node = paths_tree->MakeChild(step);
            crawlers.push_back(std::make_shared<EdgeCrawlerImpl>(work_grm, next_path_node));
        }

        while (!crawlers.empty()) {
            EdgeCrawlerPtr crawler = crawlers.back();
            crawlers.pop_back();

            LOG_DEBUG << "Run crawler";
            LOG_DEBUG << debug::DebugDump(*crawler);

            while (true) {
                LOG_DEBUG << "Crawler iteration";
                LOG_DEBUG << debug::DebugDump(*crawler);

                // Prepare next steps
                auto steps = crawler->NextSteps();

                // Select one step of available
                auto crawler_next_step = steps.back();
                steps.pop_back();

                LOG_DEBUG << "Crawler next step";
                LOG_DEBUG << debug::DebugDump(*crawler_next_step);

                // Add crawlers for other steps
                for (StepPtr step : steps) {
                    auto next_crawler = std::make_shared<EdgeCrawlerImpl>(dynamic_cast<EdgeCrawlerImpl&>(*crawler));
                    next_crawler->Commit(step);

                    if (next_crawler->CheckEdge()) {
                        LOG_DEBUG << "Add another crawler with step";
                        LOG_DEBUG << debug::DebugDump(*step);

                        crawlers.push_back(next_crawler);
                    }
                }

                crawler->Commit(crawler_next_step);
                if (crawler->IsComplete()) {
                    std::move(*crawler).Materialize(edge_id_counter++);
                    break;
                }

                if (!crawler->CheckEdge()) {
                    break;
                }
            }
        }

        return work_grm;
    }
}