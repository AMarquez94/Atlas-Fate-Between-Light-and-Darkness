#include "mcv_platform.h"
#include "module_logic.h"
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
