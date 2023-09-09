#include <vulkan/vulkan.h> 

namespace benpu { 

#define EXPORTED_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define GLOBAL_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define INSTANCE_VULKAN_FUNCTION( name ) extern PFN_##name  name; 
#define INSTANCE_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name; 
#define DEVICE_VULKAN_FUNCTION( name ) extern PFN_##name name; 
#define DEVICE_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) extern PFN_##name name; 
#include "vulkan_functions_list.inl" 

} // namespace benpu 
