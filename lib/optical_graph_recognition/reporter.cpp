#include "reporter.h"

#include <tabulate/table.hpp>

#include <algorithm>
#include <iostream>

namespace ogr {
    void MakeReport(OgrAlgo& baseline, std::vector<OgrAlgo> algos) {
        using namespace tabulate;
        using Row_t = Table::Row_t;

        Table results;
        results.add_row({"Aesthetics metrics of bundling graph evaluation"}).format()
                .width(100).font_align(FontAlign::center);

        results[0].format()
                .font_color(Color::green)
                .font_style({FontStyle::bold})
                .font_align(FontAlign::center);

        std::vector<Row_t::value_type> general_algo_infos;
        general_algo_infos.emplace_back(baseline.GetGeneralData("Baseline algo"));
        for (size_t i = 0; i < algos.size(); ++i) {
            const std::string title = std::string{"Algo "} + std::to_string(i);
            general_algo_infos.emplace_back(algos[i].GetExtendedGeneralData(title, baseline));
        }

        results.add_row({"Algorithms visualization comparison"});
        results[1].format().hide_border_bottom().font_color(Color::magenta).font_style({FontStyle::italic});

        Table subtable;
        subtable.format().hide_border();
        subtable.add_row(general_algo_infos);
        results.add_row(Row_t{subtable});
        results[2].format().hide_border_top();

        for (size_t i = 0; i < algos.size(); ++i) {
            std::stringstream ss;
            ss << "Edges info for algo " << i;
            results.add_row({ss.str()});
            results[3 + i].format().hide_border_bottom().font_color(Color::cyan).font_style({FontStyle::italic});
            results.add_row(Row_t{algos[i].GetEdgesInfo(ss.str())});
            results[3 + i + 1].format().hide_border_top();
        }

        std::cout << results << std::endl;
    }
}
