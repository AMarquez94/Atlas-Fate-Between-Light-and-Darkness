#ifndef INC_COMP_TAGS_H_
#define INC_COMP_TAGS_H_

#include "comp_base.h"

class CTagsManager {

	std::unordered_map< uint32_t, VHandles > entities_by_tag;
	std::unordered_map< uint32_t, std::string > tags_names;

public:
	void registerEntityHasTag(CHandle h, uint32_t tag);
	void unregisterEntityHasTag(CHandle h, uint32_t tag);
	const VHandles& getAllEntitiesByTag(uint32_t tag);
    const VHandles getAllEntitiesWithoutTags(const std::vector<uint32_t>& tags);
	static CTagsManager &get();
	void debugInMenu();

	void registerTagName(uint32_t tag, const std::string& tag_name);
	const std::string& getTagName(uint32_t tag) const;
};

struct TCompTags : public TCompBase {

	const static uint32_t max_tags = 4;
	uint32_t tags[max_tags];

	~TCompTags();
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	bool addTag(uint32_t tag);
	bool hasTag(uint32_t tag) const;
	void delTag(uint32_t tag);

};


#endif
