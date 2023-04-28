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
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    Camera* camera;
    VkDescriptorSet globalDescriptorSet;
    Map gameObjects;
    DescriptorSetLayout* globalDescriptorSetLayout;
    DescriptorPool* globalDescriptorPool;
    Sampler* sampler;
};

struct ObjectInfo
{
    Device* device;
    Sampler* sampler;
    DescriptorPool* descriptorPool;
};