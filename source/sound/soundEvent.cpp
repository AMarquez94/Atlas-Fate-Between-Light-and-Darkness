#include "mcv_platform.h"
#include "soundEvent.h"
#include "sound/fmod/fmod.hpp"
#include "sound/fmod/fmod_studio.h"

SoundEvent::SoundEvent() {
    myID = 0;
}

SoundEvent::SoundEvent(unsigned int id) {
    myID = id;
}

bool SoundEvent::isValid() {
    return EngineSound.getEventInstance(myID) != nullptr;
}

void SoundEvent::restart() {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->start();
    }
}

void SoundEvent::stop(bool allowFadeOut) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        FMOD_STUDIO_STOP_MODE mode = allowFadeOut ? 
            FMOD_STUDIO_STOP_ALLOWFADEOUT :
            FMOD_STUDIO_STOP_IMMEDIATE;
        event->stop(mode);
    }
}

void SoundEvent::setPaused(bool pause) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->setPaused(pause);
    }
}

void SoundEvent::setVolume(float volume) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->setVolume(volume);
    }
}

void SoundEvent::setPitch(float pitch) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->setPitch(pitch);
    }
}

void SoundEvent::setParameter(const std::string& name, float value) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->setParameterValue(name.c_str(), value);
    }
}

bool SoundEvent::getPaused() const {
    bool isPaused = false;
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->getPaused(&isPaused);
    }
    return isPaused;
}

float SoundEvent::getVolume() const {
    float volume = 0.f;
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->getVolume(&volume);
    }
    return volume;
}

float SoundEvent::getPitch() const {
    float pitch = 0.f;
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->getPitch(&pitch);
    }
    return pitch;
}

float SoundEvent::getParameter(const std::string& name) {
    float value = 0.f;
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        event->getParameterValue(name.c_str(), &value);
    }
    return value;
}

bool SoundEvent::is3D() const {
    bool value = false;
    auto event = EngineSound.getEventInstance(myID);
    if (event) {

        // Get event description
        FMOD::Studio::EventDescription* ed = nullptr;
        event->getDescription(&ed);
        if (ed) {
            ed->is3D(&value);
        }
    }
    return value;
}

void SoundEvent::set3DAttributes(const CTransform& worldTrans) {
    auto event = EngineSound.getEventInstance(myID);
    if (event) {
        FMOD_3D_ATTRIBUTES attr;
        attr.position = VEC3_TO_FMOD(worldTrans.getPosition());
        attr.forward = VEC3_TO_FMOD(worldTrans.getFront());
        attr.up = VEC3_TO_FMOD(worldTrans.getUp());
        attr.velocity = VEC3_TO_FMOD(VEC3::Zero);
        event->set3DAttributes(&attr);
    }
}
