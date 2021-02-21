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
  return py::make_tuple(out_latitude, out_longitude, azimuth);
}


py::tuple rhumb_inverse(const double latitude1, const double longitude1, const double latitude2, const double longitude2) {
  static const Rhumb& rhumb = Rhumb::WGS84();
  double azimuth;
  double distance;
  rhumb.Inverse(latitude1, longitude1, latitude2, longitude2, distance, azimuth);
  return py::make_tuple(azimuth, distance, azimuth);
}


py::tuple geodesic_direct(const double latitude, const double longitude, const double azimuth, const double distance) {
  static const Geodesic& geodesic = Geodesic::WGS84();
  double out_latitude;
  double out_longitude;
  double out_azimuth;
  geodesic.Direct(latitude, longitude, azimuth, distance, out_latitude, out_longitude, out_azimuth);
  return py::make_tuple(out_latitude, out_longitude, out_azimuth);
}


py::tuple geodesic_inverse(const double latitude1, const double longitude1, const double latitude2, const double longitude2) {
  static const Geodesic& geodesic = Geodesic::WGS84();
  double azimuth1;
  double distance;
  double azimuth2;
  geodesic.Inverse(latitude1, longitude1, latitude2, longitude2, distance, azimuth1, azimuth2);
  return py::make_tuple(azimuth1, distance, azimuth2);
}


PYBIND11_MODULE(geofun2, m) {
    m.doc() = "Geographic utilities: orthodrome/loxodrome, geodesic/rhumb line evaluation.";

    m.def("get_version", &get_version,
        "Get the library version");
    m.def("rhumb_direct", &rhumb_direct, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
        "Get position and final azimuth after moving distance from starting position at fixed azimuth/along rhumb line");
    m.def("rhumb_inverse", &rhumb_inverse, "latitude1"_a, "longitude1"_a, "latitude2"_a, "longitude2"_a,
        "Get rhumb line azimuth, distance and final azimuth between positions");
    m.def("geodesic_direct", &geodesic_direct, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
        "Get position and final azimuth after moving distance along great circle with starting azimuth");
    m.def("geodesic_inverse", &geodesic_inverse, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
        "Get starting azimuth, distance and ending azimuth of great circle between positions");
}
