#include "edges_detector.h"

#include <crawler/step_tree_node.h>
#include <crawler/edge_crawler.h>

namespace ogr::crawler {
    matrix::Grm FindEdges(const Vertex& source, const matrix::Grm& sample) {
        constexpr size_t kSubPathStepsSize = 10;
        constexpr size_t kStepSize = 10;
        using PathNode = StepTreeNode<kSubPathStepsSize>;
        using PathNodePtr = std::shared_ptr<PathNode>;
        using TEdgeCrawler = EdgeCrawler<kStepSize, kSubPathStepsSize>;
        using EdgeCrawlerPtr = std::shared_ptr<TEdgeCrawler>;
        EdgeId  edge_id_counter = 0;

        matrix::Grm work_grm = matrix::CopyFromSample(sample);

        utils::StackVector<point::FilledPointPtr, 64> port_points;
        for (auto port_point : source.port_points) {
            port_points.PushBack(port_point.lock());
        }

        std::vector<EdgeCrawlerPtr> crawlers;
        PathNodePtr paths_root = std::make_shared<PathNode>();
        auto initial_steps = MakeSteps<kStepSize>(port_points, work_grm);


        for (StepPtr step : initial_steps) {
            // Step consists only of 1 port point
            if (step->Size() == 1) {
                continue;
            }

            PathNodePtr next_path_node = std::make_shared<PathNode>(step, paths_root);
            paths_root->AddChild(next_path_node);
            crawlers.push_back(std::make_shared<TEdgeCrawler>(work_grm, next_path_node));
        }

        while (!crawlers.empty()) {
            EdgeCrawlerPtr crawler = crawlers.back();
            crawlers.pop_back();

            while (true) {
                // Prepare next steps
                auto steps = crawler->NextSteps();

                // Select one step of available
                auto crawler_next_step = steps.back();
                steps.pop_back();

                // Add crawlers for other steps
                for (StepPtr step : steps) {
                    auto next_crawler = std::make_shared<TEdgeCrawler>(*crawler);
                    next_crawler->Commit(step);
                    if (next_crawler->CheckEdge()) {
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