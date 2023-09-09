#include "vulkan_functions.h"

namespace benpu { 
#define EXPORTED_VULKAN_FUNCTION( name ) PFN_##name name; 
#define GLOBAL_VULKAN_FUNCTION( name ) PFN_##name name; 
#define INSTANCE_VULKAN_FUNCTION( name ) PFN_##name name; 
#define INSTANCE_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name; 
#define DEVICE_VULKAN_FUNCTION( name ) PFN_##name name; 
#define DEVICE_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name; 
#include "vulkan_functions_list.inl" 
} // namespace benpu
