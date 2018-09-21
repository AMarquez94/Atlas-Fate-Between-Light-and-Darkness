#include "mcv_platform.h"
#include "module_sound.h"
#include <experimental/filesystem>
#include "sound/fmod/fmod_errors.h"
#include "components/comp_tags.h"

#pragma comment(lib, "fmod64_vc.lib" )
#pragma comment(lib, "fmodstudio64_vc.lib")

/* Load all sounds in given path and its subfolders */
void CModuleSound::registerAllSoundClipsInPath(char * path)
{

    //TODO: Register all banks?
    
    //try {
    //    if (std::experimental::filesystem::exists(path) && std::experimental::filesystem::is_directory(path)) {

    //        std::experimental::filesystem::recursive_directory_iterator iter(path);
    //        std::experimental::filesystem::recursive_directory_iterator end;

    //        while (iter != end) {
    //            std::string filePath = iter->path().string();
    //            std::string extension = iter->path().extension().string();
    //            if ((extension == ".wav" || extension == ".ogg") &&
    //                !std::experimental::filesystem::is_directory(iter->path())) {
    //                std::string fileName = iter->path().filename().string();
    //                size_t lastindex = fileName.find_last_of(".");
    //                std::string filenameWithoutExtension = fileName.substr(0, lastindex);
    //                dbg("File : %s loaded\n", filePath.c_str());
    //                registerClip(filenameWithoutExtension, filePath, FMOD_LOOP_OFF | FMOD_2D | FMOD_CREATESAMPLE);   //TODO: Move to 3D instead of 2D
    //            }
    //            std::error_code ec;
    //            iter.increment(ec);
    //            if (ec) {
    //                fatal("Error while accessing %s: %s\n", iter->path().string().c_str(), ec.message().c_str());
    //            }
    //        }
    //    }
    //}
    //catch (std::system_error & e) {
    //    fatal("Exception %s while loading scripts\n", e.what());
    //}
}

const std::string CModuleSound::getPlayingState(FMOD::Studio::EventInstance* ei)
{
    FMOD_STUDIO_PLAYBACK_STATE ei_state;
    ei->getPlaybackState(&ei_state);

    std::string name;
    switch (ei_state) {
    case FMOD_STUDIO_PLAYBACK_PLAYING:
        name = "Playing";
        break;
    case FMOD_STUDIO_PLAYBACK_SUSTAINING:
        name = "Sustaining";
        break;
    case FMOD_STUDIO_PLAYBACK_STOPPED:
        name = "Stopped";
        break;
    case FMOD_STUDIO_PLAYBACK_STARTING:
        name = "Starting";
        break;
    case FMOD_STUDIO_PLAYBACK_STOPPING:
        name = "Stopping";
        break;
    }
    return name;
}

unsigned int CModuleSound::sNextID = 0;

// Just adding an ambient sound to the game for milestone 2
bool CModuleSound::start() {

    FMOD_RESULT result;

    result = FMOD::Studio::System::create(&_system);
    assert(result == FMOD_OK);
    result = _system->initialize(1024, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_3D_RIGHTHANDED, _extradriverdata);
    assert(result == FMOD_OK);
    if (result != FMOD_OK) {
        fatal("Failed to initialize FMOD system %s\n", FMOD_ErrorString(result));
        return false;
    }
    result = _system->getLowLevelSystem(&_lowlevelsystem);

    loadBank("data/fmod/Banks/Master Bank.strings.bank");
    loadBank("data/fmod/Banks/Master Bank.bank");

    return true;
}


bool CModuleSound::stop() {

    unloadAllBanks();

    if (_system) {
        _system->release();
    }

    return true;
}

void CModuleSound::update(float delta) {

    /* Look for finished event instances */
    std::vector<unsigned int> done;
    for (auto& iter : myEventInstances) {
        FMOD::Studio::EventInstance * e = iter.second;
        FMOD_STUDIO_PLAYBACK_STATE state;
        e->getPlaybackState(&state);
        if (state == FMOD_STUDIO_PLAYBACK_STOPPED) {
            e->release();
            done.emplace_back(iter.first);
        }
    }

    for (auto id : done) {
        myEventInstances.erase(id);
    }

    /* Set listener according to active camera */
    CHandle candidate_h_listener = EngineCameras.getCurrentCamera();
    if (candidate_h_listener.isValid()) {
        setListener(candidate_h_listener);
    }

    /* Update FMOD */
    _system->update();
}

