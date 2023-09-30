#ifndef BENPU_COMMAND_POOL_H_
#define BENPU_COMMAND_POOL_H_

#include <vulkan/vulkan.hpp>

#include "status_code.h"
#include "render/vulkan/queue.h"

namespace benpu {

class CommandPool {
public:
  CommandPool(vk::Device& device);

  StatusCode initialize(const QueueFamilyIndices& queueFamilyIndices);
  StatusCode createCommandBuffer();
  StatusCode createSyncObjects();

private:
  vk::Device& device;
  vk::CommandPool commandPool = nullptr;
  vk::CommandBuffer commandBuffer = nullptr;
  vk::Semaphore imageAvailableSemaphore = nullptr;
  vk::Semaphore renderFinishedSemaphore = nullptr;
  vk::Fence inFlightFence ;

private:
  
};

} //namespace benpu

#endif