#include "mcv_platform.h"
#include "module_file.h"
#include "resources/json_resource.h"

bool CModuleFile::start() {

    // Register the resource types
    Resources.registerResourceClass(getResourceClassOf<CJsonResource>());

    preloadResources(false);
    
    resource_thread = std::thread(&CModuleFile::resourceThreadMain, this);

    return true;
}

bool CModuleFile::stop() {

    ending_engine = true;
    condition_variable.notify_one();
    resource_thread.join();
    return true;
}

void CModuleFile::update(float delta) {
    if (areResourcesToLoad()) {
        int amountLoaded = 0;
        int i = 0;

        /*if (!Resources.resourceExists(resources_to_load[0])) {
            Resources.get(resources_to_load[0]);
        }
        resources_to_load.erase(resources_to_load.begin());*/

        for (i = 0; amountLoaded <= 20 && i < resources_to_load.size(); i++) {

            if (resources_to_load[i].substr(resources_to_load[i].find_last_of(".") + 1).compare("bin") != 0
                && !Resources.resourceExists(resources_to_load[i])) {
                const IResource* res = Resources.get(resources_to_load[i]);
                dbg("PRELOADED %s\n", res->getName().c_str());
                if (res->getClass()->class_name.compare("Textures")) {
                    amountLoaded += 5;
                }
                else if (res->getClass()->class_name.compare("Materials") == 0) {
                    amountLoaded += 20;
                }
                else {
                    amountLoaded += 1;
                }
            }
        }
        dbg("ITERACIONES %d\n", i);

        resources_to_load.erase(resources_to_load.begin(), resources_to_load.begin() + i);
    }
}

void CModuleFile::render() {

}

std::vector<char> CModuleFile::loadResourceFile(const std::string & name)
{
    resource_files_mutex.lock();
    std::vector<char> &resource_file = resource_files[name];
    if (resource_file.empty()) {

        /* file not in the map => load it */
        std::ifstream is(name, std::ifstream::binary);

        if (is) {
            is.seekg(0, is.end);
            int length = is.tellg();
            is.seekg(0, is.beg);

            resource_file.resize(length);
            is.read(resource_file.data(), length);
        }
    }

    std::vector<char> file = resource_files[name];
    resource_files_mutex.unlock();
    return file;
}

bool CModuleFile::removeResourceFile(const std::string & name)
{
    bool result = false;
    resource_files_mutex.lock();
    auto it = resource_files.find(name);
    if (result = it != resource_files.end()) {
        resource_files.erase(it);
    }
    resource_files_mutex.unlock();
    return result;
}

const std::vector<std::string> CModuleFile::getFileResourceVector(std::string filename)
{
    return resources_by_file[filename];
}

void CModuleFile::writeResourcesInFile(const std::string filename, const std::vector<std::string>& resources)
{
    std::ofstream file(filename, std::ofstream::out);
    for (int i = 0; i < resources.size(); i++) {
        file << resources[i];
        if (i != resources.size() - 1) {
            file << std::endl;
        }
    }
    file.close();
}

void CModuleFile::preloadResources(bool overwrite)
{
    const json& jboot = Resources.get("data/boot.json")->as<CJsonResource>()->getJson();
    std::vector<std::string> pending_resources_to_load;
    for (auto it = std::next(jboot.begin(), 1); it != jboot.end(); ++it) {
        std::string scene_name = "data/resource_list/" + it.key() + ".txt";
        std::ifstream read_file(scene_name);
        if (!read_file.is_open() || overwrite) {
            if (jboot[it.key()]["static_data"].count("navmesh")) {
                pending_resources_to_load.push_back(jboot[it.key()]["static_data"].value("navmesh", ""));
            }
            std::vector< std::string > groups_subscenes = jboot[it.key()]["scene_group"];
            for (int i = 0; i < groups_subscenes.size(); i++) {
                const json& j = Resources.get(groups_subscenes[i])->as<CJsonResource>()->getJson();
                pending_resources_to_load.push_back(groups_subscenes[i]);
                parseResourceScene(j, pending_resources_to_load);
            }
            writeResourcesInFile(scene_name, pending_resources_to_load);
        }
        else {
            std::string new_resource;
            while (std::getline(read_file, new_resource)) {
                pending_resources_to_load.push_back(new_resource);
            }
        }
        read_file.close();
        for (int i = 0; i < pending_resources_to_load.size(); i++) {
            addPendingResourceFile(pending_resources_to_load[i]);
        }

        resources_by_file[it.key()] = pending_resources_to_load;
        pending_resources_to_load.clear();
    }
}

