#include "video_manager.h"
#include "status_code.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#if defined _WIN32 
#include <windows.h>
#elif defined __linux 
#include <dlfcn.h>
#endif 

#include "vulkan_functions.h"


namespace benpu {

void VideoManager::setUp() {
  BOOST_LOG_TRIVIAL(info) << "Loading Vulkan Runtime Library.";
  if (loadVulkanLibrary() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Failed to load Vulkan Runtime Library.";
  }

  BOOST_LOG_TRIVIAL(info) << "Loading vkGetInstanceProcAddr.";
  if (loadExportedVulkanFunction() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Could not load exported Vulkan function \"vkGetInstanceProcAddr\"\n";      
  }

  BOOST_LOG_TRIVIAL(info) << "Loading global-level Vulkan functions.";
  if (loadExportedVulkanFunction() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Failed to load Vulkan global-level functions."; 
  }



}


StatusCode VideoManager::loadVulkanLibrary(){
#if defined _WIN32 
  vulkanLibrary = LoadLibrary( "vulkan-1.dll" ); 
#elif defined __linux 
  vulkanLibrary = dlopen( "libvulkan.so.1", RTLD_NOW ); 
#endif 
  if (vulkanLibrary == nullptr) {
    return StatusCode::vulkanLibraryLoadError;
  }
  return StatusCode::success;
}

StatusCode VideoManager::loadExportedVulkanFunction() {

#if defined _WIN32 
  auto loadFunction = GetProcAddress;
#elif defined __linux 
  auto loadFunction = dlsym;
#endif 

#define EXPORTED_VULKAN_FUNCTION( name )                              \
  name = (PFN_##name)loadFunction( vulkanLibrary, #name );            \
  if( name == nullptr ) {                                             \
    return StatusCode::vulkanExportedFunctionLoadError;               \
  }

#include "vulkan_functions_list.inl"

  return StatusCode::success;
}

StatusCode VideoManager::loadGlobalVulkanFunction() {
  bool error = false;
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                          \
  name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name );         \
  if( name == nullptr ) {                                             \
    BOOST_LOG_TRIVIAL(error) <<                                       \
    "Failed to load global-level function \"" #name "\"\n";           \
    error = true;                                                     \
  } 
#include "vulkan_functions_list.inl" 

  return !error
    ? StatusCode::success
    : StatusCode::vlukanGlobalFunctionLoadError;
}
void VideoManager::dismantle() {

}

VideoManager mVideoManager;

} // namespace benpu
