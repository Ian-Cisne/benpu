
#include <set>

#include <boost/log/trivial.hpp>

#include "core/configuration_manager.h"
#include "render/vulkan/command_pool.h"
#include "render/vulkan/renderer.h"
#include "render/vulkan/swapchain.h"

namespace benpu {

Renderer& Renderer::getInstance() {
  static Renderer instance;
  return instance;
}

Renderer::Renderer():
  mainWindow{
    ConfigurationManager::getInstance()["screenSize"]["x"],
    ConfigurationManager::getInstance()["screenSize"]["y"]
  },
  pipeline(device),
  swapchain(device),
  renderPass(device),
  commandPool(device) {
  
  vkfw::init();

  if (createVulkanInstance() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create Vulkan instance.";
    status = ObjectStatus::error;
    return;
  }

  if (swapchain.setSurface(instance, mainWindow) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create windown surface.";
    status = ObjectStatus::error;
    return;
  }

  std::vector<const char*> requiredExtensions = { 
    VK_KHR_SWAPCHAIN_EXTENSION_NAME 
  };

  QueueFamilyIndices queueFamilyIndices;

  if(pickPhysicalDevice(queueFamilyIndices, requiredExtensions) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't pick a physical device.";
    status = ObjectStatus::error;
    return;
  }

  if(createDevice(queueFamilyIndices, requiredExtensions) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create logical device.";
    status = ObjectStatus::error;
    return;
  }

  if(swapchain.initialize(physicalDevice, queueFamilyIndices, mainWindow) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create swapchain.";
    status = ObjectStatus::error;
    return;
  }

  if(renderPass.initialize(swapchain.getFormat()) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create render pass.";
    status = ObjectStatus::error;
    return;
  }

  if(pipeline.initialize() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create graphical pipeline.";
    status = ObjectStatus::error;
    return;
  }

  if(swapchain.createFramebuffers(renderPass) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create frame buffers.";
    status = ObjectStatus::error;
    return;
  }

  if(commandPool.initialize(queueFamilyIndices) != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create command pool.";
    status = ObjectStatus::error;
    return;
  }

  if(commandPool.createCommandBuffer() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create command buffer.";
    status = ObjectStatus::error;
    return;
  }

  if(commandPool.createSyncObjects() != StatusCode::success) {
    BOOST_LOG_TRIVIAL(error) << "Couldn't create synchronization objects.";
    status = ObjectStatus::error;
    return;
  }

  status = ObjectStatus::ok;
}

StatusCode checkRequiredExtensions(const std::vector<const char*> &requiredExtensions) {
  BOOST_LOG_TRIVIAL(info) << "Checking required Extensions.";
  
  std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();
  for (auto requiredExtension : requiredExtensions) {
    bool extensionFound = false;
    for (const vk::ExtensionProperties availableExtension : availableExtensions) {
      if (strcmp(requiredExtension, availableExtension.extensionName.data()) == 0) {
        extensionFound = true;
        break;
      }
    }
    if (!extensionFound) {
      return StatusCode::extensionNotFound;
    }
  }
  return StatusCode::success;
}

StatusCode checkRequiredLayers(const std::vector<const char*> &requiredLayers) {
  std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
  for (const char* requiredLayer : requiredLayers) {
      bool layerFound = false;
    for (const vk::LayerProperties availableLayer : availableLayers) {
      if (strcmp(requiredLayer, availableLayer.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return StatusCode::layerNotFound;
    }
  }
  return StatusCode::success;
}

bool checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
  std::vector<vk::ExtensionProperties> availableExtension = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const vk::ExtensionProperties& extension : availableExtension) {
    requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}

int Renderer::getBestPhysicalDevice(const std::vector<vk::PhysicalDevice>& physicalDevices, QueueFamilyIndices& bestDeviceQueueFamilyIndices, const std::vector<const char*>& requiredExtensions) {

  int bestPhysicalDevicesIndex = -1;
  int bestScore = 0;


  for(int i = 0; i - physicalDevices.size(); ++i) {
    QueueFamilyIndices queueFamilyIndices;
    vk::PhysicalDeviceProperties deviceProperties = physicalDevices[i].getProperties();
    vk::PhysicalDeviceFeatures deviceFeatures = physicalDevices[i].getFeatures();
    
    std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevices[i].getQueueFamilyProperties();

    int score = 0;
    
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    } else if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 500;
    }
    
    uint32_t apiVersion = deviceProperties.apiVersion;
    if (VK_VERSION_MAJOR(apiVersion) >= 1 && VK_VERSION_MINOR(apiVersion) >= 2) {
        score += 500; 
    }
    
    uint32_t maxTextureSize = deviceProperties.limits.maxImageDimension2D;
    score += (maxTextureSize/4096) * 250;
    
    if (deviceFeatures.tessellationShader) {
        score += 100;
    }

    for (int j = 0; j < queueFamiliesProperties.size(); ++j) {
      
      if (queueFamiliesProperties[j].queueFlags & vk::QueueFlagBits::eGraphics) {
        queueFamilyIndices.graphicsFamily = j;
      }

      if (physicalDevices[i].getSurfaceSupportKHR(j, swapchain.getSurface())) {
        queueFamilyIndices.presentFamily = j;
      }

      if (queueFamilyIndices.isComplete()) {
        break;
      }

    }
    
    if (!deviceFeatures.geometryShader
      || !queueFamilyIndices.isComplete()
      || !checkDeviceExtensionSupport(physicalDevices[i], requiredExtensions)) {
        //If it does support the queue families we need, we can't use it.
        score = 0;
        continue; 
    }

    SwapChainSupportDetails swapChainSupport = swapchain.querySwapChainSupport(physicalDevices[i]);

    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
      score = 0;
    }
    