void CModuleSound::render()
{
    if (ImGui::TreeNode("Sound")) {
        if (ImGui::TreeNode("Banks")) {
            for (auto bank : myBanks) {
                char path[512];
                bank.second->getPath(path, 512, nullptr);
                ImGui::Text(path);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Event Descriptions")) {
            int index = 0;
            for (auto ed : myEvents) {
                char path[512];
                ed.second->getPath(path, 512, nullptr);
                ImGui::Text(path);
                ImGui::SameLine();
                if (ImGui::Button(("Play " + std::to_string(index)).c_str())) {
                    playEvent(path);
                }
                index++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Event instances")) {
            int index = 0;
            for (auto ei : myEventInstances) {
                char path[512];
                FMOD::Studio::EventDescription *mydes;
                ei.second->getDescription(&mydes);
                mydes->getPath(path, 512, nullptr);
                ImGui::Text(path);
                ImGui::SameLine();
                ImGui::Text(" - %s", getPlayingState(ei.second).c_str());
                ImGui::SameLine();
                if (ImGui::Button(("Stop " + std::to_string(index)).c_str())) {
                    SoundEvent* test;   //TODO: Not working atm
                    FMOD_RESULT result = ei.second->getUserData((void**)&test);
                    test->stop();
                    //playEvent(path);
                }
                index++;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Buses")) {
            /* TODO: Not working as they should */
            int index = 0;
            for (auto bus : myBuses) {
                char path[512];
                bus.second->getPath(path, 512, nullptr);
                float busVol = 0.f;
                bool busPaused = false;
                bus.second->getVolume(&busVol);
                bus.second->getPaused(&busPaused);
                ImGui::Text(path);
                ImGui::SameLine();
                ImGui::Text("Vol: %f - Paused: %s", busVol, busPaused ? "Y" : "N");
                index++;
            }
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

void CModuleSound::loadBank(const std::string & name)
{
    // Avoid loading a bank twice
    if (myBanks.find(name) != myBanks.end()) {
        return;
    }

    // Load bank
    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result = _system->loadBankFile(name.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
    assert(result == FMOD_OK);

    const int maxPathLength = 512;
    if (result == FMOD_OK) {

        // Add bank to mybanks
        myBanks.emplace(name, bank);
        // Load non-streaming data
        bank->loadSampleData();
        // Get number of events in this bank
        int numEvents = 0;
        bank->getEventCount(&numEvents);

        if (numEvents > 0) {

            // Get list of event descriptions in this bank
            std::vector<FMOD::Studio::EventDescription*> events(numEvents);
            bank->getEventList(events.data(), numEvents, &numEvents);
            char eventName[maxPathLength];
            for (int i = 0; i < numEvents; i++) {
                FMOD::Studio::EventDescription* e = events[i];
                //get path of the event
                e->getPath(eventName, maxPathLength, nullptr);
                myEvents.emplace(eventName, e);
            }
        }

        // Get the number of buses in this bank
        int numBuses = 0;
        bank->getBusCount(&numBuses);
        if (numBuses > 0) {
            
            // Get list of buses in this bank
            std::vector<FMOD::Studio::Bus*> buses(numBuses);
            bank->getBusList(buses.data(), numBuses, &numBuses);
            char busName[maxPathLength];
            for (int i = 0; i < numBuses; i++) {
                FMOD::Studio::Bus* bus = buses[i];
                bus->getPath(busName, maxPathLength, nullptr);
                myBuses.emplace(busName, bus);
            }
        }
    }
}

void CModuleSound::unloadBank(const std::string & name)
{
    // Avoid unloading a no loaded bank
    auto iter = myBanks.find(name);
    if (iter == myBanks.end()) {
        return;
    }

    // First: Remove all events from this bank
    FMOD::Studio::Bank* bank = iter->second;
    int numEvents = 0;
    bank->getEventCount(&numEvents);
    const int maxPathLength = 512;

    if (numEvents > 0) {

        // Get event descriptions for this bank
        std::vector <FMOD::Studio::EventDescription*> events(numEvents);

        // Get list of events
        bank->getEventList(events.data(), numEvents, &numEvents);
        char eventName[maxPathLength];
        for (int i = 0; i < numEvents; i++) {
            FMOD::Studio::EventDescription* e = events[i];
            e->getPath(eventName, maxPathLength, nullptr);
            auto i_event = myEvents.find(eventName);
            if (i_event != myEvents.end()) {

                // Remove event
                myEvents.erase(i_event);
            }
        }
    }

    // Get the number of buses in this bank
    int numBuses = 0;
    bank->getBusCount(&numBuses);
    if (numBuses > 0) {

        // Get list of buses in this bank
        std::vector<FMOD::Studio::Bus*> buses(numBuses);
        bank->getBusList(buses.data(), numBuses, &numBuses);
        char busName[maxPathLength];
        for (int i = 0; i < numBuses; i++) {
            FMOD::Studio::Bus* bus = buses[i];
            bus->getPath(busName, maxPathLength, nullptr);
            auto i_bus = myBuses.find(busName);
            if (i_bus != myBuses.end()) {
                myBuses.erase(i_bus);
            }
        }
    }

    // Unload sample data and bank
    bank->unloadSampleData();
    bank->unload();

    // Remove from banks map
    myBanks.erase(iter);
}

void CModuleSound::unloadAllBanks()
{
    for (auto& iter : myBanks) {
        iter.second->unloadSampleData();
        iter.second->unload();
    }

    myBanks.clear();
    myEvents.clear();
}

SoundEvent CModuleSound::playEvent(const std::string & name)
{
    unsigned int retID = 0;
    auto iter = myEvents.find(name);
    SoundEvent soundEvent;
    if (iter != myEvents.end()) {

        /* Create instance of an event */
        FMOD::Studio::EventInstance* event = nullptr;
        iter->second->createInstance(&event);
        if (event) {

            /* Start the event instance */
            event->start();

            /* Get the next id and add it to map */
            sNextID++;
            retID = sNextID;
            myEventInstances.emplace(retID, event);
        }

        soundEvent = SoundEvent(retID);
        event->setUserData(&soundEvent);
    }
    else {
        soundEvent = SoundEvent(retID);
    }
    return soundEvent;
}



void CModuleSound::setListener(CHandle h_listener /*const CTransform & transform*/)
{
    FMOD_3D_ATTRIBUTES attr;
    
    this->h_listener = h_listener;
    CEntity* e_listener = h_listener;
    TCompTransform* listener_pos = e_listener->get<TCompTransform>();
    attr.position = VEC3_TO_FMOD(listener_pos->getPosition());
    attr.forward = VEC3_TO_FMOD(listener_pos->getFront());
    attr.up = VEC3_TO_FMOD(listener_pos->getUp());
    attr.velocity = VEC3_TO_FMOD(VEC3::Zero);
    _system->setListenerAttributes(0, &attr);
}

float CModuleSound::getBusVolume(const std::string & name) const
{
    float volume = 0.f;
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->getVolume(&volume);
    }
    return volume;
}

bool CModuleSound::getBusPaused(const std::string & name) const
{
    bool paused = false;
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->getPaused(&paused);
    }
    return paused;
}

void CModuleSound::setBusVolume(const std::string & name, float volume)
{
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->setVolume(volume);
    }
}

void CModuleSound::setBusPaused(const std::string & name, bool pause)
{
    const auto iter = myBuses.find(name);
    if (iter != myBuses.end()) {
        iter->second->setPaused(pause);
    }
}


CTransform CModuleSound::getVirtual3DAttributes(const CTransform& soundTransform, CHandle reference)
{
    if (h_listener.isValid()) {
        CEntity* e_listener = EngineSound.getListener();
        TCompTags* listener_tags = e_listener->get<TCompTags>();

        if (listener_tags && listener_tags->hasTag(getID("main_camera"))) {
            CEntity* e_reference = reference;
            TCompTransform* ppos = e_reference->get<TCompTransform>();
            TCompTransform* listenerPos = e_listener->get<TCompTransform>();

            float distance = VEC3::Distance(ppos->getPosition(), soundTransform.getPosition());
            VEC3 direction = (soundTransform.getPosition() - listenerPos->getPosition()).Normalized();
            VEC3 virtualPos = listenerPos->getPosition() + direction * distance;

            CTransform newTransform;
            newTransform.setPosition(virtualPos);
            newTransform.setRotation(soundTransform.getRotation());
            return newTransform;
        }
        else {
            return soundTransform;
        }
    }
    else {
        return soundTransform;
    }
}

//CTransform CModuleSound::getVirtual3DAttributes(const VEC3 & soundPos, const QUAT & soundRot, CHandle reference)
//{
//    CTransform newT;
//    newT.setPosition(soundPos);
//    newT.setRotation(soundRot);
//    return getVirtual3DAttributes(newT, reference);
//}

FMOD::Studio::EventInstance * CModuleSound::getEventInstance(unsigned int id)
{
    FMOD::Studio::EventInstance* event = nullptr;
    auto iter = myEventInstances.find(id);
    if (iter != myEventInstances.end())
    {
        event = iter->second;
    }
    return event;
}