#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup.hpp>

#include "configuration.h"
#include "video_manager.h"

static void init_log() {
  static const std::string COMMON_FMT("[%TimeStamp%][%Severity%]:  %Message%");

  boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");

  boost::log::add_console_log(
      std::cout,
      boost::log::keywords::format = COMMON_FMT,
      boost::log::keywords::auto_flush = true
      );

  boost::log::add_file_log (
      boost::log::keywords::file_name = "output_%3N.log",
      boost::log::keywords::rotation_size = 1 * 512 * 1024,
      boost::log::keywords::max_size = 10 * 1024 * 1024,
      boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
      boost::log::keywords::format = COMMON_FMT,
      boost::log::keywords::auto_flush = true
      );

  boost::log::add_common_attributes();
#ifndef _DEBUG
  boost::log::core::get()->set_filter(
      boost::log::trivial::severity >= boost::log::trivial::info
      );
#endif

}

int main() {
  init_log();

  benpu::mVideoManager.setUp();

  BOOST_LOG_TRIVIAL(info) << "Video setted up.";

  benpu::mVideoManager.run();


  benpu::mVideoManager.dismantle();

  return 0;    
}
