////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Lights.hpp"

#include <Illusion/Graphics/CoherentBuffer.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Device.hpp>
#include <Illusion/Graphics/Shader.hpp>

#include <glm/gtc/constants.hpp>
#include <random>

////////////////////////////////////////////////////////////////////////////////////////////////////

Lights::Lights(Illusion::Graphics::DeviceConstPtr const& device,
    Illusion::Graphics::FrameResourceIndexPtr const& frameIndex, uint32_t lightCount)
    : mLights(lightCount)
    , mDrawLights(Illusion::Graphics::Shader::createFromFiles("DrawLights", device,
          {"data/DeferredRendering/shaders/Light.vert",
              "data/DeferredRendering/shaders/Light.frag"}))
    , mDoShading(Illusion::Graphics::Shader::createFromFiles("DoShading", device,
          {"data/DeferredRendering/shaders/Quad.vert",
              "data/DeferredRendering/shaders/Shading.frag"}))
    , mLightBuffer(frameIndex, [=](uint32_t index) {
      return Illusion::Graphics::CoherentBuffer::create(
          "LightStorageBuffer " + std::to_string(index), device, sizeof(Light) * lightCount,
          vk::BufferUsageFlagBits::eStorageBuffer);
    }) {
  // clang-format off
  const std::array<glm::vec3, 12> POSITIONS = {
    glm::vec3( 0.000000, -1.000000,  0.000000), glm::vec3( 0.723600, -0.447215,  0.525720),
    glm::vec3(-0.276385, -0.447215,  0.850640), glm::vec3(-0.894425, -0.447215,  0.000000),
    glm::vec3(-0.276385, -0.447215, -0.850640), glm::vec3( 0.723600, -0.447215, -0.525720),
    glm::vec3( 0.276385,  0.447215,  0.850640), glm::vec3(-0.723600,  0.447215,  0.525720),
    glm::vec3(-0.723600,  0.447215, -0.525720), glm::vec3( 0.276385,  0.447215, -0.850640),
    glm::vec3( 0.894425,  0.447215,  0.000000), glm::vec3( 0.000000,  1.000000,  0.000000)};

  const std::array<uint32_t, 60> INDICES = {
    0, 1, 2, 1, 0, 5, 0, 2, 3, 0, 3, 4, 0, 4, 5, 1, 5, 10, 2, 1, 6,
    3, 2, 7, 4, 3, 8, 5, 4, 9, 1, 10, 6, 2, 6, 7, 3, 7, 8, 4, 8, 9,
    5, 9, 10, 6, 10, 11, 7, 6, 11, 8, 7, 11, 9, 8, 11, 10, 9, 11
  };
  // clang-format on

  mPositionBuffer = device->createVertexBuffer("SpherePositions", POSITIONS);
  mIndexBuffer    = device->createIndexBuffer("SphereIndices", INDICES);

  std::default_random_engine            generator;
  std::uniform_real_distribution<float> position(0.f, 2.f);
  std::uniform_real_distribution<float> color(0.5f, 1.f);

  for (auto& light : mLights) {
    float dir         = position(generator) * glm::pi<float>();
    float dist        = position(generator) * 2;
    light.mPosition.x = std::sin(dir) * dist;
    light.mPosition.y = 0.f;
    light.mPosition.z = std::cos(dir) * dist;
    light.mPosition.w = 1.f;
    light.mColor.r    = color(generator);
    light.mColor.g    = color(generator);
    light.mColor.b    = color(generator);
    light.mColor.a    = 1.f;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Lights::update(float time, glm::mat4 const& matVP) {
  mMatVP = matVP;
  for (size_t i(0); i < mLights.size(); ++i) {
    mLights[i].mPosition.y = std::sin(time + i) * 0.9;
  }

  mLightBuffer.current()->updateData(
      reinterpret_cast<uint8_t*>(&mLights[0]), sizeof(Light) * mLights.size(), 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Lights::draw(Illusion::Graphics::CommandBufferPtr const& cmd) {
  // The indices are provided as a triangle list
  cmd->graphicsState().setTopology(vk::PrimitiveTopology::eTriangleList);

  // Here we define what kind of vertex buffers will be bound. The vertex data (positions, normals
  // and texture coordinates) actually comes from three different vertex buffer objects.
  cmd->graphicsState().setVertexInputBindings(
      {{0, sizeof(glm::vec3), vk::VertexInputRate::eVertex}});

  // Here we define which vertex attribute comes from which vertex buffer.
  cmd->graphicsState().setVertexInputAttributes({{0, 0, vk::Format::eR32G32B32Sfloat, 0}});

  // Bind the three vertex buffers and the index buffer.
  cmd->bindVertexBuffers(0, {mPositionBuffer});
  cmd->bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint32);

  cmd->bindingState().setStorageBuffer(
      mLightBuffer.current()->getBuffer(), sizeof(Light) * mLights.size(), 0, 0, 0);

  cmd->setShader(mDrawLights);
  cmd->pushConstants(mMatVP);

  cmd->drawIndexed(60, mLights.size(), 0, 0, 0);

  cmd->bindingState().reset(0);
  cmd->graphicsState().setVertexInputBindings({});
  cmd->graphicsState().setVertexInputAttributes({});
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Lights::doShading(Illusion::Graphics::CommandBufferPtr const& cmd,
    std::vector<Illusion::Graphics::BackedImagePtr> const&         inputAttachments) {
  cmd->setShader(mDoShading);

  for (size_t i(0); i < inputAttachments.size(); ++i) {
    cmd->bindingState().setInputAttachment(inputAttachments[i], 0, i);
  }

  cmd->bindingState().setStorageBuffer(
      mLightBuffer.current()->getBuffer(), sizeof(Light) * mLights.size(), 0, 1, 0);
  cmd->specialisationState().setIntegerConstant(0, mLights.size());
  cmd->pushConstants(glm::inverse(mMatVP));

  cmd->draw(3);
  cmd->bindingState().reset(0);
  cmd->bindingState().reset(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
