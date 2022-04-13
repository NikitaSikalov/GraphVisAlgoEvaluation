#pragma once

#include <ogr_components/matrix.h>
#include <ogr_components/structured_elements.h>

namespace ogr::crawler {
    matrix::Grm FindEdges(const Vertex& source, matrix::Grm& sample);
}
