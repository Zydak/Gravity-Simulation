#pragma once

#include "camera.h"
#include "object.h"
#include "vulkan/descriptors.h"
#include "vulkan/sampler.h"

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <memory>

using Map = std::unordered_map<int, std::shared_ptr<Object>>;

struct FrameInfo
{
    glm::mat4 skyboxTransform;
    float frameTime;
    glm::dvec3 offset;
    VkCommandBuffer commandBuffer;
    Camera* camera;
    VkDescriptorSet globalDescriptorSet;
    Map gameObjects;
};