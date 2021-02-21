#include <pybind11/pybind11.h>
#include <string>

namespace py = pybind11;

std::string get_version() {
  return "0.0.1";
}


PYBIND11_MODULE(geofun2, m) {
    m.doc() = "Geographic utilities";

    m.def("get_version", &get_version, "Get the library version");
}

