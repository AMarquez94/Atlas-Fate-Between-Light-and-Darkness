#ifndef INC_COMP_BONE_TRACKER_H_
#define INC_COMP_BONE_TRACKER_H_

#include "../comp_base.h"
#include "../../entity/entity.h"

class CalModel;

struct TCompBoneTracker : public TCompBase {

    CHandle     h_skeleton;        // Handle to comp_skeleton of the entity being tracked
    int         bone_id = -1;      // Id of the bone of the skeleton to track
    std::string bone_name;
    std::string parent_name;
	QUAT rot_offset;

    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    DECL_SIBLING_ACCESS();
};

#endif
