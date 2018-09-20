#include "mcv_platform.h"
#include "rigid_anim.h"
#include "utils/data_provider.h"

// ----------------------------------------------
class CRigidAnimResourceClass : public CResourceClass {
public:
  CRigidAnimResourceClass() {
    class_name = "RigidAnim";
    extensions = { ".anims" };
  }
  IResource* create(const std::string& name) const override {
    dbg("Creating json resource %s\n", name.c_str());
    RigidAnims::CRigidAnimResource* res = new RigidAnims::CRigidAnimResource();
    bool is_ok = res->create(name);
    assert(is_ok);
    return res;
  }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<RigidAnims::CRigidAnimResource>() {
  static CRigidAnimResourceClass the_resource_class;
  return &the_resource_class;
}

namespace RigidAnims {

  struct THeader {
    
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t num_tracks = 0;
    uint32_t num_keys = 0;
    uint32_t name_max_length = 0;
    uint32_t bytes_per_track = 0;
    float    total_duration = 0.f;

    static const uint32_t valid_magic = 0x11777711;
    static const uint32_t valid_version = 2;
    bool isValid() const {
      return magic == valid_magic 
        && version == valid_version
        && name_max_length == TTrackInfo::name_max_length
        && bytes_per_track == sizeof(TTrackInfo)
        ;
    }
  };

  bool CRigidAnimResource::create(const std::string& name) {
    
    // Try to load the file
    CFileDataProvider fdp(name.c_str());
    if (!fdp.isValid())
      return false;

    THeader header;
    fdp.read(header);
    if (!header.isValid())
      return false;

    tracks.resize(header.num_tracks);
    fdp.readBytes(tracks.data(), tracks.size() * sizeof(TTrackInfo));
    
    keys.resize(header.num_keys);
    fdp.readBytes(keys.data(), keys.size() * sizeof(TKey));

    total_duration = header.total_duration;

    return true;
  }

  void CRigidAnimResource::debugInMenu() {
    ImGui::Text("%ld tracks - %ld keys", tracks.size(), keys.size());
    if (ImGui::TreeNode("Tracks")) {
      for (auto& t : tracks) {
        ImGui::PushID(&t);
        if (ImGui::TreeNode(t.name)) {
          ImGui::Text("%d keys (From %d)", t.num_keys, t.first_key);
          ImGui::DragFloat("Min Time", &t.min_time, 0.01f, 0.f, 10.f);
          ImGui::DragFloat("Max Time", &t.max_time, 0.01f, 0.f, 10.f);
          ImGui::DragFloat("FPS", &t.fps, 0.01f, 0.f, 60.f);
          ImGui::TreePop();
        }
        ImGui::PopID();
      }
      ImGui::TreePop();
    }
  }

  bool CController::sample(TKey* out_key, float t) const {
    return anims->sample(track_index, out_key, t);
  }

  bool CController::isEmpty() {
	  return anims->isEmpty(track_index);
  }

  uint32_t CRigidAnimResource::findTrackIndexByName(const std::string& name) const {
    uint32_t idx = 0;
    for (auto& t : tracks) {
      if (t.name == name)
        return idx;
      ++idx;
    }
    return CController::invalid_track_index;
  }
  
  void CRigidAnimResource::onFileChanged(const std::string& filename) {
    create(filename);
  }

  bool CRigidAnimResource::sample(uint32_t track_index, TKey* out_key, float t) const {

    // asking for an invalid track, nothing to do.
    if( track_index >= tracks.size())
      return true;

    auto track = &tracks[track_index];
    
    float ut = 1.f;
    
    // If we only have one key, we are not really animated
    if (track->num_keys > 1) {

      float st = t;
      // Here enter the animated objects.
      if (t < track->min_time)
        st = track->min_time;
      else if (t > track->max_time)
        st = track->max_time;

      assert(track->max_time > track->min_time);
      ut = (st - track->min_time) / (track->max_time - track->min_time);
    }

    // Check time limits
    if (ut < 0) {
      *out_key = keys[track->first_key];
      return false;
    }

    // I want single key object to enter this if, because we don't have two
    // keys to interpolate
    if (ut >= 1.0f) {
      // Copy my last key
		if (track->num_keys == 0) {
			
			out_key->pos = VEC3(0,0,0);
			out_key->rot = QUAT(0,0,0,1);
			out_key->scale = 1.0f;
			return false;
		}

      *out_key = keys[track->first_key + track->num_keys - 1];
      // Return true only when the whole animation has loop
      return t >= this->total_duration;
    }

    float key_float_idx = (track->num_keys - 1) * ut;
    int key_index = (int)key_float_idx;
    float amount_of_next_key = key_float_idx - key_index;

    key_index += track->first_key;

    auto prev_key = &keys[key_index];
    auto next_key = &keys[key_index+1];
    out_key->pos = lerp(prev_key->pos, next_key->pos, amount_of_next_key);
    out_key->rot = QUAT::Slerp(prev_key->rot, next_key->rot, amount_of_next_key);
    out_key->scale = lerp(prev_key->scale, next_key->scale, amount_of_next_key);
    return false;
  }

  bool CRigidAnimResource::isEmpty(uint32_t track_index) const{
	  auto track = &tracks[track_index];
	  return track->num_keys == 0;
  }

}
