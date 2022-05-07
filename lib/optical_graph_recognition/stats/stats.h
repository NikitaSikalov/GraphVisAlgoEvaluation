#pragma once

#include <vector>

namespace ogr::stats {
    struct Stats {
        double mean;
        double var;
    };

    Stats CalculateStats(const std::vector<double>& sample);
}
