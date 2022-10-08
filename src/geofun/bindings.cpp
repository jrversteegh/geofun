#include <string>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <string>
#include <cstdlib>
#include <initializer_list>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Rhumb.hpp>
#include <GeographicLib/Constants.hpp>

#include <fmt/format.h>

#include "version.h"

namespace gl = GeographicLib;
namespace py = pybind11;
using namespace pybind11::literals;

static constexpr double pi = 3.14159265358979323846;
static constexpr double r2d = 180.0 / pi;
static constexpr double d2r = pi / 180.0;


double angle_mod(const double angle) {
  double result = std::fmod(angle, 360.0);
  return result < 0.0 ? result + 360.0 : result;
}


double angle_mod_signed(const double angle) {
  double result = std::fmod(angle, 360.0);
  return result < -180.0 ? result + 360.0 : result >= 180.0 ? result - 360.0 : result;
}


double angle_diff(const double angle1, const double angle2) {
  return angle_mod_signed(angle1 - angle2);
}


inline bool floats_equal(const double value1, const double value2)
{
  double abs1 = std::fabs(value1);
  double abs2 = std::fabs(value2);
  double absmax = std::max(abs1, abs2);
  double eps = 1E-13;
  // Get relative eps value except for values very close to zero
  if (absmax > 1E-7) {
    eps *= absmax;
  }
  return std::fabs(value1 - value2) < eps;
}


inline bool float_smaller(const double value1, const double value2)
{
  return value1 < value2 and not floats_equal(value1, value2);
}


inline bool scan_floats(std::vector<double>& values, const std::string input) {
  const char* s = input.c_str();
  int result = true;

  // Scan if the string only contains integral values
  int i = 0;
  char c;
  while ((c = s[i++]) != '\0') {
    if (!(isspace(c) || isdigit(c) || (c == '-'))) {
      result = false;
      break;
    }
  }

  // Now scan the string for floats
  while (*s != '\0') {
    char* e;
    double f = std::strtof(s, &e);
    if (e == s) {
      ++e;
    }
    else {
      values.push_back(f);
    }
    s = e;
  }
  return result;
}


std::string get_version() {
  return VERSION;
}


py::tuple rhumb_direct(const double latitude, const double longitude, const double azimuth, const double distance) {
  static const gl::Rhumb& rhumb = gl::Rhumb::WGS84();
  double out_latitude;
  double out_longitude;
  rhumb.Direct(latitude, longitude, azimuth, distance, out_latitude, out_longitude);
  return py::make_tuple(out_latitude, out_longitude, azimuth);
}


py::tuple rhumb_inverse(const double latitude1, const double longitude1, const double latitude2, const double longitude2) {
  static const gl::Rhumb& rhumb = gl::Rhumb::WGS84();
  double azimuth;
  double distance;
  rhumb.Inverse(latitude1, longitude1, latitude2, longitude2, distance, azimuth);
  return py::make_tuple(azimuth, distance, azimuth);
}


py::tuple geodesic_direct(const double latitude, const double longitude, const double azimuth, const double distance) {
  static const gl::Geodesic& geodesic = gl::Geodesic::WGS84();
  double out_latitude;
  double out_longitude;
  double out_azimuth;
  geodesic.Direct(latitude, longitude, azimuth, distance, out_latitude, out_longitude, out_azimuth);
  return py::make_tuple(out_latitude, out_longitude, out_azimuth);
}


py::tuple geodesic_inverse(const double latitude1, const double longitude1, const double latitude2, const double longitude2) {
  static const gl::Geodesic& geodesic = gl::Geodesic::WGS84();
  double azimuth1;
  double distance;
  double azimuth2;
  geodesic.Inverse(latitude1, longitude1, latitude2, longitude2, distance, azimuth1, azimuth2);
  return py::make_tuple(azimuth1, distance, azimuth2);
}


struct Point {
  Point() = default;
  Point(const Point&) = default;
  Point(Point&&) = default;
  Point(const double x, const double y): x_(x), y_(y) {}
  Point(const std::vector<double>& initializer) {
    if (initializer.size() != 2) {
      throw std::out_of_range("Initializer length isn't 2 in construction of point");
    }
    set_x(initializer[0]);
    set_y(initializer[1]);
  }
  Point& operator=(const Point&) = default;
  Point& operator=(Point&&) = default;

  double get_x() const {
    return x_;
  }

  double get_y() const {
    return y_;
  }

  Point& set_x(const double x) {
    x_ = x;
    return *this;
  }

