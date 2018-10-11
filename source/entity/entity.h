#pragma once

#include "handle/handle.h"
#include "components/comp_base.h"

class CEntity : public TCompBase {

    CHandle comps[CHandle::max_types];

public:
    ~CEntity();

    CHandle get(uint32_t comp_type) const {
        assert(comp_type < CHandle::max_types);
        return comps[comp_type];
    }

    CHandle getCompByName(const char * comp_name);

    template< typename TComp >
    CHandle get() const {
        auto om = getObjectManager<TComp>();
        assert(om);
        return comps[om->getType()];
    }

    void debugInMenu();
    void renderDebug();

    void set(uint32_t comp_type, CHandle new_comp);
    void load(const json& j, TEntityParseContext& ctx);

    const char* getName() const;

    template< class TMsg >
    void sendMsg(const TMsg& msg) {
        auto range = all_registered_msgs.equal_range(TMsg::getMsgID());
        while (range.first != range.second) {
            const auto& slot = range.first->second;

            // Si YO como entidad tengo ese component activo...
            CHandle h_comp = comps[slot.comp_type];
            if (h_comp.isValid())
                slot.callback->sendMsg(h_comp, &msg);

            ++range.first;
        }
    }


};


