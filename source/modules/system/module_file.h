#pragma once

#include "modules/module.h"
#include <mutex>
#include "entity/entity.h"

class CModuleFile : public IModule
{

public:

    CModuleFile(const std::string& aname) : IModule(aname) { }

    bool start() override;
    bool stop() override;
    void update(float delta) override;
    void render() override;

    std::vector<char> loadResourceFile(const std::string& name);

private:

    /* Auxiliar methods */
    void writeResourcesInFile(const std::string scene_name, const std::vector<std::string>& resources);
    void preloadResources(bool overwrite);
    const std::vector<std::string> getResourcesByFile(const std::string filename);

    /* Resource Files */
    std::recursive_mutex resource_files_mutex;
    std::map < std::string, std::vector<char>> resource_files;
    std::thread resource_thread;

    /* Pending Resource Files */
    void addPendingResourceFile(const std::string& resource);
    std::vector<std::string> pending_resource_files;
    std::mutex pending_resource_files_mutex;
    std::condition_variable condition_variable;

    /* Thread */
    bool ending_engine = false;
    const std::string getFirstPendingResourceFile();
    void resourceThreadMain();
    void parseResourceScene(const json& j, std::vector<std::string>& scene_resources);
    void parseMaterial(const std::string& material_path, std::vector<std::string>& scene_resources);
};