#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup.hpp>

#include "core/configuration_manager.h"
#include "core/utils/configuration.h"
#include "core/utils/system.h"
#include "core/utils/args.h"
#include "render/vulkan/renderer.h"

static void init_log() {

  static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]:  %Message%");

  boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

  boost::log::add_console_log(
    std::cout,
    boost::log::keywords::format = COMMON_FMT,
    boost::log::keywords::auto_flush = true
  );

  boost::log::add_file_log (
    boost::log::keywords::file_name = "benpu_%3N.log",
    boost::log::keywords::rotation_size = 1 * 512 * 1024,
    boost::log::keywords::max_size = 10 * 1024 * 1024,
    boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
    boost::log::keywords::format = COMMON_FMT,
    boost::log::keywords::auto_flush = true,
    boost::log::keywords::format
  );

  boost::log::add_common_attributes();

#ifndef _DEBUG
  boost::log::core::get()->set_filter(
    boost::log::trivial::severity >= boost::log::trivial::info
  );
#endif

}

int main(int argc, char** argv) {
  
  init_log();

  benpu::Args args{argc, argv};

  if (!args.isCorrect()) {
    benpu::System::printHelp();
    return 1;
  }
  
  benpu::ConfigurationManager& conifgurationManager = benpu::ConfigurationManager::getInstance(&args);

  benpu::Renderer& renderer = benpu::Renderer::getInstance();

  if (renderer.getStatus() != benpu::Renderer::ok) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't initialize renderer.";
    return 1;
  }

  renderer.mainLoop();

  return 0;    
}
