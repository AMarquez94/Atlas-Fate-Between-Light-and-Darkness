#ifndef INC_GEOMETRY_TRANSFORM_H_
#define INC_GEOMETRY_TRANSFORM_H_

#include "utils\utils.h"

class CTransform {
    QUAT rot;
    VEC3 pos;
    float scale = 1.0f;

public:

    CTransform combineWith(const CTransform& delta_transform) const;

    // -------------------------------------------
    VEC3 getPosition() const { return pos; }
    void setPosition(VEC3 new_pos) { pos = new_pos; }

    // -------------------------------------------
    QUAT getRotation() const { return rot; }
    void setRotation(QUAT new_rot) { rot = new_rot; }

    float getScale() const { return scale; }
    void  setScale(float new_scale) { scale = new_scale; }
    // -------------------------------------------
    MAT44 asMatrix() const {
        return MAT44::CreateScale(scale)
            * MAT44::CreateFromQuaternion(rot)
            * MAT44::CreateTranslation(pos)
            ;
    }

    // -------------------------------------------
    void setYawPitchRoll(float new_yaw, float new_pitch, float new_roll = 0.f);
    void getYawPitchRoll(float* yaw, float* pitch, float* roll = nullptr) const;
    void lookAt(VEC3 new_pos, VEC3 new_target);

    // -------------------------------------------
    VEC3 getFront() const {
        // - Because MAT44 forward is like cameras than look backward.
        return -MAT44::CreateFromQuaternion(rot).Forward();
    }

    VEC3 getLeft() const {
        return -MAT44::CreateFromQuaternion(rot).Left();
    }

    VEC3 getUp() const {
        return MAT44::CreateFromQuaternion(rot).Up();
    }

    // -------------------------------------------
    bool isInFront(VEC3 p) const {
        VEC3 delta = p - pos;
        return delta.Dot(getFront()) > 0.f;
    }

    bool isInLeft(VEC3 p) const {
        VEC3 delta = p - pos;
        return delta.Dot(getLeft()) > 0.f;
    }

    // -------------------------------------------
    float getDeltaYawToAimTo(VEC3 p) const {
        VEC3 delta = p - pos;
        float amount_front = getFront().Dot(delta);
        float amount_left = getLeft().Dot(delta);
        return atan2f(amount_left, amount_front);
    }

    float getDeltaPitchToAimTo(VEC3 p) const {
        VEC3 delta = p - pos;
        float amount_front = getFront().Dot(delta);
        float amount_up = getUp().Dot(delta);
        return atan2f(amount_up, amount_front);
    }

    bool isInFov(VEC3 p, float hor_full_fov, float ver_full_fov) const {
        return fabsf(getDeltaYawToAimTo(p)) <= (hor_full_fov * 0.5f) &&
            fabsf(getDeltaPitchToAimTo(p)) <= (ver_full_fov * 0.5f);
    }

    bool isInVerticalFov(VEC3 p, float ver_full_fov) const {
        return fabsf(getDeltaPitchToAimTo(p)) <= (ver_full_fov * 0.5f);
    }

    bool isInHorizontalFov(VEC3 p, float hor_full_fov) const {
        return fabsf(getDeltaYawToAimTo(p)) <= (hor_full_fov * 0.5f);
    }

    // -------------------------------------------
    bool load(const json& j);
    bool debugInMenu();

};


#endif

