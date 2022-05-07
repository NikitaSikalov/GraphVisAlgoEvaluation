#include "stats.h"

namespace ogr::stats {
    Stats CalculateStats(const std::vector<double>& sample) {
        Stats stats;

        double mean_accumulator = 0;
        for (const auto& x : sample) {
            mean_accumulator += x;
        }
        stats.mean = mean_accumulator / sample.size();

        double var_accumulator = 0;
        for (const auto& x : sample) {
            var_accumulator += ((x - stats.mean) * (x - stats.mean));
        }
        stats.var = sqrt(var_accumulator / sample.size());

        return stats;
    }
}
