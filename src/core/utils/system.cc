
#include <iostream>
#include <string>

#include "core/utils/plataform.h"
#include "core/utils/system.h"
#include "core/utils/configuration.h"


#ifdef BENPU_WINDOWS_PLATAFORM
#include <windows.h>
#include <shlobj.h>
#elif defined(BENPU_LINUX_PLATAFORM)

#endif

namespace benpu {

void System::printHelp() {
  std::cout << R"(
Benpu - )" << BENPU_VERSION_MAJOR << "." << BENPU_VERSION_MINOR << "." << BENPU_VERSION_PATCH << R"( - Ian Cisneros 2023

Usage: benpu [-c path] 

Arguments
	-c		Configuration path

)";
}

std::filesystem::path System::getDefaultConfigurationFile() {

#ifdef BENPU_WINDOWS_PLATAFORM
  PWSTR programDataPath = nullptr;  
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &programDataPath))) {
    std::wstring wstrPath(programDataPath);
    CoTaskMemFree(programDataPath);
    return std::filesystem::path(wstrPath) / "\\benpu\\config.json";
  }
#elif defined(BENPU_LINUX_PLATAFORM)
  char *homeDir = std::getenv("HOME");
  return std::filesystem::path(homeDir) / ".config/benpu/config.json";
#endif
}

}