  Point& set_y(const double y) {
    y_ = y;
    return *this;
  }

  bool operator==(const Point& point) const {
    return floats_equal(x_, point.x_) && floats_equal(y_, point.y_);
  }

  bool operator==(const std::vector<double>& other) const {
    if (other.size() != 2) {
      throw std::out_of_range(fmt::format("Can't compare Point to container of size {}", other.size()));
    }
    return floats_equal(get_x(), other[0]) && floats_equal(get_y(), other[1]);
  }

  Point& operator+=(const Point& point) {
    x_ += point.x_;
    y_ += point.y_;
    return *this;
  }

  Point& operator-=(const Point& point) {
    x_ -= point.x_;
    y_ -= point.y_;
    return *this;
  }

  Point operator+(const Point& point) const {
    Point result(*this);
    result += point;
    return result;
  }

  Point operator-(const Point& point) const {
    Point result(*this);
    result -= point;
    return result;
  }

  Point& operator*=(const double multiplier) {
    x_ *= multiplier;
    y_ *= multiplier;
    return *this;
  }

  Point operator*(const double multiplier) const {
    Point result(*this);
    result *= multiplier;
    return result;
  }

  Point operator-() const {
    return Point(-x_, -y_);
  }

  double get_item(int i) const {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return get_x();
      case 1: return get_y();
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Point", i));
    }
  }

  Point& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_x(value);
      case 1: return set_y(value);
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Point", i));
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

  std::string get_string() const {
    return fmt::format("{:.3f}, {:.3f}", x_, y_);
  }

  std::string get_representation() const {
    double i;
    double x_frac = modf(x_, &i);
    double y_frac = modf(y_, &i);
    std::string format = x_frac == 0.0 ? "Point({:.1f}" : "Point({:.15g}";
    format += y_frac == 0.0 ? ", {:.1f})" : ", {:.15g})";
    return fmt::format(fmt::runtime(format), x_, y_);
  }

private:
  friend struct Vector;
  double x_;
  double y_;
};


Point operator*(const double multiplier, const Point& point) {
  return point.operator*(multiplier);
}


struct Vector {
  Vector() = default;
  Vector(const Vector&) = default;
  Vector(Vector&&) = default;
  Vector(const double azimuth, const double length): azimuth_(), length_() {
    set_azimuth(azimuth);
    set_length(length);
  }
  Vector(const Point& point): azimuth_(), length_() {
    set_x_y(point.x_, point.y_);
  }
  Vector(const std::vector<double>& initializer) {
    if (initializer.size() != 2) {
      throw std::out_of_range("Initializer length isn't 2 in construction of vector");
    }
    set_azimuth(initializer[0]);
    set_length(initializer[1]);
  }
  Vector& operator=(const Vector&) = default;
  Vector& operator=(Vector&&) = default;

  double get_azimuth() const {
    return azimuth_;
  }

  double get_length() const {
    return length_;
  }

  Vector& set_azimuth(const double azimuth) {
    azimuth_ = angle_mod(azimuth);
    return *this;
  }

  Vector& set_length(const double length) {
    // Prefer a positive length
    if (length < 0) {
      set_azimuth(azimuth_ + 180.0);
      length_ = -length;
    }
    else {
      length_ = length;
    }
    return *this;
  }

  double get_x() const {
    return cos(d2r * azimuth_) * length_;
  }

  double get_y() const {
    return sin(d2r * azimuth_) * length_;
  }

  Vector& set_x(const double x) {
    set_x_y(x, get_y());
    return *this;
  }

  Vector& set_y(const double y) {
    set_x_y(get_x(), y);
    return *this;
  }

  Vector& set_x_y(const double x, const double y) {
    azimuth_ = r2d * atan2(y, x);
    azimuth_ = azimuth_ < 0 ? azimuth_ + 360.0 : azimuth_;
    length_ = hypot(x, y);
    return *this;
  }

  bool operator==(const Vector& vector) const {
    return floats_equal(azimuth_, vector.azimuth_) && floats_equal(length_, vector.length_);
  }

  bool operator==(const Point& point) const {
    return floats_equal(get_x(), point.get_x()) && floats_equal(get_y(), point.get_y());
  }

  bool operator==(const std::vector<double>& other) const {
    if (other.size() != 2) {
      throw std::out_of_range(fmt::format("Can't compare Vector to container of size {}", other.size()));
    }
    return floats_equal(get_azimuth(), other[0]) && floats_equal(get_length(), other[1]);
  }

