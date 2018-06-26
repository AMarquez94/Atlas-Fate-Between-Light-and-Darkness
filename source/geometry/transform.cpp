#include "mcv_platform.h"
#include "transform.h"

bool CTransform::load(const json& j) {

    if (j.count("pos"))
        pos = loadVEC3(j["pos"]);

    if (j.count("rotation"))
        setRotation(loadQUAT(j["rotation"]));

    if (j.count("lookat"))
        lookAt(getPosition(), loadVEC3(j["lookat"]));

    if (j.count("axis")) {
        VEC3 axis = loadVEC3(j["axis"]);
        float angle_deg = j.value("angle", 0.f);
        float angle_rad = deg2rad(angle_deg);
        setRotation(QUAT::CreateFromAxisAngle(axis, angle_rad));
    }
    if (j.count("scale"))
        scale = j.value("scale", 1.0f);
    return true;
}

// ---------------------------
CTransform CTransform::combineWith(const CTransform& delta_transform) const {
    CTransform new_t;
    new_t.rot = delta_transform.rot * rot;

    VEC3 delta_pos_rotated = VEC3::Transform(delta_transform.pos, rot);
    new_t.pos = pos + (delta_pos_rotated * scale);

    new_t.scale = scale * delta_transform.scale;
    return new_t;
}

// ---------------------------

void CTransform::getYawPitchRoll(float* yaw, float* pitch, float* roll) const {
    VEC3 f = getFront();
    getYawPitchFromVector(f, yaw, pitch);

    // If requested...
    if (roll) {
        VEC3 roll_zero_left = VEC3(0, 1, 0).Cross(getFront());
        VEC3 roll_zero_up = VEC3(getFront()).Cross(roll_zero_left);
        VEC3 my_real_left = getLeft();
        float rolled_left_on_up = my_real_left.Dot(roll_zero_up);
        float rolled_left_on_left = my_real_left.Dot(roll_zero_left);
        *roll = atan2f(rolled_left_on_up, rolled_left_on_left);
    }
}

void CTransform::setYawPitchRoll(float new_yaw, float new_pitch, float new_roll) {
    // pitch goes in the opposite direction in SimpleMath
    rot = QUAT::CreateFromYawPitchRoll(new_yaw, -new_pitch, new_roll);
}

void CTransform::lookAt(VEC3 new_pos, VEC3 new_target) {
    pos = new_pos;
    VEC3 front = new_target - new_pos;
    float yaw, pitch;
    getYawPitchFromVector(front, &yaw, &pitch);
    setYawPitchRoll(yaw, pitch, 0.f);
}

bool CTransform::debugInMenu() {
    bool changed = false;
    float posfloat[] = { pos.x ,pos.y ,pos.z };
    changed |= ImGui::DragFloat3("Pos", posfloat, 0.025f, -1000.f, 1000.f);
    pos.x = posfloat[0];
    pos.y = posfloat[1];
    pos.z = posfloat[2];
    changed |= ImGui::DragFloat("Scale", &scale, 0.01f, -10.f, 10.f);
    // Angulos
    float yaw, pitch, roll;
    getYawPitchRoll(&yaw, &pitch, &roll);
    yaw = rad2deg(yaw);
    pitch = rad2deg(pitch);
    roll = rad2deg(roll);
    changed |= ImGui::DragFloat("Yaw", &yaw, 0.1f, -180.f, 180.f);
    const float max_pitch = 89.95f;
    changed |= ImGui::DragFloat("Pitch", &pitch, 0.1f, -max_pitch, max_pitch);
    changed |= ImGui::DragFloat("Roll", &roll, 0.1f, -180.f, 180.f);
    yaw = deg2rad(yaw);
    pitch = deg2rad(pitch);
    roll = deg2rad(roll);
    setYawPitchRoll(yaw, pitch, roll);

    // 
    VEC3 left = getLeft();
    VEC3 up = getUp();
    VEC3 front = getFront();
    ImGui::Text("Left: %f %f %f", left.x, left.y, left.z);
    ImGui::Text("Up: %f %f %f", up.x, up.y, up.z);
    ImGui::Text("Front: %f %f %f", front.x, front.y, front.z);
    ImGui::Text("Quaternion: %f %f %f %f", getRotation().x, getRotation().y, getRotation().z, getRotation().w);

    return changed;
}

QUAT CTransform::getLookAt(VEC3 new_pos, VEC3 new_target, float customRoll)
{
    VEC3 front = new_target - new_pos;
    float yaw, pitch;
    getYawPitchFromVector(front, &yaw, &pitch);
    return QUAT::CreateFromYawPitchRoll(yaw, -pitch, customRoll);
}

