////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/PipelineReflection.hpp>
#include <Illusion/Graphics/RenderPass.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Texture.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

Illusion::Graphics::TexturePtr createBRDFLuT(Illusion::Graphics::DevicePtr const& device) {
  auto shader =
    Illusion::Graphics::ShaderProgram::createFromFiles(device, {"data/shaders/BRDFLuT.comp"});

  auto storageImage = Illusion::Graphics::Texture::create2D(device, 512, 512,
    vk::Format::eR32G32Sfloat, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
    vk::SamplerCreateInfo(vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eNearest, vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge));

  auto cmd =
    Illusion::Graphics::CommandBuffer::create(device, Illusion::Graphics::QueueType::eCompute);
  cmd->bindingState().setStorageImage(storageImage, 0, 0);

  cmd->begin();
  cmd->setShaderProgram(shader);
  cmd->dispatch(512 / 16, 512 / 16, 1);
  cmd->end();
  cmd->submit();
  cmd->waitIdle();

  return storageImage;
}

int main(int argc, char* argv[]) {

  Illusion::Core::Logger::enableTrace = true;

  auto engine = Illusion::Graphics::Engine::create("Physically Based Rendering Demo");
  auto device = Illusion::Graphics::Device::create(engine->getPhysicalDevice());
  auto window = Illusion::Graphics::Window::create(engine, device);

  Illusion::Graphics::TexturePtr brdflut = createBRDFLuT(device);

  auto shader = Illusion::Graphics::ShaderProgram::createFromFiles(
    device, {"data/shaders/TexturedQuad.vert", "data/shaders/TexturedQuad.frag"});

  auto renderPass = Illusion::Graphics::RenderPass::create(device);
  renderPass->addAttachment(vk::Format::eR8G8B8A8Unorm);
  renderPass->setExtent(window->pExtent.get());

  auto cmd = Illusion::Graphics::CommandBuffer::create(device);
  cmd->graphicsState().addBlendAttachment({});
  cmd->graphicsState().addViewport({glm::vec2(0), glm::vec2(window->pExtent.get()), 0.f, 1.f});
  cmd->graphicsState().addScissor({glm::ivec2(0), window->pExtent.get()});
  cmd->bindingState().setTexture(brdflut, 0, 0);
  cmd->begin();
  cmd->setShaderProgram(shader);
  cmd->beginRenderPass(renderPass);
  cmd->draw(4);
  cmd->endRenderPass();
  cmd->end();

  auto renderFinishedFence     = device->createFence();
  auto renderFinishedSemaphore = device->createSemaphore();

  window->open();
  while (!window->shouldClose()) {
    window->processInput();

    device->waitForFences(*renderFinishedFence, true, ~0);
    device->resetFences(*renderFinishedFence);

    cmd->submit({}, {}, {*renderFinishedSemaphore});

    window->present(
      renderPass->getFramebuffer()->getImages()[0], renderFinishedSemaphore, renderFinishedFence);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  device->waitIdle();

  return 0;
}