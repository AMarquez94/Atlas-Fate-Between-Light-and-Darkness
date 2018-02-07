#pragma once

#include "modules/module.h"
#include "input/enums.h"
#include "input/host.h"

namespace Input
{
	class IDevice;
}

class CModuleInput : public IModule
{
public:
	CModuleInput(const std::string& name);
  bool start() override;
  bool stop() override;
  void update(float delta) override;
	void render() override;

	void assignDevice(int hostIdx, Input::IDevice* device);
	void assignMapping(int hostIdx, Input::CMapping* mapping);

	Input::IDevice* getDevice(const std::string& name);
	const Input::CHost& host(Input::PlayerID playerId) const;
	const Input::TInterface_Keyboard& keyboard() const;
	const Input::TInterface_Mouse& mouse() const;
	const Input::TInterface_Pad& pad() const;
	const Input::TInterface_Mapping& mapping() const;
	const Input::CHost& operator[](Input::PlayerID playerId) const;
	const Input::TButton& operator[](Input::KeyID key) const;
	const Input::TButton& operator[](Input::EMouseButton bt) const;
	const Input::TButton& operator[](Input::EPadButton bt) const;
	const Input::TButton& operator[](const std::string& name) const;

	void feedback(const Input::TInterface_Feedback& data);

	void loadButtonDefinitions(const std::string& filename);
	const Input::TButtonDef* getButtonDefinition(const std::string& name) const;
	const std::string& getButtonName(Input::EInterface type, int id) const;

private:
	void renderInMenu();

	Input::CHost _hosts[Input::NUM_PLAYERS];
	std::vector<Input::IDevice*> _registeredDevices;
	std::map<std::string, Input::TButtonDef> _buttonDefinitions;
};
