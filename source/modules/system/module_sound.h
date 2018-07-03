#pragma once

#include "modules/module.h"
#include "entity/entity.h"
#include "fmod/fmod.hpp"
 
// 3D Sound, use to play sounds on 3d scene at given point
// DSP, For filters, like a sound shader

// Create sound as a resource TO-DO

class CModuleSound : public IModule
{
	// To be extended, move this to another file
	struct SoundClip {

		std::string tag;
		FMOD::Sound *sound;
		FMOD::Channel * channel = 0;

		FMOD_RESULT result;
	};

	struct SoundClip3D: public SoundClip {
		
		FMOD_VECTOR pos;
		FMOD_VECTOR velocity;
		FMOD_VECTOR up;
		FMOD_VECTOR forward;

		void * user_data;
	};

	SoundClip * _ambiance = nullptr;

	std::map<std::string, SoundClip*> _clips;
	std::map<std::string, SoundClip3D*> _clips3d;

	// To be replaced
	FMOD::System *_system;
	void *_extradriverdata = 0;

    void registerAllSoundClipsInPath(char* path);

public:

	CModuleSound(const std::string& aname) : IModule(aname) { }

	virtual bool start() override;
	virtual void update(float delta) override;
    virtual void render() override;
	virtual bool stop() override;

	void setAmbientSound(const std::string & path);
	void registerClip(const std::string & tag, const std::string & source, FMOD_MODE mode);
	void registerClip3D(const std::string & tag, const std::string & source);
    void playSound2D(const std::string& tag);
	void exeStepSound();
    void exeShootImpactSound();
	//void registerEvent(const std::string & tag, const std::string & source);
};
