#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class CModuleEntities : public IModule
{
	std::vector< CHandleManager* > om_to_update;
	std::vector< CHandleManager* > om_to_render_debug;
	void loadListOfManagers(const json& j, std::vector< CHandleManager* > &managers);
	void renderDebugOfComponents();
  bool _animationsEnabled = true;

public:
	float time_scale_factor = 1.f;

	CModuleEntities(const std::string& aname) : IModule(aname) { }
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;
	void destroyAllEntities();
	float getTimeScale() { return time_scale_factor; }
  bool const getAnimationsEnabled() { return _animationsEnabled; }
  void setAnimationsEnabled(bool anims) { _animationsEnabled = anims; }

	template< class TMsg >
	void broadcastMsg(const TMsg& msg) {
		auto om = getObjectManager<CEntity>();
		om->forEach([&](CEntity* e) {
			CHandle h_e(e);
			h_e.sendMsg(msg);
		});
	}
};

CHandle getEntityByName(const std::string& name);
