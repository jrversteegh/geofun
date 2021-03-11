#include <string>
#include <cmath>
#include <stdexcept>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>

#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Rhumb.hpp>
#include <GeographicLib/Constants.hpp>

#include "version.h"

using namespace GeographicLib;
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
  if (absmax > 1E-6) {
    eps *= absmax;
  }
  return std::fabs(value1 - value2) < eps;
}


inline bool float_smaller(const double value1, const double value2)
{
  return value1 < value2 and not floats_equal(value1, value2);
}



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


struct Point {

  Point() = default;
  Point(const Point&) = default;
  Point(Point&&) = default;
  Point(const double x, const double y): x_(x), y_(y) {}
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
      default: throw std::out_of_range("Index is out of range for Point");
    }
  }

  Point& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_x(value);
      case 1: return set_y(value);
      default: throw std::out_of_range("Index is out of range for Point");
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

private:
  friend class Vector;
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
  Vector(const double azimuth, const double length): azimuth_(angle_mod(azimuth)), length_(length) {}
  Vector(const Point& point): azimuth_(), length_() {
    set_x_y(point.x_, point.y_);
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
    length_ = length;
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


  Vector& operator+=(const Vector& vector) {
    set_x_y(get_x() + vector.get_x(), get_y() + vector.get_y());
    return *this;
  }

  Vector& operator-=(const Vector& vector) {
    set_x_y(get_x() - vector.get_x(), get_y() - vector.get_y());
    return *this;
  }

  Vector& operator+=(const double angle) {
    azimuth_ = angle_mod(azimuth_ + angle);
    return *this;
  }

  Vector& operator-=(const double angle) {
    azimuth_ = angle_mod(azimuth_ - angle);
    return *this;
  }

  Vector& operator*=(const double multiplier) {
    length_ *= multiplier;
    return *this;
  }

  Vector operator+(const Vector& vector) const {
    Vector result(*this);
    result += vector;
    return result;
  }

  Vector operator+(const double angle) const {
    Vector result(*this);
    result += angle;
    return result;
  }

  Vector operator-(const Vector& vector) const {
    Vector result(*this);
    result -= vector;
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
      default: throw std::out_of_range("Index is out of range for Vector");
    }
  }

  Vector& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_azimuth(value);
      case 1: return set_length(value);
      default: throw std::out_of_range("Index is out of range for Vector");
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

private:
  friend class Position;
  double azimuth_;
  double length_;
};


Vector operator*(const double multiplier, const Vector& vector) {
  return vector.operator*(multiplier);
}

