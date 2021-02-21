#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(geofun2, m) {
    m.doc() = "Geographic utilities";

    m.def("add", &add, "A function which adds two numbers");
}

