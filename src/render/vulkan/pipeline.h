#ifndef BENPU_PIPELINE_H_
#define BENPU_PIPELINE_H_

#include <vulkan/vulkan.hpp>

#include "status_code.h"

namespace benpu {

class Pipeline {
public:

  Pipeline(vk::Device& device);
  
  StatusCode initialize();

private:
  vk::Device& device;
  vk::ShaderModule fragmentShaderModule = nullptr;
  vk::ShaderModule vertexShaderModule = nullptr;
  vk::PipelineLayout pipelineLayout = nullptr;
  vk::RenderPass renderPass = nullptr;
  vk::Pipeline graphicsPipeline = nullptr;

private:
  StatusCode createShaderModule(const std::vector<char>& code, vk::ShaderModule& shaderModule);
};

} //namespace benpu

#endif