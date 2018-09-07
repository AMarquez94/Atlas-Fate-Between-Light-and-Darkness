#pragma once

class CalSkeleton;

struct TBoneCorrection {
  std::string bone_name;
  int         bone_id = -1;
  VEC3        local_axis_to_correct; // 0,1,-0.2
  float       amount;                // In case we want to rotate not the 100%
  void load(const json& j);
  void debugInMenu();
  void apply(CalSkeleton* skel, VEC3 world_target, float external_unit_amount) const;
};

