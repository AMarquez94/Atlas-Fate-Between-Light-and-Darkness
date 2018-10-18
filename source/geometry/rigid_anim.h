#pragma once

#include "resources/resource.h"

namespace RigidAnims {

  class CRigidAnimResource;

  // ----------------------------------------------
  struct TKey {
    VEC3  pos;
    float scale;
    QUAT  rot;
  };

  // ----------------------------------------------
  struct CController {
    static const uint32_t invalid_track_index = ~0;
    const CRigidAnimResource* anims = nullptr;
    uint32_t                  track_index = invalid_track_index;
    std::string               track_name;
    bool sample(TKey* out_key, float t) const;
	bool isEmpty();
  };

  // ----------------------------------------------
  struct TTrackInfo {
    static const int name_max_length = 32;
    char     name[name_max_length];
    uint32_t first_key = 0;
    uint32_t num_keys = 0;
    float    fps = 0.f;
    float    min_time = 0.f;
    float    max_time = 1.0f;
  };

  // ----------------------------------------------
  class CRigidAnimResource : public IResource {

    std::vector< TTrackInfo > tracks;
    std::vector< TKey > keys;
    float total_duration = 0.f;

  public:
    bool create(const std::string& name);
    void debugInMenu() override;
    bool sample(uint32_t track_index, TKey* out_key, float t) const;
	bool isEmpty(uint32_t track_index) const;
    void onFileChanged(const std::string& filename) override;
    uint32_t findTrackIndexByName(const std::string& name) const;
  };

}