  Vector& operator+=(const Vector& vector) {
    set_x_y(get_x() + vector.get_x(), get_y() + vector.get_y());
    return *this;
  }

  Vector& operator-=(const Vector& vector) {
    set_x_y(get_x() - vector.get_x(), get_y() - vector.get_y());
    return *this;
  }

  Vector& operator+=(const double angle) {
    return set_azimuth(azimuth_ + angle);
  }

  Vector& operator-=(const double angle) {
    return set_azimuth(azimuth_ - angle);
  }

  Vector& operator*=(const double multiplier) {
    return set_length(length_ * multiplier);
  }

  Vector operator+(const Vector& vector) const {
    Vector result(*this);
    result += vector;
    return result;
  }

  Vector operator-(const Vector& vector) const {
    Vector result(*this);
    result -= vector;
    return result;
  }

  Vector operator+(const double angle) const {
    Vector result(*this);
    result += angle;
    return result;
  }

  Vector operator-(const double angle) const {
    Vector result(*this);
    result -= angle;
    return result;
  }

  Vector operator*(const double multiplier) const {
    Vector result(*this);
    result *= multiplier;
    return result;
  }

  Vector operator-() const {
    return Vector(azimuth_ + 180.0, length_);
  }

  Vector norm() const {
    return Vector(azimuth_, 1.0);
  }

  double dot(const Vector& vector) const {
    return length_ * vector.length_ * cos(d2r * (vector.azimuth_ - azimuth_));
  }

  double cross(const Vector& vector) const {
    return length_ * vector.length_ * sin(d2r * (vector.azimuth_ - azimuth_));
  }

  Point point() const {
    return Point(get_x(), get_y());
  }

  double get_item(int i) const {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return get_azimuth();
      case 1: return get_length();
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Vector", i));
    }
  }

  Vector& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_azimuth(value);
      case 1: return set_length(value);
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Vector", i));
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

  std::string get_string() const {
    return fmt::format("{:.3f}, {:.3f}", azimuth_, length_);
  }

  std::string get_representation() const {
    double i;
    double x_frac = modf(azimuth_, &i);
    double y_frac = modf(length_, &i);
    std::string format = x_frac == 0.0 ? "Vector({:.1f}" : "Vector({:.15g}";
    format += y_frac == 0.0 ? ", {:.1f})" : ", {:.15g})";
    return fmt::format(fmt::runtime(format), azimuth_, length_);
  }

private:
  friend struct Position;
  double azimuth_;
  double length_;
};

bool operator==(const Point& point, const Vector& vector) {
  return vector.operator==(point);
}

Vector operator*(const double multiplier, const Vector& vector) {
  return vector.operator*(multiplier);
}

Vector operator+(const double angle, const Vector& vector) {
  return vector.operator+(angle);
}

Vector operator-(const double angle, const Vector& vector) {
  return -vector.operator+(angle);
}


struct Position {
  Position() = default;
  Position(const Position&) = default;
  Position(Position&&) = default;

  Position(const double latitude, const double longitude): latitude_(), longitude_() {
    set_latitude(latitude);
    set_longitude(longitude);
  }

  Position(const int latitude, const int longitude): latitude_(), longitude_() {
    set_latitude_seconds(latitude);
    set_longitude_seconds(longitude);
  }

  Position(const std::vector<double>& initializer) {
    if (initializer.size() != 2) {
      throw std::out_of_range("Initializer length isn't 2 in construction of Position");
    }
    set_latitude(initializer[0]);
    set_longitude(initializer[1]);
  }

