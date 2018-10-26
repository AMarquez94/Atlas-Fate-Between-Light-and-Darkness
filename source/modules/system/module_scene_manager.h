#pragma once

#include "modules/module.h"

class SceneBloom {

public:
    bool enabled = false;
    int max_steps = 4;
    float global_distance = 1.f;
    float threshold_min = 0.53f;
    float threshold_max = 0.72f;
    float multiplier = 0.39f;
    VEC4 weights = VEC4(0.36f,0.52f,0.3f,0.72f);
    std::string rt_name = "Bloom";
};

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

    SceneBloom * bloom;

    std::vector<std::string> persistent_subscenes;
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
    void preloadOnlyScene(const std::string& sceneName);
    void removeSceneResources(const std::string& sceneName);
    bool loadScene(const std::string & name);
    bool loadPartialScene(const std::string & name);
    Scene* createScene(const std::string& name);
    Scene* getSceneByName(const std::string& name);
    Scene* getActiveScene();
    bool unLoadActiveScene(bool partial = false, const std::string& new_scene = "");
    void destroySceneResources(const std::string& sceneName);
    void changeGameState(const std::string& gamestate);

    std::string getDefaultSceneName();

private:
    Scene * _activeScene;
    Scene * _persistentScene;
    std::string _default_scene;
    std::map<std::string, Scene*> _scenes;

    std::string next_scene;

    void loadJsonScenes(const std::string filepath);
    void setActiveScene(Scene* scene);
};
 