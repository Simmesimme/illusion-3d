////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/CommandBuffer.hpp>
#include <Illusion/Graphics/DisplayPass.hpp>
#include <Illusion/Graphics/Engine.hpp>
#include <Illusion/Graphics/GraphicsState.hpp>
#include <Illusion/Graphics/ShaderProgram.hpp>
#include <Illusion/Graphics/Window.hpp>

#include <thread>

int main(int argc, char* argv[]) {
  auto engine  = std::make_shared<Illusion::Graphics::Engine>("Triangle Demo");
  auto context = std::make_shared<Illusion::Graphics::Context>(engine->getPhysicalDevice());
  auto window  = std::make_shared<Illusion::Graphics::Window>(engine, context);
  window->open();

  auto shader = Illusion::Graphics::ShaderProgram::createFromGlslFiles(
    context,
    {{vk::ShaderStageFlagBits::eVertex, "data/shaders/Triangle.vert"},
     {vk::ShaderStageFlagBits::eFragment, "data/shaders/Triangle.frag"}});

  Illusion::Graphics::GraphicsState state;
  state.setShaderProgram(shader);
  state.addBlendAttachment({});
  state.addViewport({glm::vec2(0), glm::vec2(window->pSize.get()), 0.f, 1.f});
  state.addScissor({glm::ivec2(0), window->pSize.get()});

  window->pSize.onChange().connect([&state](glm::uvec2 const& size) {
    auto viewports       = state.getViewports();
    viewports[0].mExtend = size;
    state.setViewports(viewports);

    auto scissors       = state.getScissors();
    scissors[0].mExtend = size;
    state.setScissors(scissors);

    return true;
  });

  auto renderPass = window->getDisplayPass();

  while (!window->shouldClose()) {
    window->processInput();
    auto cmd      = renderPass->acquireCommandBuffer();
    auto pipeline = renderPass->getPipelineHandle(state);
    renderPass->begin(cmd);
    cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
    cmd->draw(3, 1, 0, 0);
    renderPass->end(cmd);
    renderPass->submitCommandBuffer(cmd);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  return 0;
}