const std::vector<std::string> CModuleFile::getResourcesByFile(const std::string filename)
{
    std::vector<std::string> resources;
    std::ifstream read_file(filename);
    if (read_file.is_open()) {
        std::string resource;
        while (std::getline(read_file, resource)) {
            resources.push_back(resource);
        }
    }
    return resources;
}

void CModuleFile::addPendingResourceFile(const std::string & resource, bool add)
{
    std::unique_lock<std::mutex> lck(pending_resource_files_mutex);
    pending_resource_files.push_back(std::make_pair(resource, add));
    condition_variable.notify_one();
}

void CModuleFile::addResourceToLoad(const std::string & resourceToLoad)
{
    resources_to_load.push_back(resourceToLoad);
}

void CModuleFile::addVectorResourceToLoad(const std::vector<std::string>& resourcesToLoad)
{
    resources_to_load.insert(std::end(resources_to_load), std::begin(resourcesToLoad), std::end(resourcesToLoad));
    //resources_to_load = resourcesToLoad;
}

const std::pair<const std::string, bool> CModuleFile::getFirstPendingResourceFile()
{
    std::unique_lock<std::mutex> lck(pending_resource_files_mutex);
    condition_variable.wait(lck, [this] { return ending_engine || pending_resource_files.size() > 0; });
    if (pending_resource_files.size() > 0 && !ending_engine) {
        const std::pair<const std::string, bool> temp = pending_resource_files[0];
        pending_resource_files.erase(pending_resource_files.begin());
        return temp;
    }
    else {
        return std::make_pair("", true);
    }
}

void CModuleFile::resourceThreadMain()
{
    while (!ending_engine) {
        const std::pair<std::string, bool> resource = getFirstPendingResourceFile();
        if (resource.first.compare("") != 0) {
            if (resource.second) {
                dbg("Adding resource %s\n", resource.first.c_str());
                loadResourceFile(resource.first);
            }
            else {
                dbg("Removing resource %s\n", resource.first.c_str());
                removeResourceFile(resource.first);
            }
            Sleep(1);
        }
    }
    dbg("Ending thread\n");
}



#pragma region parsers

