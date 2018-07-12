#include "mcv_platform.h"
#include "module_sound.h"
#include <experimental/filesystem>

#pragma comment(lib, "fmod64_vc.lib" )

/* Load all sounds in given path and its subfolders */
void CModuleSound::registerAllSoundClipsInPath(char * path)
{
    try {
        if (std::experimental::filesystem::exists(path) && std::experimental::filesystem::is_directory(path)) {

            std::experimental::filesystem::recursive_directory_iterator iter(path);
            std::experimental::filesystem::recursive_directory_iterator end;

            while (iter != end) {
                std::string filePath = iter->path().string();
                std::string extension = iter->path().extension().string();
                if ((extension == ".wav" || extension == ".ogg") &&
                    !std::experimental::filesystem::is_directory(iter->path())) {
                    std::string fileName = iter->path().filename().string();
                    size_t lastindex = fileName.find_last_of(".");
                    std::string filenameWithoutExtension = fileName.substr(0, lastindex);
                    dbg("File : %s loaded\n", filePath.c_str());
                    registerClip(filenameWithoutExtension, filePath, FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE);   //TODO: Move to 3D instead of 2D
                }
                std::error_code ec;
                iter.increment(ec);
                if (ec) {
                    fatal("Error while accessing %s: %s\n", iter->path().string().c_str(), ec.message().c_str());
                }
            }
        }
    }
    catch (std::system_error & e) {
        fatal("Exception %s while loading scripts\n", e.what());
    }
}
// Just adding an ambient sound to the game for milestone 2
bool CModuleSound::start() {

	System_Create(&_system);
	_system->init(32, FMOD_INIT_NORMAL, _extradriverdata);

    registerAllSoundClipsInPath("data/sounds/soundclips");

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

void CModuleSound::render()
{
    if (ImGui::TreeNode("Sound")) {
        for (auto& clip : _clips) {
            ImGui::Text(clip.first.c_str());
            ImGui::SameLine();
            if (ImGui::Button(("Play " + clip.first).c_str())) {
                playSound2D(clip.first);
            }
        }
        for (auto& clip : _clips3d) {
            ImGui::Text(clip.first.c_str());
            ImGui::SameLine();
            if (ImGui::Button(("Play " + clip.first).c_str())) {

            }
        }
        ImGui::TreePop();
    }
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
    clip->tag = tag;

	_clips.insert(std::pair<std::string, SoundClip*>(tag, clip));
}

void CModuleSound::playSound2D(const std::string & tag)
{
    assert(_clips.find(tag) != _clips.end());
    _system->playSound(_clips[tag]->sound, 0, false, &_clips[tag]->channel);
}

//TO-DO: Borrar todo esto y implementarlo como lo haria una persona con dos dedos de frente
void CModuleSound::exeStepSound() {
	
	int index = (int)(((float)rand() / RAND_MAX) * 7) + 1;
    playSound2D("step" + std::to_string(index));
	float volume = 0.75f;
	if (EngineInput["btRun"].isPressed()) volume = 1.0f;
	if (EngineInput["btCrouch"].isPressed()) volume = 0.5f;
	_clips["step" + std::to_string(index)]->channel->setVolume(volume);
}

void CModuleSound::exeShootImpactSound()
{
    int index = (int)(((float)rand() / RAND_MAX) * 2) + 1;
    playSound2D("bullet_impact" + std::to_string(index));
}
