////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_SHADERMODULE_HPP
#define ILLUSION_GRAPHICS_SHADERMODULE_HPP

#include "PipelineResource.hpp"
#include "fwd.hpp"

#include <set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderModule {
 public:
  static std::vector<uint32_t> compileGlsl(std::string const& glsl, vk::ShaderStageFlagBits stage);

  // Syntactic sugar to create a std::shared_ptr for this class
  template <typename... Args>
  static ShaderModulePtr create(Args&&... args) {
    return std::make_shared<ShaderModule>(args...);
  };

  ShaderModule(DevicePtr const& device, std::string const& glsl, vk::ShaderStageFlagBits stage,
      std::set<std::string> const& dynamicBuffers = {});

  ShaderModule(DevicePtr const& device, std::vector<uint32_t>&& spirv,
      vk::ShaderStageFlagBits stage, std::set<std::string> const& dynamicBuffers);

  virtual ~ShaderModule();

  vk::ShaderStageFlagBits              getStage() const;
  vk::ShaderModulePtr                  getModule() const;
  std::vector<PipelineResource> const& getResources() const;

 private:
  void createReflection(std::set<std::string> const& dynamicBuffers);

  std::vector<uint32_t>         mSpirv;
  vk::ShaderStageFlagBits       mStage;
  vk::ShaderModulePtr           mModule;
  std::vector<PipelineResource> mResources;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_SHADERMODULE_HPP
