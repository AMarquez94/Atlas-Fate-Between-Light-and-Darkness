#pragma once

#include "modules/module.h"

class CModuleLoading : public IModule
{
public:
    CModuleLoading(const std::string& name)
        : IModule(name)
    {}
    bool start() override;
    bool stop() override;
    void update(float delta) override;

private:

};