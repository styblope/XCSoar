// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The XCSoar Project

#pragma once

struct lua_State;

namespace Lua {

/**
 * Provide the Lua table "xcsoar.wind".
 */
void
InitWind(lua_State *L);

}
