#pragma once

enum eTextureSlot {
  // Materials
  TS_ALBEDO,
  TS_LIGHTMAP,
  TS_NORMAL,
  // Other slots of the material
  TS_NUM_MATERIALS_SLOTS,
  // Here goes the slots not associated to a material
  TS_LIGHT_PROJECTOR = TS_NUM_MATERIALS_SLOTS,
  // other textures not from the material...
  TS_COUNT
};