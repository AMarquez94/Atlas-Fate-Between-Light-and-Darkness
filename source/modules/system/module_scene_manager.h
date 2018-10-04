#pragma once

#include "modules/module.h"

// Container class for scenes.
class Scene {

public:
    bool isLoaded;
    CHandle root;

    VEC3 env_fog;
    VEC3 ground_fog;
    VEC3 shadow_color;
    float env_fog_density;
    float ground_fog_density;
    
    float scene_exposure;
    float scene_ambient;
    float scene_tone_mapping;
    float scene_gamma;
    float scene_shadow_intensity;

    std::string name;
    std::string navmesh;
    std::string initial_script_name;

    std::vector<std::string> groups_subscenes;
};

class CModuleSceneManager : public IModule
{
public:

    int sceneCount;
    bool hasReloaded = false;

    CModuleSceneManager(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;

    void preloadScene(const std::string& sceneName);
    bool loadScene(const std::string & name);
    Scene* createScene(const std::string& name);
    Scene* getSceneByName(const std::string& name);
    Scene* getActiveScene();
    bool unLoadActiveScene();

    std::string getDefaultSceneName();
    //bool endThread = false;

private:
    Scene * _activeScene;
    Scene * _persistentScene;
    std::string _default_scene;
    std::map<std::string, Scene*> _scenes;

    void loadJsonScenes(const std::string filepath);
    void setActiveScene(Scene* scene);

    struct PreResource {
        std::string name;
        bool isBig;
        bool operator == (const PreResource &rhs) const
        { 
            return this->name.compare(rhs.name) == 0;
        }
    };

    //std::thread sceneThread;
    //bool ending_thread = false;
    //void sceneThreadMain();
    //void parseResourceScene(const json& j, std::vector<PreResource>& scene_resources);

    //void sceneThreadTask();
};
 