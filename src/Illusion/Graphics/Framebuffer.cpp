////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Framebuffer.hpp"

#include "../Core/Logger.hpp"
#include "Device.hpp"
#include "Utils.hpp"

#include <iostream>

namespace Illusion::Graphics {

////////////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::Framebuffer(DevicePtr const& device, vk::RenderPassPtr const& renderPass,
    glm::uvec2 const& extent, std::vector<vk::Format> const& attachments)
    : mDevice(device)
    , mRenderPass(renderPass)
    , mExtent(extent) {

  ILLUSION_TRACE << "Creating Framebuffer." << std::endl;

  for (auto attachment : attachments) {
    vk::ImageAspectFlags aspect;

    if (Utils::isDepthOnlyFormat(attachment)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
    } else if (Utils::isDepthStencilFormat(attachment)) {
      aspect |= vk::ImageAspectFlagBits::eDepth;
      aspect |= vk::ImageAspectFlagBits::eStencil;
    } else {
      aspect |= vk::ImageAspectFlagBits::eColor;
    }

    // eTransferSrc is actually only required for the attachment which will be blitted to the
    // swapchain images
    vk::ImageUsageFlags usage =
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
    vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal;

    if (Utils::isDepthFormat(attachment)) {
      usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
      layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    vk::ImageCreateInfo imageInfo;
    imageInfo.imageType     = vk::ImageType::e2D;
    imageInfo.format        = attachment;
    imageInfo.extent.width  = extent.x;
    imageInfo.extent.height = extent.y;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.samples       = vk::SampleCountFlagBits::e1;
    imageInfo.tiling        = vk::ImageTiling::eOptimal;
    imageInfo.usage         = usage;
    imageInfo.sharingMode   = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    auto image = mDevice->createBackedImage(imageInfo, vk::ImageViewType::e2D, aspect,
        vk::MemoryPropertyFlagBits::eDeviceLocal, layout);

    mImageStore.push_back(image);
  }

  std::vector<vk::ImageView> imageViews(mImageStore.size());

  for (size_t i(0); i < mImageStore.size(); ++i) {
    imageViews[i] = *mImageStore[i]->mView;
  }

  vk::FramebufferCreateInfo info;
  info.renderPass      = *mRenderPass;
  info.attachmentCount = static_cast<uint32_t>(imageViews.size());
  info.pAttachments    = imageViews.data();
  info.width           = mExtent.x;
  info.height          = mExtent.y;
  info.layers          = 1;

  mFramebuffer = mDevice->createFramebuffer(info);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Framebuffer::~Framebuffer() {
  ILLUSION_TRACE << "Deleting Framebuffer." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Graphics