    if (score > bestScore) {
      bestScore = score;
      bestPhysicalDevicesIndex = i;
      bestDeviceQueueFamilyIndices = queueFamilyIndices;
      
    }
  }

  return bestPhysicalDevicesIndex;
}

StatusCode Renderer::pickPhysicalDevice(QueueFamilyIndices &queueFamilyIndices, const std::vector<const char*>& requiredExtensions) {
  auto availablePhysicalDevices = instance.enumeratePhysicalDevices();

  if (availablePhysicalDevices.size() == 0) {
    return StatusCode::noPhysicalDeviceFound;
  }

  int bestPhysicalDeviceIndex;
  bestPhysicalDeviceIndex = getBestPhysicalDevice(availablePhysicalDevices, queueFamilyIndices, requiredExtensions);

  if (bestPhysicalDeviceIndex == -1) {
    return StatusCode::noProprerPhysicalDeviceFound;
  }

  physicalDevice = availablePhysicalDevices[bestPhysicalDeviceIndex];

  return StatusCode::success;
}

StatusCode Renderer::createVulkanInstance() {
  try {
    BOOST_LOG_TRIVIAL(info) << "Creating Vulkan instance.";

    vk::ApplicationInfo appInfo(
      "Benpu",
      VK_MAKE_API_VERSION(0, 0, 1, 0),
      "Benpu engine",
      VK_MAKE_API_VERSION(0, 0, 1, 0),
      VK_API_VERSION_1_3
    );

    std::vector<const char *> requiredExtensions = Window::getRequiredVulkanExtensions();

    requiredExtensions.push_back("VK_KHR_portability_enumeration");

    if (checkRequiredExtensions(requiredExtensions) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "One or more of required extensions haven't been found.";
      return StatusCode::extensionNotFound;
    }

    std::vector<const char *> requiredLayers{
#ifndef NDEBUG 
      "VK_LAYER_KHRONOS_validation"
#endif
    };

    BOOST_LOG_TRIVIAL(info) << "Checking required layers.";

    if (checkRequiredLayers(requiredLayers) != StatusCode::success) {
      BOOST_LOG_TRIVIAL(error) << "One or more of required layers haven't been found.";
      return StatusCode::layerNotFound;
    }

    vk::InstanceCreateInfo instanceInfo (
      vk::InstanceCreateFlags{VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR | VK_KHR_portability_enumeration}, 
      &appInfo,
      static_cast<uint32_t>(requiredLayers.size()),
      requiredLayers.data(), static_cast<uint32_t>(requiredExtensions.size()),
      requiredExtensions.data()
    );

    BOOST_LOG_TRIVIAL(info) << "Creating instance.";

    instance = vk::createInstance(instanceInfo);

  } catch (vk::SystemError &e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Instance creation: " << e.what();
    return StatusCode::vulkanError;
  }
  return StatusCode::success;
}

StatusCode Renderer::createDevice(QueueFamilyIndices& queueFamilyIndices, const std::vector<const char*>& requiredExtensions) {
  try {

    BOOST_LOG_TRIVIAL(info) << "Creating logical device.";

    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueInfo({},queueFamilyIndices.graphicsFamily.value(), 1, &queuePriority);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(),
                                              queueFamilyIndices.presentFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies) {
      vk::DeviceQueueCreateInfo queueCreateInfo(
        {},
        queueFamily,
        1,
        &queuePriority
      );
      queueCreateInfos.push_back(queueCreateInfo);
    }
    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceInfo(
      {},
      static_cast<uint32_t>(queueCreateInfos.size()),
      queueCreateInfos.data(),
      {},
      {},
      static_cast<uint32_t>(requiredExtensions.size()),
      requiredExtensions.data(),
      &deviceFeatures
    );

    device = physicalDevice.createDevice(deviceInfo);
    

  } catch (vk::SystemError& e) {
    BOOST_LOG_TRIVIAL(error) << "Vulkan error ocurred while Device creation: " << e.what();
    return StatusCode::deviceCreationError;
  }
  return StatusCode::success;
}

void Renderer::mainLoop() {

  while (!mainWindow.shouldClose()) {
    mainWindow.pollEvents();
    //drawFrame();
  }
  device.waitIdle();
}

Renderer::~Renderer() {

}

} //namespace benpu