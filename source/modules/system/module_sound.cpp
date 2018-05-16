#include "mcv_platform.h"
#include "module_sound.h"

#pragma comment(lib, "fmod64_vc.lib" )

// Just adding an ambient sound to the game for milestone 2
bool CModuleSound::start() {

	System_Create(&_system);
	_system->init(32, FMOD_INIT_NORMAL, _extradriverdata);

	return true;
}

// To be replaced, hardcoded by now
void CModuleSound::setAmbientSound(const std::string & path) {

	if (_ambiance != nullptr) {
		_ambiance->sound->release();
	}

	_ambiance = new SoundClip();
	_ambiance->result = _system->createStream(path.c_str(), FMOD_LOOP_NORMAL | FMOD_2D, 0, &_ambiance->sound); // Carga de disco, decodificarse
	_ambiance->result = _system->playSound(_ambiance->sound, 0, false, &_ambiance->channel);
	_ambiance->channel->setVolume(0.5f);
}

bool CModuleSound::stop() {

	_system->close();
	_system->release();

	// Clear all the clips registered previously

	return true;
}

void CModuleSound::update(float delta) {

	// Update here with player data.
	//_system->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);

	// Update all the 3d sounds with it's new positions
	// Maybe we can mark those static ones to not be updating...
	//for (auto p : _clips3d) {
	//  //Use userdata to retrieve it's new transforms...
	//	p->channel->set3DAttributes(&pos, &vel); //move this somewhere else
	//}

	//_system->update();
}

void CModuleSound::registerClip(const std::string & tag, const std::string & source, FMOD_MODE mode = 0) {

	SoundClip * clip = new SoundClip();
	clip->result = _system->createStream(source.c_str(), mode, 0, &clip->sound); // Carga de disco, decodificarse
	clip->tag = tag;

	_clips.insert(std::pair<std::string, SoundClip*>(tag, clip));
}

void CModuleSound::registerClip3D(const std::string & tag, const std::string & source) {

	SoundClip3D * clip = new SoundClip3D();
	clip->result = _system->createStream(source.c_str(), FMOD_3D, 0, &clip->sound); // Carga de disco, decodificarse

	_clips.insert(std::pair<std::string, SoundClip*>(tag, clip));
}

//TO-DO: Borrar todo esto y implementarlo como lo haria una persona con dos dedos de frente
void CModuleSound::exeStepSound() {

	FMOD_RESULT       result;
	void             *extradriverdata = 0;
	FMOD::Sound      *sound;
	FMOD::Channel * channel = 0;
	
	int index = (int)(((float)rand() / RAND_MAX) * 7);
	switch (index) {
		case 0:
			result = _system->createSound("../bin/data/sounds/step1.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;

		case 1:
			result = _system->createSound("../bin/data/sounds/step2.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;	

		case 2:
			result = _system->createSound("../bin/data/sounds/step3.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;

		case 3:
			result = _system->createSound("../bin/data/sounds/step2.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;

		case 4:
			result = _system->createSound("../bin/data/sounds/step5.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;

		case 5:
			result = _system->createSound("../bin/data/sounds/step6.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;

		case 6:
			result = _system->createSound("../bin/data/sounds/step7.ogg", FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE, 0, &sound);
			result = _system->playSound(sound, 0, false, &channel);
			break;
	}	
	float volume = 0.75f;
	if (EngineInput["btRun"].isPressed()) volume = 1.0f;
	if (EngineInput["btCrouch"].isPressed()) volume = 0.5f;
	channel->setVolume(volume);
	

}