#include "mcv_platform.h"
#include "resources_manager.h"

CResourceManager Resources;

void CResourceManager::registerResourceClass(const CResourceClass* new_class) {
    // Given obj can't be null
    assert(new_class);

    // There must be at least one extension 
    assert(!new_class->extensions.empty());

    // Register all extension with this handler
    for (auto& e : new_class->extensions) {
        // Can't repeat extension
        assert(resource_classes.find(e) == resource_classes.end());
        assert(!e.empty());
        resource_classes[e] = new_class;
    }
    resource_classes_by_file_change_priority.push_back(new_class);
}

void CResourceManager::onFileChanged(const std::string& filename) {
    // Scan each category in the order that were registered
    for (auto& rc : resource_classes_by_file_change_priority) {
        // Give the oportunity to each resource to reload/refresh if the file has changed
        for (auto& r : all_resources) {
            if (r.second->getClass() == rc)
                r.second->onFileChanged(filename);
        }
    }
}

const IResource* CResourceManager::get(const std::string& res_name) {

    // Buscar si lo tengo
    auto it = all_resources.find(res_name);

    // si esta lo devuelvo
    if (it != all_resources.end())
        return it->second;

    // si no .. lo creo...
    // Find extension of the name, will identify the type of resource
    std::string::size_type str_pos = res_name.find_last_of(".");
    if (str_pos == std::string::npos) {
        fatal("Can't identify extension in resource %s\n", res_name.c_str());
    }
    std::string extension = res_name.substr(str_pos);

    // Find an appropiate resource factory based on the extension
    auto it_res = resource_classes.find(extension);
    if (it_res == resource_classes.end()) {
        fatal("Invalid resource extension %s\n", res_name.c_str());
    }

    // Forward the creation to the resource class factory
    IResource* new_res = it_res->second->create(res_name);
    assert(new_res);

    // Set the name and class
    new_res->setNameAndClass(res_name, it_res->second);

    // Save it for the next time someone wants it
    registerResource(new_res);

    return new_res;
}

void CResourceManager::registerResource(IResource* new_res) {
    assert(new_res);
    assert(!new_res->getName().empty());
    assert(new_res->getClass());
    // The name must be unique
    //assert(all_resources.find(new_res->getName()) == all_resources.end());
    all_resources[new_res->getName()] = new_res;
}

void CResourceManager::destroyAll() {
    for (auto it : all_resources) {
        IResource* r = it.second;
        r->destroy();
    }
    all_resources.clear();
}

void CResourceManager::debugInMenu() {

    if (ImGui::TreeNode("Resources")) {

        // Scan each registered resource class
        for (auto it : resource_classes) {
            auto res_class = it.second;

            // If a resource class has several extensions, only show the first one
            if (it.second->extensions[0] != it.first)
                continue;

            // Open section Textures?
            if (ImGui::TreeNode(res_class->class_name.c_str())) {

                // Check all resources and show only those of the current resource class
                for (auto rit : all_resources) {
                    IResource* r = rit.second;
                    if (r->getClass() == res_class) {

                        // Does the user wants the details of this resource
                        if (ImGui::TreeNode(r->getName().c_str())) {
                            r->debugInMenu();
                            ImGui::TreePop();
                        }

                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}
