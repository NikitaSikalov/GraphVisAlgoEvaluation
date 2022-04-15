#pragma once

#include <ogr_components/matrix.h>
#include <ogr_components/structured_elements.h>

#include <vector>

namespace ogr::crawler {
    std::vector<EdgePtr> FindEdges(const Vertex& source, matrix::Grm& grm, size_t& edge_id_counter);
}
