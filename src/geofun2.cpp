#include <pybind11/pybind11.h>
#include <string>

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Rhumb.hpp>
#include <GeographicLib/Constants.hpp>

#include "version.h"

using namespace GeographicLib;
namespace py = pybind11;
using namespace pybind11::literals;


std::string get_version() {
  return VERSION;
}


py::tuple rhumb_direct(const double latitude, const double longitude, const double azimuth, const double distance) {
  static const Rhumb& rhumb = Rhumb::WGS84();
  double out_latitude;
  double out_longitude;
  rhumb.Direct(latitude, longitude, azimuth, distance, out_latitude, out_longitude);
  return py::make_tuple(out_latitude, out_longitude);
}


PYBIND11_MODULE(geofun2, m) {
    m.doc() = "Geographic utilities";

    m.def("get_version", &get_version, "Get the library version");
    m.def("rhumb_direct", &rhumb_direct, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a, "Get position after moving distance at fixed azimuth");
}

