#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "grid_shuffler_alg.hpp"

PYBIND11_MODULE(grid_shuffler, m) {
    m.doc() = "Grid shuffler algorithm for Python";

    pybind11::class_<GridShuffler>(m, "GridShuffler")
        .def(pybind11::init<const Grid&>())
        .def("shuffle", &GridShuffler::shuffle)
        .def("get_shuffled_grid", &GridShuffler::getShuffledGrid)
        .def("validate_result", &GridShuffler::validateResult);
}