Vector operator+(const double angle, const Vector& vector) {
  return vector.operator+(angle);
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

  Position& operator+=(const Vector& vector) {
    static const Rhumb& rhumb = Rhumb::WGS84();
    double latitude;
    double longitude;
    rhumb.Direct(latitude_, longitude_, vector.azimuth_, vector.length_, latitude, longitude);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator-=(const Vector& vector) {
    static const Rhumb& rhumb = Rhumb::WGS84();
    double latitude;
    double longitude;
    rhumb.Direct(latitude_, longitude_, vector.azimuth_, -vector.length_, latitude, longitude);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator*=(const Vector& vector) {
    static const Geodesic& geodesic = Geodesic::WGS84();
    double latitude;
    double longitude;
    double azimuth;
    geodesic.Direct(latitude_, longitude_, vector.azimuth_, vector.length_, latitude, longitude, azimuth);
    latitude_ = latitude;
    longitude_ = longitude;
    return *this;
  }

  Position& operator/=(const Vector& vector) {
    static const Geodesic& geodesic = Geodesic::WGS84();
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
      default: throw std::out_of_range("Index is out of range for Position");
    }
  }

  Position& set_item(int i, const double value) {
    i = i < 0 ? i + 2 : i;
    switch(i) {
      case 0: return set_latitude(value);
      case 1: return set_longitude(value);
      default: throw std::out_of_range("Index is out of range for Position");
    }
  }

  constexpr int get_len() const {
    return 2;
  }

  double operator[](const int i) {
    return get_item(i);
  }

private:
  double latitude_;
  double longitude_;
};


Vector operator-(const Position& position2, const Position& position1) {
  static const Rhumb& rhumb = Rhumb::WGS84();
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
  static const Geodesic& geodesic = Geodesic::WGS84();
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


PYBIND11_MODULE(geofun2, m) {
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
  m.def("geodesic_inverse", &geodesic_inverse, "latitude"_a, "longitude"_a, "azimuth"_a, "distance"_a,
      "Get starting azimuth, distance and ending azimuth of great circle between positions");

  // Angle arithmetic
  m.def("angle_mod", py::vectorize(angle_mod), "Return angle bound to [0.0, 360.0>");
  m.def("angle_mod_signed", py::vectorize(angle_mod_signed), "Return angle bound to [-180.0, 180.0>");
  m.def("angle_diff", py::vectorize(angle_diff), "Signed difference between to angles");

  // Primitives
  py::class_<Point>(m, "Point")
    .def(py::init<>())
    .def(py::init<Point&>())
    .def(py::init<const double, const double>())
    .def("__getitem__", &Point::get_item)
    .def("__setitem__", &Point::set_item)
    .def("__len__", &Point::get_len)
    .def("__copy__", [](const Point& self) { return Point(self); })
    .def("__deepcopy__", [](const Point& self, py::dict) { return Point(self); }, "memo"_a) 
    .def("copy", [](const Position& self) { return Position(self); })
    .def_property("x", &Point::get_x, &Point::set_x)
    .def_property("y", &Point::get_y, &Point::set_y)
    .def(py::self == py::self)
    .def(py::self += py::self)
    .def(py::self + py::self)
    .def(py::self -= py::self)
    .def(py::self - py::self)
    .def(py::self *= float())
    .def(float() * py::self)
    .def(py::self * float())
    .def(-py::self)
    ;

  py::class_<Vector>(m, "Vector")
    .def(py::init<>())
    .def(py::init<Vector&>())
    .def(py::init<const double, const double>())
    .def("__getitem__", &Vector::get_item)
    .def("__setitem__", &Vector::set_item)
    .def("__len__", &Vector::get_len)
    .def("__copy__", [](const Vector& self) { return Vector(self); })
    .def("__deepcopy__", [](const Vector& self, py::dict) { return Vector(self); }, "memo"_a) 
    .def("copy", [](const Vector& self) { return Vector(self); })
    .def_property("azimuth", &Vector::get_azimuth, &Vector::set_azimuth)
    .def_property("length", &Vector::get_length, &Vector::set_length)
    .def_property("x", &Vector::get_x, &Vector::set_x)
    .def_property("y", &Vector::get_y, &Vector::set_y)
    .def(py::self == py::self)
    .def(py::self += py::self)
    .def(py::self + py::self)
    .def(py::self -= py::self)
    .def(py::self - py::self)
    .def(py::self *= float())
    .def(float() * py::self)
    .def(py::self * float())
    .def(py::self += float())
    .def(float() + py::self)
    .def(py::self + float())
    .def(-py::self)
    ;

  py::class_<Position>(m, "Position")
    .def(py::init<>())
    .def(py::init<Position&>())
    .def(py::init<const double, const double>())
    .def(py::init<const int, const int>())
    .def("__getitem__", &Position::get_item)
    .def("__setitem__", &Position::set_item)
    .def("__len__", &Position::get_len)
    .def("__copy__", [](const Position& self) { return Position(self); })
    .def("__deepcopy__", [](const Position& self, py::dict) { return Position(self); }, "memo"_a) 
    .def("copy", [](const Position& self) { return Position(self); })
    .def("__len__", &Position::get_len)
    .def_property("latitude", &Position::get_latitude, &Position::set_latitude)
    .def_property("longitude", &Position::get_longitude, &Position::set_longitude)
    .def(py::self == py::self)
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
    ;
}
