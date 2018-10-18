#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "sound/fmod/fmod_studio.hpp"
#include "sound/fmod/fmod.hpp"
#include "sound/soundEvent.h"

// 3D Sound, use to play sounds on 3d scene at given point
// DSP, For filters, like a sound shader

// Create sound as a resource TO-DO

#define VEC3_TO_FMOD(VEC3) FMOD_VECTOR {VEC3.x, VEC3.y, VEC3.z};
#define FMOD_TO_VEC3(FMOD_VECTOR) VEC3(FMOD_VECTOR.x, FMOD_VECTOR.y, FMOD_VECTOR.z);

class CModuleSound : public IModule
{
    /* FMOD systems */
    FMOD::Studio::System *_system;
    FMOD::System* _lowlevelsystem;
    void *_extradriverdata = 0;

    /* FMOD banks, events, eventinstances and buses */
    std::unordered_map<std::string, FMOD::Studio::Bank*> myBanks;
    std::unordered_map<std::string, FMOD::Studio::EventDescription*> myEvents;
    std::unordered_map<unsigned int, FMOD::Studio::EventInstance*> myEventInstances;
    std::unordered_map<std::string, FMOD::Studio::Bus*> myBuses;

    /* ID for event instances (always incremental) */
    static unsigned int sNextID;

    CHandle h_listener;

    void registerAllSoundClipsInPath(char* path);
    const std::string getPlayingState(FMOD::Studio::EventInstance* ei);


protected:
    friend class SoundEvent;
    FMOD::Studio::EventInstance* getEventInstance(unsigned int id);

public:

    CModuleSound(const std::string& aname) : IModule(aname) { }

    virtual bool start() override;
    virtual void update(float delta) override;
    virtual void render() override;
    virtual bool stop() override;

    void loadBank(const std::string& name);
    void unloadBank(const std::string& name);
    void unloadAllBanks();

    SoundEvent playEvent(const std::string& name);

    CHandle getListener() { return h_listener; };
    void setListener(CHandle h_listener /*const CTransform& transform*/);

    float getBusVolume(const std::string& name) const;
    bool getBusPaused(const std::string& name) const;
    void setBusVolume(const std::string& name, float volume);
    void setBusPaused(const std::string& name, bool pause);

    CTransform getVirtual3DAttributes(const CTransform& soundTransform, CHandle reference = getEntityByName("The Player"));
    //CTransform getVirtual3DAttributes(const VEC3& soundPos, const QUAT& soundRot, CHandle reference = getEntityByName("The Player"));
};