void CModuleFile::parseResourceScene(const json& j, std::vector<std::string>& scene_resources) {

    assert(j.is_array());
    for (int i = 0; i < j.size(); i++) {

        auto& j_item = j[i];

        assert(j_item.is_object());

        int n_entities = j_item.count("entity");
        if (n_entities) {
            auto& j_entity = j_item["entity"];

            if (j_entity.count("prefab")) {

                std::string prefab_src = j_entity["prefab"];
                const json& j_prefab = Resources.get(prefab_src)->as<CJsonResource>()->getJson();

                if (prefab_src.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), prefab_src) == scene_resources.end()) {
                    scene_resources.push_back(prefab_src);
                }
                parseResourceScene(j_prefab, scene_resources);
            }
            else {
                if (j_entity.count("render")) {

                    /* Render parser (meshes and textures) */
                    auto& j_render = j_entity["render"];
                    for (auto& render_item = j_render.begin(); render_item != j_render.end(); render_item++) {
                        if (render_item.value().count("mesh")) {

                            /* Mesh */
                            std::string s_mesh = render_item.value().value("mesh", "");
                            if (s_mesh.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_mesh) == scene_resources.end()) {
                                scene_resources.push_back(s_mesh);
                            }
                        }
                        if (render_item.value().count("materials")) {

                            /* Material */
                            std::vector<std::string> j_materials = render_item.value()["materials"];
                            for (auto& material : j_materials) {
                                parseMaterial(material, scene_resources);
                            }
                        }
                    }
                }

                if (j_entity.count("instance")) {

                    /* Render parser (meshes and textures) */
                    auto& j_instance = j_entity["instance"];
                    //for (auto& instance_item = j_instance.begin(); instance_item != j_instance.end(); instance_item++) {
                    if (j_instance.count("mesh")) {
                        /* Mesh */
                        std::string s_mesh = j_instance.value("mesh", "");
                        if (s_mesh.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_mesh) == scene_resources.end()) {
                            scene_resources.push_back(s_mesh);
                        }
                    }
                    if (j_instance.count("materials")) {

                        /* Material */
                        std::vector<std::string> j_materials = j_instance["materials"];
                        for (auto& material : j_materials) {
                            parseMaterial(material, scene_resources);
                        }
                    }
                }
                if (j_entity.count("fade_controller")) {

                    /* Fade controller */
                    auto& j_fade = j_entity["fade_controller"];
                    if (j_fade.count("material")) {
                        const std::string material = j_fade.value("material", "");
                        parseMaterial(material, scene_resources);
                    }
                }

                if (j_entity.count("fsm")) {

                    /* FSM */
                    auto& j_fsm = j_entity["fsm"];
                    if (j_fsm.count("file")) {
                        const std::string file = j_fsm.value("file", "");
                        if (file.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), file) == scene_resources.end()) {
                            scene_resources.push_back(file);
                        }
                    }
                }

                if (j_entity.count("skeleton")) {

                    /* Skeleton */
                    auto& j_skel = j_entity["skeleton"];
                    if (j_skel.count("skeleton")) {
                        const std::string s_skeleton = j_skel.value("skeleton", "");
                        if (s_skeleton.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_skeleton) == scene_resources.end()) {
                            scene_resources.push_back(s_skeleton);
                        }
                    }
                }

                if (j_entity.count("collider")) {

                    /* Collider */
                    auto& j_col = j_entity["collider"];
                    if (j_col.count("name")) {
                        const std::string s_col = j_col.value("name", "");
                        if (s_col.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_col) == scene_resources.end()) {
                            scene_resources.push_back(s_col);
                        }
                    }
                }

                if (j_entity.count("particles")) {

                    /* Particles */
                    auto& j_particles = j_entity["particles"];
                    if (j_particles.count("cores")) {
                        const std::vector<std::string> j_cores = j_particles["cores"];
                        for (int i = 0; i < j_cores.size(); i++) {
                            if (j_cores[i].compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), j_cores[i]) == scene_resources.end()) {
                                scene_resources.push_back(j_cores[i]);
                            }
                        }
                    }
                }

                if (j_entity.count("curve_controller")) {

                    /* Curve controller */
                    auto& j_curve = j_entity["curve_controller"];
                    if (j_curve.count("curve")) {
                        const std::string s_curve = j_curve.value("curve", "");
                        if (s_curve.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_curve) == scene_resources.end()) {
                            scene_resources.push_back(s_curve);
                        }
                    }
                }

                if (j_entity.count("color_grading")) {
                    
                    /* Color grading */
                    auto& j_color_grading = j_entity["color_grading"];
                    if (j_color_grading.count("lut")) {
                        const std::string s_lut = j_color_grading.value("lut", "");
                        if (s_lut.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_lut) == scene_resources.end()) {
                            scene_resources.push_back(s_lut);
                        }
                    }
                }

                if (j_entity.count("animated_object_controller")) {

                    /* Rigid animations */
                    auto& j_rigid_anims = j_entity["animated_object_controller"];
                    if (j_rigid_anims.count("animations")) {
                        auto& j_anim = j_rigid_anims["animations"];
                        for (auto& anim_item = j_anim.begin(); anim_item != j_anim.end(); anim_item++) {
                            const std::string s_anim = anim_item.value().value("src", "");
                            if (s_anim.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), s_anim) == scene_resources.end()) {
                                scene_resources.push_back(s_anim);
                            }
                        }
                    }
                }
            }
        }
        if (j_item.count("instance_container")) {

            /* Instance containers */
            auto& j_instance_container = j_item["instance_container"];
            if (j_instance_container.count("instance_data")) {
                auto& j_instance_data = j_instance_container["instance_data"];
                if (j_instance_data.count("materials")) {
                    std::vector<std::string> j_materials = j_instance_data["materials"];
                    for (auto& material : j_materials) {
                        parseMaterial(material, scene_resources);
                    }
                }
            }
        }
    }
}

void CModuleFile::parseMaterial(const std::string & material_path, std::vector<std::string>& scene_resources)
{
    if (material_path.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), material_path) == scene_resources.end()) {
        const json& j_material = loadJson(material_path);

        if (j_material.count("mats")) {
            auto& j_mats = j_material["mats"];

            for (auto it = j_mats.begin(); it != j_mats.end(); it++) {
                parseMaterial(it.value(), scene_resources);
            }
        }

        /* Textures */
        if (j_material.count("textures")) {
            auto& j_textures = j_material["textures"];
            for (auto it = j_textures.begin(); it != j_textures.end(); it++) {
                std::string texture_name = it.value();

                if (texture_name.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), texture_name) == scene_resources.end()) {
                    scene_resources.push_back(texture_name);
                }
            }
        }

        if (j_material.count("mix_blend_weights")) {
            const std::string weights = j_material.value("mix_blend_weights", "");
            if (weights.compare("") != 0 && std::find(scene_resources.begin(), scene_resources.end(), weights) == scene_resources.end()) {
                scene_resources.push_back(weights);
            }
        }

        /* Material */
        scene_resources.push_back(material_path);
    }
}

#pragma endregion These are the preload scene parsers