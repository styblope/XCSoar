// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#include "FinishPoint.hpp"
#include "Task/TaskBehaviour.hpp"

#include <stdlib.h>
#include <cassert>

FinishPoint::FinishPoint(std::unique_ptr<ObservationZonePoint> &&_oz, WaypointPtr && wp,
                         const TaskBehaviour& tb,
                         const FinishConstraints &_constraints,
                         bool boundary_scored)
  :OrderedTaskPoint(TaskPointType::FINISH, std::move(_oz), std::move(wp),
                    boundary_scored),
   safety_height(tb.safety_height_arrival),
   constraints(_constraints)
{
}

void
FinishPoint::SetTaskBehaviour(const TaskBehaviour &tb) noexcept
{
  safety_height = tb.safety_height_arrival;
}

void
FinishPoint::Reset()
{
  OrderedTaskPoint::Reset();
  fai_finish_height = 0;
}

bool
FinishPoint::EntryPrecondition() const
{
  return GetPrevious() != NULL && GetPrevious()->HasEntered();
}

double
FinishPoint::GetElevation() const noexcept
{
  const auto nominal_elevation = GetBaseElevation() + safety_height;

  if (constraints.fai_finish) {
    return std::max(nominal_elevation, fai_finish_height);
  } else {
    return std::max(nominal_elevation,
                    constraints.min_height +
                    (constraints.min_height_ref == AltitudeReference::AGL
                     ? GetBaseElevation() : 0));
  }
}

void
FinishPoint::SetOrderedTaskSettings(const OrderedTaskSettings &otb) noexcept
{
  OrderedTaskPoint::SetOrderedTaskSettings(otb);
  constraints = otb.finish_constraints;
}

void
FinishPoint::SetNeighbours(OrderedTaskPoint *_prev,
                           OrderedTaskPoint *_next) noexcept
{
  assert(_next == NULL);
  // should not ever have an outbound leg
  OrderedTaskPoint::SetNeighbours(_prev, _next);
}

void
FinishPoint::SetFaiFinishHeight(const double height)
{
  fai_finish_height = std::max(0., height);
}

bool
FinishPoint::IsInSector(const AircraftState &state) const noexcept
{
  if (!OrderedTaskPoint::IsInSector(state))
    return false;

  return InInHeightLimit(state);
}

bool
FinishPoint::InInHeightLimit(const AircraftState &state) const
{
  if (!constraints.CheckHeight(state, GetBaseElevation()))
    return false;

  if (constraints.fai_finish)
    return state.altitude > fai_finish_height;

  return true;
}

bool
FinishPoint::CheckEnterTransition(const AircraftState &ref_now,
                                  const AircraftState &ref_last) const noexcept
{
  const bool now_in_height = InInHeightLimit(ref_now);
  const bool last_in_height = InInHeightLimit(ref_last);

  if (now_in_height && last_in_height)
    // both within height limit, so use normal location checks
    return OrderedTaskPoint::CheckEnterTransition(ref_now, ref_last);

  return false;
}
