#pragma once

class IModule
{
public:
	IModule(const std::string& name)
		: _name(name)
	{}

  virtual bool start() { return true; }
  virtual bool stop() { return true; }
  virtual void update(float delta) { (void)delta; }
  virtual void render() {}

  const std::string& getName() const { return _name; }
	bool isActive() const { return _active; }
	void setActive(bool how) { _active = how; }

  virtual LRESULT OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { return 0; }

private:
	std::string _name;
	bool _active;

};

using VModules = std::vector<IModule*>;
