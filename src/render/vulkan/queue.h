#ifndef BENPU_QUEUE_H_
#define BENPU_QUEUE_H_

#include <optional>

#include <vulkan/vulkan.hpp>

#include "status_code.h"

namespace benpu {

struct QueueFamilyIndices{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  bool isComplete() const noexcept { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class Queue {
public:

  Queue(vk::Device& device);

  StatusCode initialize(uint32_t index);

private:
  vk::Device& device;
  vk::Queue queue = nullptr;
};

} //namespace benpu

#endif