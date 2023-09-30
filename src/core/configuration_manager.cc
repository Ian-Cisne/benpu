
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/log/trivial.hpp>

#include "core/utils/configuration.h"
#include "core/configuration_manager.h"
#include "core/utils/args.h"
#include "core/utils/system.h"

namespace benpu {

ConfigurationManager& ConfigurationManager::getInstance(Args* args) {
  static ConfigurationManager instance{*args};
  return instance;
}

ConfigurationManager::ConfigurationManager(Args& args) {

  std::filesystem::path configurationFilePath = std::filesystem::path(args.values[ArgType::configurationFile]);

  bool writeConfigurationFile = false;

  if(!std::filesystem::exists(configurationFilePath)) {
    configurationFilePath = System::getDefaultConfigurationFile();
  }

  std::filesystem::create_directories(configurationFilePath.parent_path());

  try {

    std::ifstream configFile(configurationFilePath);

    if (configFile.is_open()) {
      configFile >> configuration;
      return;
    } else {
      writeConfigurationFile = true;
      BOOST_LOG_TRIVIAL(warning) << "Couldn't open configuration file.";
    }

  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(warning) << "Couldn't read configuration file.";
  }


  std::stringstream defaultConfiguration(R"(
{
  "screenSize": {
    "x": 800,
    "y": 600
  }
}
  )");

  BOOST_LOG_TRIVIAL(info) << "Loading default configuration.";
  defaultConfiguration >> configuration;

  if (writeConfigurationFile) {
    BOOST_LOG_TRIVIAL(info) << "Saving configuration.";
      
    try {
      std::ofstream configFile(configurationFilePath);
      if (configFile.is_open()) {
        configFile << configuration;
        return;
      } else {
        BOOST_LOG_TRIVIAL(warning) << "Couldn't create configuration file.";
      }
    } catch (const std::exception& e) {
      BOOST_LOG_TRIVIAL(warning) << "Couldn't write configuration file.";
    }
  }
}

nlohmann::json ConfigurationManager::operator[](std::string a) {
  return configuration[a];
}

} // namespace benpu