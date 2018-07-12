#ifndef INC_RENDER_MANAGER_H_
#define INC_RENDER_MANAGER_H_

#include "entity/entity.h"

class CTexture;
class CRenderMesh;
class CMaterial;

class CRenderManager {

public:

    struct TRenderKey {
        CHandle                h_render_owner;     // Propietario de esta key
        CHandle                h_transform;        // sibling component transform handle
        CHandle                h_aabb;
        uint32_t               subgroup_idx = 0;
        const CRenderMesh*     mesh = nullptr;     // Qué mesh hay que pintar
        const CMaterial*       material = nullptr;

        void debugInMenu();
    };

    static bool sortRenderKeys(const TRenderKey& k1, const TRenderKey& k2);

private:

    // ---------------------------------------------------------
    template < typename TClassKey >
    struct TContainerKeys : public std::vector< TClassKey > {

        void deleteByOwner(CHandle h_owner) {
            auto it = begin();
            while (it != end()) {
                if (it->h_render_owner == h_owner) {
                    it = this->erase(it);
                }
                else
                    ++it;
            }
        }

        void addKey(TClassKey& new_key) {
            push_back(new_key);
            is_dirty = true;
        }

        void sortIfRequired() {
            if (!is_dirty)
                return;

            std::sort(begin(), end(), sortRenderKeys);

            bool ownerHasBeenDeleted = false;
            for (auto& key : (*this)) {
                // Cache the handle of the component transform of my entity

                if (key.h_render_owner.getOwner().isValid()) {
                    CEntity* e_owner = key.h_render_owner.getOwner();
                    assert(e_owner);
                    key.h_transform = e_owner->get< TCompTransform >();
                }
                else {
                    ownerHasBeenDeleted = true;
                }
            }

            if (!ownerHasBeenDeleted) {
                is_dirty = false;
            }
        }

        bool is_dirty = false;
    };

    TContainerKeys<TRenderKey> render_keys;
    CHandle                    h_camera;

public:

    void delRenderKeys(CHandle h_owner);
    void addRenderKey(CHandle h_comp_render_owner
        , const CRenderMesh* mesh
        , const CMaterial* material
        , uint32_t subgroup_idx
    );

    void renderCategory(const char* category_name);
    void debugInMenu();
    void setEntityCamera(CHandle h_new_entity_camera);
    void forceDirty();
    static CRenderManager& get();

};

#endif

