////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
#define ILLUSION_GRAPHICS_RENDER_GRAPH_HPP

#include "../Core/NamedObject.hpp"
#include "../Core/ThreadPool.hpp"
#include "FrameResource.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <list>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class FrameGraph : public Core::StaticCreate<FrameGraph>, public Core::NamedObject {
 public:
  // -----------------------------------------------------------------------------------------------
  enum class ResourceSizing { eAbsolute, eRelative };

  enum class ResourceAccess {
    eReadOnly,
    eWriteOnly,
    eReadWrite,
    eLoad,
    eLoadWrite,
    eLoadReadWrite
  };

  // -----------------------------------------------------------------------------------------------
  class LogicalResource {
   public:
    LogicalResource& setName(std::string const& name);
    LogicalResource& setFormat(vk::Format format);
    LogicalResource& setSizing(ResourceSizing sizing);
    LogicalResource& setExtent(glm::uvec2 const& extent);

    glm::uvec2 getAbsoluteExtent(glm::uvec2 const& windowExtent) const;
    bool       isDepthResource() const;
    bool       isColorResource() const;

    friend class FrameGraph;

   private:
    std::string    mName   = "Unnamed Resource";
    vk::Format     mFormat = vk::Format::eR8G8B8A8Unorm;
    ResourceSizing mSizing = ResourceSizing::eRelative;
    glm::vec2      mExtent = glm::vec2(1.f, 1.f);

    // this is directly accessed by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------

  class LogicalPass {
   public:
    LogicalPass& setName(std::string const& name);

    LogicalPass& assignResource(LogicalResource const& resource, ResourceAccess access);
    LogicalPass& assignResource(LogicalResource const& resource, vk::ClearValue const& clear);

    LogicalPass& setProcessCallback(std::function<void(CommandBufferPtr)> const& callback);

    LogicalResource const* getDepthAttachment() const;

    friend class FrameGraph;

   private:
    void assignResource(LogicalResource const& resource, ResourceAccess access,
        std::optional<vk::ClearValue> const& clear);

    struct Info {
      ResourceAccess                mAccess;
      std::optional<vk::ClearValue> mClear;
    };

    std::unordered_map<LogicalResource const*, Info> mLogicalResources;
    std::string                                      mName = "Unnamed Pass";
    std::function<void(CommandBufferPtr)>            mProcessCallback;

    // this is directly accessed by the FrameGraph
    bool mDirty = true;
  };

  // -----------------------------------------------------------------------------------------------
  struct PhysicalResource {
    std::vector<LogicalResource const*> mLogicalResources;
    BackedImagePtr                      mImage;
  };

  // -----------------------------------------------------------------------------------------------
  struct PhysicalPass {
    std::vector<LogicalPass const*> mSubPasses;
    std::vector<LogicalPass const*> mDependencies;

    glm::uvec2    mExtent = glm::uvec2(0);
    RenderPassPtr mRenderPass;
  };

  // -----------------------------------------------------------------------------------------------

  FrameGraph(
      std::string const& name, DevicePtr const& device, FrameResourceIndexPtr const& frameIndex);

  LogicalResource& createResource();
  LogicalPass&     createPass();

  void setOutput(WindowPtr const& window, LogicalPass const& pass, LogicalResource const& resource);
  void process(uint32_t threadCount = 8);

 private:
  bool isDirty() const;
  void clearDirty();
  void validate() const;

  struct PerFrame {
    CommandBufferPtr mPrimaryCommandBuffer;
    vk::SemaphorePtr mRenderFinishedSemaphore;
    vk::FencePtr     mFrameFinishedFence;

    std::list<PhysicalResource> mPhysicalResources;
    std::list<PhysicalPass>     mPhysicalPasses;
    bool                        mDirty = true;
  };

  DevicePtr mDevice;

  WindowPtr              mOutputWindow;
  LogicalPass const*     mOutputPass     = nullptr;
  LogicalResource const* mOutputResource = nullptr;

  std::list<LogicalResource> mLogicalResources;
  std::list<LogicalPass>     mLogicalPasses;
  Core::ThreadPool           mThreadPool;
  FrameResource<PerFrame>    mPerFrame;
  bool                       mDirty = true;
};

} // namespace Illusion::Graphics

#endif // ILLUSION_GRAPHICS_RENDER_GRAPH_HPP
