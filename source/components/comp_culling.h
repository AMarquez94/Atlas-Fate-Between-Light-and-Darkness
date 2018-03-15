#ifndef INC_COMPONENT_CULLING_H_
#define INC_COMPONENT_CULLING_H_

#include "geometry/geometry.h"
#include "comp_base.h"
#include "entity/entity.h"
#include <bitset>

struct TCompCulling : public TCompBase {
  static const size_t nBits = 4096;
  typedef std::bitset<nBits> TCullingBits;
  TCullingBits bits;

  // A single plane
  struct CPlane {
    VEC3  n;
    float d;
    void from(VEC4 k) {
      n = VEC3(k.x, k.y, k.z);
      d = k.w;
    }

    // Returns true if the aabb is fully in the negative side of the plane
    bool isCulled(const AABB* aabb) const {
      // Checking the 8 points at the same time
      // The 'r' will be max when n and extents have the same sign
      // as the Extents is already positive, we move the sign to the n
      const float r = aabb->Extents.x * fabsf(n.x)
        + aabb->Extents.y * fabsf(n.y)
        + aabb->Extents.z * fabsf(n.z)
        ;

      // Distance from box center to the plane
      const float c = n.Dot(aabb->Center) + d;
      return c < -r;
    }
  };

  // the 6 sides of the frustum
  enum EFrustumPlanes {
      FP_LEFT = 0
    , FP_RIGHT
    , FP_BOTTOM
    , FP_TOP
    , FP_NEAR
    , FP_FAR
    , FP_NPLANES
  };

  // An array of N planes
  class VPlanes : std::vector< CPlane > {
  public:
    void fromViewProjection(MAT44 view_proj) {
      resize(FP_NPLANES);
      MAT44 m = view_proj.Transpose();
      VEC4 mx = VEC4(m.m[0]);
      VEC4 my = VEC4(m.m[1]);
      VEC4 mz = VEC4(m.m[2]);
      VEC4 mw = VEC4(m.m[3]);
      VPlanes &vp = *this;
      vp[FP_LEFT].from(mw + mx);
      vp[FP_RIGHT].from(mw - mx);
      vp[FP_BOTTOM].from(mw + my);
      vp[FP_TOP].from(mw - my);
      vp[FP_NEAR].from(mw + mz);      // + mz if frustum is 0..1
      vp[FP_FAR].from(mw - mz);
    }

    bool isVisible(const AABB* aabb) const;
  };

  VPlanes planes;

  void debugInMenu();
  void update(float dt);
  void updateFromMatrix(MAT44 view_proj);

  DECL_SIBLING_ACCESS();
};


#endif
