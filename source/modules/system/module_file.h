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
    bool removeResourceFile(const std::string& name);

    bool ending_engine = false;

    const std::vector<std::string> getFileResourceVector(std::string filename);
    void addPendingResourceFile(const std::string& resource, bool add = true);

    void addResourceToLoad(const std::string& resourceToLoad);
    void addVectorResourceToLoad(const std::vector<std::string>& resourcesToLoad);
    bool areResourcesToLoad() { return resources_to_load.size() > 0; };

private:

    /* Auxiliar methods */
    void writeResourcesInFile(const std::string scene_name, const std::vector<std::string>& resources);
    void preloadResources(bool overwrite);
    const std::vector<std::string> getResourcesByFile(const std::string filename);

    /* Resources by file */
    std::map<std::string, std::vector<std::string>> resources_by_file;

    /* Resource Files */
    std::recursive_mutex resource_files_mutex;
    std::map < std::string, std::vector<char>> resource_files;
    std::thread resource_thread;

    /* Pending Resource Files */
    std::vector<std::pair<std::string, bool>> pending_resource_files;
    std::mutex pending_resource_files_mutex;
    std::condition_variable condition_variable;

    /* Pending Resources to load */
    std::vector<std::string> resources_to_load;

    /* Thread */
    const std::pair<const std::string, bool> getFirstPendingResourceFile();
    void resourceThreadMain();
    void parseResourceScene(const json& j, std::vector<std::string>& scene_resources);
    void parseMaterial(const std::string& material_path, std::vector<std::string>& scene_resources);
};