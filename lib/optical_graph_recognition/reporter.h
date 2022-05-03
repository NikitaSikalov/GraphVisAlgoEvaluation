#pragma once

#include <optical_graph_recognition.h>

namespace ogr {
    using OgrAlgo = OpticalGraphRecognition;

    void MakeReport(OgrAlgo& baseline, std::vector<OgrAlgo> algos);
}
