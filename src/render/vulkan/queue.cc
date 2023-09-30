
#include <boost/log/trivial.hpp>

#include "render/vulkan/queue.h"

namespace benpu {

  Queue::Queue(vk::Device& device): device{device} {}

  StatusCode Queue::initialize(uint32_t index) {
    try {
      device.getQueue(index, 0, &queue);
    } catch (vk::SystemError& e) {

      BOOST_LOG_TRIVIAL(error) << "Couldn't create queue.";
      return StatusCode::queueCreationError;
    }
    return StatusCode::success;
  }

} //namespace benpu
