#include "mcv_platform.h"
#include "module_logic.h"
//#include "modules/system/module_game_console.h"
#include <experimental/filesystem>


bool CModuleLogic::start() {
  BootLuaSLB();
  execEvent(Events::GAME_START);
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

  /* Classes */

  SLB::Class< CModuleGameConsole >("GameConsole", &m)
    .comment("This is our wrapper of the console class")
    .set("addCommand", &CModuleGameConsole::addCommandToList);

  SLB::Class< CModuleLogic >("Logic", &m)
    .comment("This is our wrapper of the logic class")
    .set("printLog", &CModuleLogic::printLog);


  /* Global functions */

  m.set("getConsole", SLB::FuncCall::create(&getConsole));
  m.set("getLogic", SLB::FuncCall::create(&getLogic));
}

CModuleLogic::ConsoleResult CModuleLogic::execScript(const std::string& script) {
  std::string scriptLogged = script;
  std::string errMsg = "";
  bool success = false;
  try {
    s.doString(script);
    scriptLogged = scriptLogged + " - Success";
    success = true;
  }
  catch (std::exception e) {
    scriptLogged = scriptLogged + " - Failed";
    scriptLogged = scriptLogged + "\n" + e.what();
    errMsg = e.what();
  }
  dbg("Script %s\n", scriptLogged.c_str());
  log.push_back(scriptLogged);
  return ConsoleResult{ success, errMsg };
}

bool CModuleLogic::execEvent(Events event, const std::string & params)
{
  switch (event) {
  case Events::GAME_START:
    execScript("onGameStart()");
    break;
  case Events::GAME_END:

    break;
  default:

    break;
  }
  return false;
}

void CModuleLogic::printLog()
{
  dbg("Printing log\n");
  for (int i = 0; i < log.size(); i++) {
    dbg("%s\n", log[i].c_str());
  }
  dbg("End printing log\n");
}

/* Auxiliar functions */
CModuleGameConsole * getConsole()
{
  return EngineConsole.getPointer();
}

CModuleLogic * getLogic()
{
  return EngineLogic.getPointer();
}
