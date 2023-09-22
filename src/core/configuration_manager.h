#ifndef BENPU_CONFIGURATION_MANAGER_H_
#define BENPU_CONFIGURATION_MANAGER_H_

#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "core/utils/args.h"


namespace benpu {

class ConfigurationManager {
public:

  static ConfigurationManager& getInstance(Args args) {
    static ConfigurationManager instance{args};
    return instance;
  }

  nlohmann::json operator()() {
    return configuration;
  }

private:
  nlohmann::json configuration;

  ConfigurationManager(Args args);

  ~ConfigurationManager() {}

  ConfigurationManager(const ConfigurationManager&) = delete;
  ConfigurationManager& operator=(const ConfigurationManager&) = delete;
};

}

#endif