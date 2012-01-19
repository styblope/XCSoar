/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "RouteConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "DataField/Enum.hpp"
#include "Interface.hpp"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "DataField/Base.hpp"
#include "Form/RowFormWidget.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  RoutePlannerMode,
  RoutePlannerAllowClimb,
  RoutePlannerUseCeiling,
  empty_spacer,
  TurningReach,
  FinalGlideTerrain,
  ReachPolarMode
};

class RouteConfigPanel : public RowFormWidget {
public:
  RouteConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook(), Layout::Scale(150)) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
  void ShowRouteControls(bool show);
  void ShowReachControls(bool show);
};

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static RouteConfigPanel *instance;

void
RouteConfigPanel::ShowRouteControls(bool show)
{
  SetRowVisible(RoutePlannerAllowClimb, show);
  SetRowVisible(RoutePlannerUseCeiling, show);
}

void
RouteConfigPanel::ShowReachControls(bool show)
{
  SetRowVisible(FinalGlideTerrain, show);
  SetRowVisible(ReachPolarMode, show);
}

static void
OnRouteMode(DataField *Sender,
            DataField::DataAccessKind_t Mode)
{
  const DataFieldEnum &df = *(const DataFieldEnum *)Sender;
  RoutePlannerConfig::Mode mode = (RoutePlannerConfig::Mode)df.GetAsInteger();
  instance->ShowRouteControls(mode != RoutePlannerConfig::Mode::NONE);
}

static void
OnReachMode(DataField *Sender,
            DataField::DataAccessKind_t Mode)
{
  const DataFieldEnum &df = *(const DataFieldEnum *)Sender;
  RoutePlannerConfig::ReachMode mode =
    (RoutePlannerConfig::ReachMode)df.GetAsInteger();
  instance->ShowReachControls(mode != RoutePlannerConfig::ReachMode::OFF);
}

void
RouteConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const ComputerSettings &settings_computer = XCSoarInterface::GetComputerSettings();
  const RoutePlannerConfig &route_planner = settings_computer.task.route_planner;

  instance = this;
  RowFormWidget::Prepare(parent, rc);

  static gcc_constexpr_data StaticEnumChoice route_mode_list[] = {
    { (unsigned)RoutePlannerConfig::Mode::NONE, N_("None"),
      N_("Neither airspace nor terrain is used for route planning.") },
    { (unsigned)RoutePlannerConfig::Mode::TERRAIN, N_("Terrain"),
      N_("Routes will avoid terrain.") },
    { (unsigned)RoutePlannerConfig::Mode::AIRSPACE, N_("Airspace"),
      N_("Routes will avoid airspace.") },
    { (unsigned)RoutePlannerConfig::Mode::BOTH, N_("Both"),
      N_("Routes will avoid airspace and terrain.") },
    { 0 }
  };

  AddEnum(_("Route mode"), _T(""), route_mode_list,
          (unsigned)route_planner.mode, OnRouteMode);

  // Expert item (TODO)
  AddBoolean(_("Route climb"),
             _("When enabled and MC is positive, route planning allows climbs between the aircraft "
                 "location and destination."),
             route_planner.allow_climb);

  // Expert item
  AddBoolean(_("Route ceiling"),
             _("When enabled, route planning climbs are limited to ceiling defined by greater of "
                 "current aircraft altitude plus 500 m and the thermal ceiling.  If disabled, "
                 "climbs are unlimited."),
             route_planner.use_ceiling);

  static gcc_constexpr_data StaticEnumChoice turning_reach_list[] = {
    { (unsigned)RoutePlannerConfig::ReachMode::OFF, N_("Off"),
      N_("Reach calculations disabled.") },
    { (unsigned)RoutePlannerConfig::ReachMode::STRAIGHT, N_("Straight"),
      N_("The reach is from straight line paths from the glider.") },
    { (unsigned)RoutePlannerConfig::ReachMode::TURNING, N_("Turning"),
      N_("The reach is calculated allowing turns around terrain obstacles.") },
    { 0 }
  };

  AddSpacer(); // Spacer

  AddEnum(_("Reach mode"),
          _("How calculations are performed of the reach of the glider with respect to terrain."),
          turning_reach_list, (unsigned)route_planner.reach_calc_mode,
          OnReachMode);


  static gcc_constexpr_data StaticEnumChoice final_glide_terrain_list[] = {
    { FeaturesSettings::FGT_OFF, N_("Off"), N_("Disables the reach display.") },
    { FeaturesSettings::FGT_LINE, N_("Line"), N_("Draws a dashed line at the glide reach.") },
    { FeaturesSettings::FGT_SHADE, N_("Shade"), N_("Shades terrain outside glide reach.") },
    { 0 }
  };

  AddEnum(_("Reach display"), _T(""), final_glide_terrain_list,
          settings_computer.final_glide_terrain);

  static gcc_constexpr_data StaticEnumChoice reach_polar_list[] = {
    { (unsigned)RoutePlannerConfig::Polar::TASK, N_("Task"),
      N_("Uses task glide polar.") },
    { (unsigned)RoutePlannerConfig::Polar::SAFETY, N_("Safety MC"),
      N_("Uses safety MacCready value") },
    { 0 }
  };

  // Expert item
  AddEnum(_("Reach polar"),
          _("This determines the glide performance used in reach, landable arrival, abort and alternate calculations."),
          reach_polar_list, (unsigned)route_planner.reach_polar_mode);

  ShowRouteControls(route_planner.mode != RoutePlannerConfig::Mode::NONE);
  ShowReachControls(route_planner.reach_calc_mode != RoutePlannerConfig::ReachMode::OFF);
}

bool
RouteConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;
  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  RoutePlannerConfig &route_planner = settings_computer.task.route_planner;

  changed |= SaveValueEnum(RoutePlannerMode, szProfileRoutePlannerMode,
                           route_planner.mode);

  changed |= SaveValueEnum(ReachPolarMode, szProfileReachPolarMode,
                           route_planner.reach_polar_mode);

  changed |= SaveValueEnum(FinalGlideTerrain, szProfileFinalGlideTerrain,
                           settings_computer.final_glide_terrain);

  changed |= SaveValue(RoutePlannerAllowClimb, szProfileRoutePlannerAllowClimb,
                       route_planner.allow_climb);

  changed |= SaveValue(RoutePlannerUseCeiling, szProfileRoutePlannerUseCeiling,
                       route_planner.use_ceiling);

  changed |= SaveValueEnum(TurningReach, szProfileTurningReach,
                           route_planner.reach_calc_mode);
  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateRouteConfigPanel()
{
  return new RouteConfigPanel();
}
