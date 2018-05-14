#include "mcv_platform.h"
#include "comp_tags.h"
#include "entity/entity.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("tags", TCompTags);

/*

dado un tag -> lista de handles que lo tienen
dado un tag_id -> dame el tag_name

*/

static CTagsManager tags_manager;
CTagsManager &CTagsManager::get() {
	return tags_manager;
}

const VHandles& CTagsManager::getAllEntitiesByTag(uint32_t tag) {
	static VHandles empty_set;
	auto it = entities_by_tag.find(tag);
	if (it == entities_by_tag.end())
		return empty_set;
	return it->second;
}

void CTagsManager::registerEntityHasTag(CHandle h, uint32_t tag) {
	entities_by_tag[tag].push_back(h);
}

void CTagsManager::unregisterEntityHasTag(CHandle h, uint32_t tag) {
	auto it = entities_by_tag.find(tag);
	if (it == entities_by_tag.end())
		return;

	// In the array of handles, search the handle, if found delete it
	auto& handles = it->second;
	auto it_h = std::find(handles.begin(), handles.end(), h);
	if (it_h != handles.end())
		handles.erase(it_h);
}

// ------------------------------------------------------------------------
void CTagsManager::registerTagName(uint32_t tag, const std::string& tag_name) {
	tags_names[tag] = tag_name;
}

const std::string& CTagsManager::getTagName(uint32_t tag) const {
	auto it = tags_names.find(tag);
	if (it == tags_names.end()) {
		static const std::string undef_tag("unknown_tag");
		return undef_tag;
	}
	return it->second;
}

void CTagsManager::debugInMenu() {
	if (ImGui::TreeNode("Tag Manager")) {
		for (auto& it : tags_names) {
			// Use the name for the tree node...
			if (ImGui::TreeNode(it.second.c_str())) {
				for (auto& ig : entities_by_tag[it.first]) {
					ig.debugInMenu();
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}


// -------------------------------------------------------------
TCompTags::~TCompTags() {
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i])
			delTag(tags[i]);
	}
}

void TCompTags::debugInMenu() {
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i]) {
			ImGui::Text("Tag[%d] : %s (%08x)", i, tags_manager.getTagName(tags[i]).c_str(), tags[i]);
			ImGui::SameLine();
			if (ImGui::SmallButton("Del")) {
				delTag(tags[i]);
			}
		}
	}
}

bool TCompTags::hasTag(uint32_t tag) const {
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i] == tag)
			return true;
	}
	return false;
}

bool TCompTags::addTag(uint32_t tag) {

	// First check the tag is not already in our list
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i] == tag)
			return true;
	}
	// Find an empty slot
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i] == 0) {
			tags[i] = tag;
			tags_manager.registerEntityHasTag(CHandle(this).getOwner(), tag);
			return true;
		}
	}
	fatal("Too many tags already registered in entity while adding tag %08x\n", tag);
	return false;
}

void TCompTags::delTag(uint32_t tag) {
	for (int i = 0; i < max_tags; ++i) {
		if (tags[i] == tag) {
			tags_manager.unregisterEntityHasTag(CHandle(this).getOwner(), tag);
			tags[i] = 0;
		}
	}
}

void TCompTags::load(const json& j, TEntityParseContext& ctx) {

	CHandle(this).setOwner(ctx.current_entity);

	for (int i = 0; i < max_tags; ++i)
		tags[i] = 0;

	assert(j.is_array());
	int idx = 0;
	for (auto jtag : j) {
		assert(idx < max_tags);
		auto tag_name = jtag.get<std::string>();
		auto tag_id = getID(tag_name.c_str());

		tags_manager.registerTagName(tag_id, tag_name);

		addTag(tag_id);
	}
}