  Position(const std::string& latitude, const std::string& longitude): latitude_(), longitude_() {
    std::vector<double> values;
    bool is_integral = scan_floats(values, latitude);
    is_integral &= scan_floats(values, longitude);
    int val_count = static_cast<int>(values.size());
    int i_count = val_count / 2;
    if ((val_count == 0) || (val_count % 2) || (i_count > 3)) {
      throw std::invalid_argument(fmt::format("Invalid argument count: {} for Position", val_count));
    }
    size_t pos_N = latitude.find('N');
    size_t pos_S = latitude.find('S');
    bool is_south = pos_S != std::string::npos;
    bool single = longitude == "";
    size_t pos_E = single ? latitude.find('E') : longitude.find('E');
    size_t pos_W = single ? latitude.find('W') : longitude.find('W');
    bool is_west = pos_W != std::string::npos;
    size_t pos_la = std::min(pos_N, pos_S);
    size_t pos_lo = std::min(pos_E, pos_W);
    bool reversed = single ? pos_lo < pos_la : false;

    int offset = reversed ? i_count : 0;
    double value = values[offset];
    double lat = is_south ? -value : value;
    double mult = std::signbit(lat) ? (-1.0 / 60.0) : (1.0 / 60.0);
    int i = 1;
    while (i < i_count) {
      value = values[offset + i++];
      lat += value * mult;
      mult /= 60.0;
    }

    offset = reversed ? 0 : i_count;
    value = values[offset];
    double lon = is_west ? -value : value;
    mult = std::signbit(lon) ? (-1.0 / 60.0) : (1.0 / 60.0);
    i = 1;
    while (i < i_count) {
      value = values[offset + i++];
      lon += value * mult;
      mult /= 60.0;
    }

    if (is_integral) {
      set_latitude_seconds(static_cast<int>(lat));
      set_longitude_seconds(static_cast<int>(lon));
    }
    else {
      set_latitude(lat);
      set_longitude(lon);
    }
  }

  Position& operator=(const Position&) = default;
  Position& operator=(Position&&) = default;

  double get_latitude() const {
    return latitude_;
  }

  double get_longitude() const {
    return longitude_;
  }

  Position& set_latitude(const double latitude) {
    latitude_ = angle_mod_signed(latitude);
    latitude_ = latitude_ > 90.0 ? 180.0 - latitude_ : latitude_ < -90.0 ? -180.0 - latitude_ : latitude_;
    return *this;
  }

  Position& set_latitude_seconds(const int latitude) {
    return set_latitude(latitude / 3600.0);
  }

  Position& set_longitude(const double longitude) {
    longitude_ = angle_mod_signed(longitude);
    return *this;
  }

  Position& set_longitude_seconds(const int longitude) {
    return set_longitude(longitude / 3600.0);
  }

  bool operator==(const Position& position) const {
    return floats_equal(latitude_, position.latitude_) && floats_equal(longitude_, position.longitude_);
  }

  bool operator==(const std::vector<double>& other) const {
    if (other.size() != 2) {
      throw std::out_of_range(fmt::format("Can't compare Position to container of size {}", other.size()));
    }
    return floats_equal(get_latitude(), other[0]) && floats_equal(get_longitude(), other[1]);
  }

