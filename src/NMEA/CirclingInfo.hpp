// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

#include "Geo/GeoPoint.hpp"
#include "time/FloatDuration.hxx"
#include "time/Stamp.hpp"

#include <type_traits>

#include <cstdint>

/** Enumeration for cruise/circling mode detection */
enum class CirclingMode: uint8_t {
  /** Established cruise mode */
  CRUISE = 0,
  /** In cruise, pending transition to climb */
  POSSIBLE_CLIMB,
  /** Established climb mode */
  CLIMB,
  /** In climb, pending transition to cruise */
  POSSIBLE_CRUISE,
};

/** Data for tracking of climb/cruise mode and transition points */
struct CirclingInfo
{
  /** Turn rate based on track (angle/s)*/
  Angle turn_rate;

  /** Turn rate based on heading (including wind) (angle/s)*/
  Angle turn_rate_heading;

  /** Turn rate after low pass filter (angle/s)*/
  Angle turn_rate_smoothed;

  /** Turn rate based on heading after low pass filter (angle/s)*/
  Angle turn_rate_heading_smoothed;

  /** StartLocation of the current/last climb */
  GeoPoint climb_start_location;

  /**
   * Start altitude of the current/last climb.
   */
  double climb_start_altitude;

  /**
   * Start altitude of the current/last climb (total energy).
   */
  double climb_start_altitude_te;

  /** StartTime of the current/last climb */
  TimeStamp climb_start_time;

  /** StartLocation of the current/last cruise */
  GeoPoint cruise_start_location;
  /** StartAltitude of the current/last cruise */
  double cruise_start_altitude;
  /** StartAltitude of the current/last cruise (total energy) */
  double cruise_start_altitude_te;
  /** StartTime of the current/last cruise */
  TimeStamp cruise_start_time;

  /** Current TurnMode (Cruise, Climb or somewhere between) */
  CirclingMode turn_mode;

  /**
   * True if the turn rate is above the threshold for circling.
   */
  bool turning;

  /** True if in circling mode, False otherwise */
  bool circling;

  /**
   * Circling/total time ratio in percent.  Negative value means
   * "unknown".
   */
  double circling_percentage;

  /**
   * Time spent climbing and circling, ratio in percent.  Negative value means
   * "unknown".
   */
  double circling_climb_percentage;

  /**
   * Time spent climbing and not circling, ratio in percent.  Negative value means
   * "unknown".
   */
  double noncircling_climb_percentage;

  /** Time spent in cruise mode */
  FloatDuration time_cruise;
  /** Time spent in circling mode */
  FloatDuration time_circling;
  /** Time spent in circling mode and climbing */
  FloatDuration time_climb_circling;
  /** Time spent in non-circling climb */
  FloatDuration time_climb_noncircling;

  /** Maximum height gain (from MinAltitude) during task */
  double max_height_gain;

  /** Total height climbed during task */
  double total_height_gain;

  void Clear();

  bool TurningLeft() const {
    return turn_rate_smoothed.IsNegative();
  }
};

static_assert(std::is_trivial<CirclingInfo>::value, "type is not trivial");
