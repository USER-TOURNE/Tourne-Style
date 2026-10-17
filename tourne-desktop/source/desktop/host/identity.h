// Tourne identity: shape, material and motion, said once and read by every piece.
//
// The palette already works this way (theme-engine.h: one record, every piece
// follows it). This is the other half of the identity: how a desktop is cut
// (shape), what it is made of (material) and how it moves (motion). Each is a
// few words here, and the pieces' own settings (the switcher's corners and
// borders, the taskbar's previews and hiding, glass) take their values from
// them until you set one of those yourself:
//
//   defaults  <  identity  <  tourne.conf  <  the Studio's changes
//
//   identity.shape     inherit | square | subtle | rounded | pill
//   identity.frame     0 (leave it) or 1..8 px: the weight of drawn borders
//   identity.material  inherit | none | blur | acrylic | mica
//   identity.opacity   0 (leave it) or 10..100 %: how solid it is
//   identity.motion    inherit | off | snap | smooth | lively
//   identity.speed     50..300 %: how fast, 100 as designed
//
// All "inherit" (the default) changes nothing. Idle cost: none; it is worked
// out when the settings are read, not while anything runs.
#pragma once

#include "config.h"

namespace td {

void IdentityDefaults(Settings& out);
void IdentityDescribe(SchemaInfo& info);

// The settings the identity in `values` (the layers merged, over the defaults) asks of the pieces.
Settings IdentityDerive(const Settings& values, const Settings& defaults);

}  // namespace td
