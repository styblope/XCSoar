// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Math/Point2D.hpp"

#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/core/cs.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D(DoublePoint2D, double,
                                 boost::geometry::cs::cartesian, x, y)
