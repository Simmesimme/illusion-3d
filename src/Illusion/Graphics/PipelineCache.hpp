////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_PIPELINE_CACHE_HPP
#define ILLUSION_GRAPHICS_PIPELINE_CACHE_HPP

// ---------------------------------------------------------------------------------------- includes
#include "fwd.hpp"

#include "../Core/BitHash.hpp"

#include <map>
#include <mutex>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
class PipelineCache {

 public:
  // -------------------------------------------------------------------------------- public methods
  PipelineCache(std::shared_ptr<Context> const& context);
  virtual ~PipelineCache();

  std::shared_ptr<vk::Pipeline> getPipelineHandle(
    GraphicsState const& graphicsState, vk::RenderPass const& renderpass, uint32_t subPass);

  void clear();

 private:
  // ------------------------------------------------------------------------------- private members
  std::shared_ptr<Context>                               mContext;
  std::map<Core::BitHash, std::shared_ptr<vk::Pipeline>> mCache;
  std::mutex                                             mMutex;
};
} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_PIPELINE_CACHE_HPP