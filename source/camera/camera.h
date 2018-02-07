#pragma once

class CCamera {

  VEC3 pos;         // eye
  VEC3 target;
  VEC3 up_aux;

  // 3 orthonormals axis in world space
  VEC3 front;     
  VEC3 up;
  VEC3 left;

  // Prepective params
  float fov_vertical;     // in radians!!
  float z_near;
  float z_far;
  float aspect_ratio;

  // Matrix
  MAT44 view;
  MAT44 proj;
  MAT44 view_proj;

  void updateViewProj();

public:

  // 
  VEC3 getPosition() const { return pos; }
  VEC3 getTarget() const { return target; }

  VEC3 getFront() const { return front; }
  VEC3 getUp() const { return up; }
  VEC3 getLeft() const { return left; }

  // -------------------------------------
  MAT44 getView() const {
    return view;
  }

  MAT44 getProjection() const {
    return proj;
  }

  MAT44 getViewProjection() const {
    return view_proj;
  }
  
  // -------------------------------------
  float getFov() const { return fov_vertical; }
  float getZNear() const { return z_near; }
  float getZFar() const { return z_far; }

  // 
  void lookAt(VEC3 new_pos, VEC3 new_target, VEC3 new_up_aux = VEC3(0, 1, 0));
  void setPerspective(float new_fov_vertical, float new_z_near, float new_z_far );

};