#include "mcv_platform.h"
#include "bone_correction.h"
#include "cal3d/cal3d.h"
#include "cal3d2engine.h"

/* 

  Each bone correction which apply a local transform to a bone
  such as that one local direction aims to a world direction

  We have to choose which local direction we want to aim to that 
  target direction, as the artist can set any axis to any direction

  We normally want the 'front' local direction to aim to use as
  direction to modify, but sometimes it's the green axis, sometimes another
  Use that bone_ids_to_debug or the imgui to 'view' those directions.
  
  The last correction should have the amount = 1 to force a proper lookat

*/

void TBoneCorrection::load(const json& j) {
  bone_name = j.value("bone", "");
  bone_id = -1;
  local_axis_to_correct = loadVEC3(j["local_axis_to_correct"]);
  amount = j.value("amount", 0.f);
}

void TBoneCorrection::debugInMenu() {
  ImGui::LabelText("Bone", "%s (%d)", bone_name.c_str(), bone_id);
  ImGui::DragFloat("Amount", &amount, 0.01f, 0.0f, 1.0f);
  ImGui::InputFloat3("Local Axis", &local_axis_to_correct.x);
}

// -----------------------------------------
QUAT getRotationFromAToB(VEC3 A, VEC3 B, float unit_amount) {

  VEC3 rotation_axis = A.Cross(B);
  rotation_axis.Normalize();

  // Get the angle to rotate 
  VEC3 angle_between_a_b = DirectX::XMVector3AngleBetweenVectors(A, B);
  float angle = angle_between_a_b.x * unit_amount;

  // Nothing to correct
  if (fabsf(angle) < 1e-2f)
    return QUAT::Identity;

  return QUAT::CreateFromAxisAngle(rotation_axis, angle);
}

void TBoneCorrection::apply(CalSkeleton* skel, VEC3 dx_world_target, float external_unit_amount) const {

  CalVector world_target = DX2Cal(dx_world_target);

  // We want to correct a bone by name, but we need a bone_id
  // to access that bone
  if (bone_id == -1)   // The bone_name does not exists in the skel...
    return;

  CalBone* bone = skel->getBone(bone_id);
  assert(bone);

  // Where is the bone now in abs coords
  CalVector bone_abs_pos = bone->getTranslationAbsolute();

  // In world coordinate system...
  CalVector abs_dir = world_target - bone_abs_pos;

  // getRotationAbsolute goes from local to abs, so I need the inverse
  CalQuaternion rot_abs_to_local = bone->getRotationAbsolute();
  rot_abs_to_local.invert();

  // Convert from absolute coords to local bone coords
  CalVector local_dir = abs_dir;
  local_dir *= rot_abs_to_local;

  // Find a quaternion to rotate 'local_axis_to_correct' to 'local_dir'
  CalQuaternion q_rot = DX2Cal(getRotationFromAToB(local_axis_to_correct, Cal2DX(local_dir), amount * external_unit_amount));

  // Add the correction before the current bone
  CalQuaternion new_rot = q_rot;
  new_rot *= bone->getRotation();

  bone->setRotation(new_rot);
  bone->calculateState();
}
