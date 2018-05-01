#include "mcv_platform.h"
#include "module_sound.h"

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
	_ambiance->channel->setVolume(0.1f);
}

bool CModuleSound::stop() {

	_ambiance->result = _system->close();
	_ambiance->result = _system->release();

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