  Position& operator+=(const Vector& vector) {
    static const gl::Rhumb& rhumb = gl::Rhumb::WGS84();
    double latitude;
    double longitude;
    rhumb.Direct(latitude_, longitude_, vector.azimuth_, vector.length_, latitude, longitude);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator-=(const Vector& vector) {
    static const gl::Rhumb& rhumb = gl::Rhumb::WGS84();
    double latitude;
    double longitude;
    rhumb.Direct(latitude_, longitude_, vector.azimuth_, -vector.length_, latitude, longitude);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator*=(const Vector& vector) {
    static const gl::Geodesic& geodesic = gl::Geodesic::WGS84();
    double latitude;
    double longitude;
    double azimuth;
    geodesic.Direct(latitude_, longitude_, vector.azimuth_, vector.length_, latitude, longitude, azimuth);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator/=(const Vector& vector) {
    static const gl::Geodesic& geodesic = gl::Geodesic::WGS84();
    double latitude;
    double longitude;
    double azimuth;
    geodesic.Direct(latitude_, longitude_, vector.azimuth_, -vector.length_, latitude, longitude, azimuth);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position operator+(const Vector& vector) const {
    Position result(*this);
    result += vector;
    return result;
  }

  Position operator-(const Vector& vector) const {
    Position result(*this);
    result -= vector;
    return result;
  }

  Position operator*(const Vector& vector) const {
    Position result(*this);
    result *= vector;
    return result;
  }

  Position operator/(const Vector& vector) const {
    Position result(*this);
    result /= vector;
    return result;
  }

  double get_item(int i) const {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return get_latitude();
      case 1: return get_longitude();
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Position", i));
    }
  }

  Position& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_latitude(value);
      case 1: return set_longitude(value);
      default: throw std::out_of_range(fmt::format("Index {} is out of range for Position", i));
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

  std::string get_string() const {
    return fmt::format("{:.8f}, {:.8f}", latitude_, longitude_);
  }

  std::string get_representation() const {
    double i;
    double x_frac = modf(latitude_, &i);
    double y_frac = modf(longitude_, &i);
    std::string format = x_frac == 0.0 ? "Position({:.1f}" : "Position({:.15g}";
    format += y_frac == 0.0 ? ", {:.1f})" : ", {:.15g})";
    return fmt::format(fmt::runtime(format), latitude_, longitude_);
  }

private:
  double latitude_;
  double longitude_;
};


Vector operator-(const Position& position2, const Position& position1) {
  static const gl::Rhumb& rhumb = gl::Rhumb::WGS84();
  double azimuth;
  double distance;
  rhumb.Inverse(
      position1.get_latitude(),
      position1.get_longitude(),
      position2.get_latitude(),
      position2.get_longitude(),
      distance,
      azimuth
  );
  return Vector(azimuth, distance);
}


Vector operator/(const Position& position2, const Position& position1) {
  static const gl::Geodesic& geodesic = gl::Geodesic::WGS84();
  double azimuth1;
  double azimuth2;
  double distance;
  geodesic.Inverse(
      position1.get_latitude(),
      position1.get_longitude(),
      position2.get_latitude(),
      position2.get_longitude(),
      distance,
      azimuth1,
      azimuth2
  );
  return Vector(azimuth1, distance);
}


PYBIND11_MODULE(geofun, m) {
  m.doc() = "Geographic utilities: orthodrome/loxodrome, geodesic/rhumb line evaluation.";

  m.def("get_version", &get_version,
      "Get the library version");

  // GeographicLib wrappers
  m.def("rhumb_direct", &rhumb_direct, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
      "Get position and final azimuth after moving distance from starting position at fixed azimuth/along rhumb line");
  m.def("rhumb_inverse", &rhumb_inverse, "latitude1"_a, "longitude1"_a, "latitude2"_a, "longitude2"_a,
      "Get rhumb line azimuth, distance and final azimuth between positions");
  m.def("geodesic_direct", &geodesic_direct, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
      "Get position and final azimuth after moving distance along great circle with starting azimuth");
  m.def("geodesic_inverse", &geodesic_inverse, "latitude1"_a, "longitude1"_a, "latitude2"_a, "longitude2"_a,
      "Get starting azimuth, distance and ending azimuth of great circle between positions");

  // Angle arithmetic
  m.def("angle_mod", py::vectorize(angle_mod),
      "Return angle bound to [0.0, 360.0>");
  m.def("angle_mod_signed", py::vectorize(angle_mod_signed),
      "Return angle bound to [-180.0, 180.0>");
  m.def("angle_diff", py::vectorize(angle_diff),
      "Signed difference between to angles");

  // Primitives
  py::class_<Point>(m, "Point")
    .def(py::init<>(),
        "Construct new point.")
    .def(py::init<Point&>(),
        "Copy construct point.")
    .def(py::init<const double, const double>(), "x"_a, "y"_a,
        "Construct point from coordinate pair x, y.")
    .def(py::init<const std::vector<double>&>(),
        "Construct point from initializer list.")
    .def("__getitem__", &Point::get_item)
    .def("__setitem__", &Point::set_item)
    .def("__len__", &Point::get_len)
    .def("__str__", &Point::get_string)
    .def("__repr__", &Point::get_representation)
    .def("__copy__", [](const Point& self) { return Point(self); })
    .def("__deepcopy__", [](const Point& self, py::dict) { return Point(self); }, "memo"_a)
    .def("copy", [](const Point& self) { return Point(self); },
        "Return a copy of this point.")
    .def_property("x", &Point::get_x, &Point::set_x,
        "X coordinate of point.")
    .def_property("y", &Point::get_y, &Point::set_y,
        "Y coordinate of point.")
    .def(py::self == py::self)
    .def(py::self == Vector())
    .def(py::self == std::vector<double>())
    .def(py::self += py::self)
    .def(py::self + py::self)
    .def(py::self -= py::self)
    .def(py::self - py::self)
    .def(py::self *= double())
    .def(double() * py::self)
    .def(py::self * double())
    .def(-py::self)
    .def(py::pickle(
      [](const Point& p) {
        return py::make_tuple(p.get_x(), p.get_y());
      },
      [](const py::tuple t) {
        if (t.size() != 2)
          throw std::runtime_error("Point pickle: Invalid state!");
        Point p(t[0].cast<double>(), t[1].cast<double>());
        return p;
      }
    ))
    ;

  py::class_<Vector>(m, "Vector")
    .def(py::init<>(),
        "Construct new vector.")
    .def(py::init<Vector&>(),
        "Copy construct vector.")
    .def(py::init<const double, const double>(), "azimuth"_a, "length"_a,
        "Construct vector from pair of azimuth and length.")
    .def(py::init<const std::vector<double>&>(),
        "Construct vector from initializer list.")
    .def("__getitem__", &Vector::get_item)
    .def("__setitem__", &Vector::set_item)
    .def("__len__", &Vector::get_len)
    .def("__str__", &Vector::get_string)
    .def("__repr__", &Vector::get_representation)
    .def("__copy__", [](const Vector& self) { return Vector(self); })
    .def("__deepcopy__", [](const Vector& self, py::dict) { return Vector(self); }, "memo"_a)
    .def("copy", [](const Vector& self) { return Vector(self); },
        "Return a copy of this vector.")
    .def("norm", &Vector::norm,
        "Return a copy of this vector with a lenght of 1.")
    .def("dot", &Vector::dot, "other"_a,
        "Return the dot product of this vector with \"other\".")
    .def("cross", &Vector::cross, "other"_a,
        "Return the cross product of this vector with \"other\".")
    .def("point", &Vector::point,
        "Return copy of vector as Point with x, y coordinates.")
    .def_property("azimuth", &Vector::get_azimuth, &Vector::set_azimuth,
        "Azimuth of vector")
    .def_property("length", &Vector::get_length, &Vector::set_length,
        "Length of vector")
    .def_property("x", &Vector::get_x, &Vector::set_x,
        "X component of vector")
    .def_property("y", &Vector::get_y, &Vector::set_y,
        "Y component of vector")
    .def(py::self == py::self)
    .def(py::self == Point())
    .def(py::self == std::vector<double>())
    .def(py::self += py::self)
    .def(py::self + py::self)
    .def(py::self -= py::self)
    .def(py::self - py::self)
    .def(py::self *= double())
    .def(double() * py::self)
    .def(py::self * double())
    .def(py::self += double())
    .def(double() + py::self)
    .def(py::self + double())
    .def(py::self -= double())
    .def(double() - py::self)
    .def(py::self - double())
    .def(-py::self)
    .def(py::pickle(
      [](const Vector& v) {
        return py::make_tuple(v.get_azimuth(), v.get_length());
      },
      [](const py::tuple t) {
        if (t.size() != 2)
          throw std::runtime_error("Vector pickle: Invalid state!");
        Vector v(t[0].cast<double>(), t[1].cast<double>());
        return v;
      }
    ))
    ;

  py::class_<Position>(m, "Position")
    .def(py::init<>(),
        "Construct new position.")
    .def(py::init<Position&>(),
        "Copy construct position.")
    .def(py::init<const double, const double>(), "latitude"_a, "longitude"_a,
        "Construct position from decimal degrees of angle.")
    .def(py::init<const int, const int>(), "lat_seconds"_a, "lon_seconds"_a,
        "Construct position from seconds of angle.")
    .def(py::init<const std::vector<double>&>(),
        "Construct position from initializer list.")
    .def(py::init<const std::string&, const std::string&>(), "lat_string"_a, "lon_string"_a = "",
        "Construct position from pair of strings")
    .def("__getitem__", &Position::get_item)
    .def("__setitem__", &Position::set_item)
    .def("__len__", &Position::get_len)
    .def("__str__", &Position::get_string)
    .def("__repr__", &Position::get_representation)
    .def("__copy__", [](const Position& self) { return Position(self); })
    .def("__deepcopy__", [](const Position& self, py::dict) { return Position(self); }, "memo"_a)
    .def("copy", [](const Position& self) { return Position(self); })
    .def("__len__", &Position::get_len)
    .def_property("latitude", &Position::get_latitude, &Position::set_latitude,
        "Latitude of position")
    .def_property("longitude", &Position::get_longitude, &Position::set_longitude,
        "Longitude of position")
    .def(py::self == py::self)
    .def(py::self == std::vector<double>())
    .def(py::self - py::self)
    .def(py::self / py::self)
    .def(py::self += Vector())
    .def(py::self + Vector())
    .def(py::self -= Vector())
    .def(py::self - Vector())
    .def(py::self *= Vector())
    .def(py::self * Vector())
    .def(py::self /= Vector())
    .def(py::self / Vector())
    .def(py::pickle(
      [](const Position& p) {
        return py::make_tuple(p.get_latitude(), p.get_longitude());
      },
      [](const py::tuple t) {
        if (t.size() != 2)
          throw std::runtime_error("Position pickle: Invalid state!");
        Position p(t[0].cast<double>(), t[1].cast<double>());
        return p;
      }
    ))
    ;
}
