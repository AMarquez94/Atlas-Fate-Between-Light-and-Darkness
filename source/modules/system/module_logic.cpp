#include "mcv_platform.h"
#include "module_logic.h"
#include "modules/game/module_game_console.h"
#include <experimental/filesystem>


bool CModuleLogic::start() {
  BootLuaSLB();
  return true;
}

bool CModuleLogic::stop() {

  return true;
}

void CModuleLogic::update(float delta) {

}

/* Where we publish all functions that we want and load all the scripts in the scripts folder */
void CModuleLogic::BootLuaSLB()
{
  //Publish all the functions
  publishClasses();
  //Load all the scripts
  loadScriptsInFolder("data/scripts");
}

/* Load all scripts.lua in given path and its subfolders */
void CModuleLogic::loadScriptsInFolder(char * path)
{
  try {
    if (std::experimental::filesystem::exists(path) && std::experimental::filesystem::is_directory(path)) {

      std::experimental::filesystem::recursive_directory_iterator iter(path);
      std::experimental::filesystem::recursive_directory_iterator end;

      while (iter != end) {
        std::string fileName = iter->path().string();
        if (fileName.substr(fileName.find_last_of(".") + 1) == "lua" && 
          !std::experimental::filesystem::is_directory(iter->path())) {
            dbg("File : %s loaded\n", fileName.c_str());
            s.doFile(fileName);
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

/* Publish all the classes in LUA */
void CModuleLogic::publishClasses() {
  SLB::Class< CModuleGameConsole >("GameConsole", &m)
    // a comment/documentation for the class [optional]
    .comment("This is our wrapper of the console class")
    // empty constructor, we can also wrapper constructors
    // with arguments using .constructor<TypeArg1,TypeArg2,..>()
    // a method/function/value...
    .constructor<const std::string&>()
    .set("expandConsole", &CModuleGameConsole::expand)
    .set("contractConsole", &CModuleGameConsole::contract);
}

bool CModuleLogic::execScript(const std::string& script) {
  std::string scriptLogged = script;
  bool success = false;
  try {
    s.doString(script);
    scriptLogged = scriptLogged + " - Success";
    success = true;
  }
  catch (std::exception e) {
    scriptLogged = scriptLogged + " - Failed";
  }
  log.push_back(scriptLogged);
  return success;
}
