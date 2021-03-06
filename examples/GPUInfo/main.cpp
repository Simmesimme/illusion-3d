////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Graphics/Instance.hpp>
#include <Illusion/Graphics/PhysicalDevice.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
// This minimal example creates a Vulkan instance and prints a very verbose list of your Vulkan   //
// implementation's hardware and software capabilities.                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
  // We enable eHeadlessMode as we won't create a window and therefore do not need to initialize our
  // windowing toolkit.
  auto instance = Illusion::Graphics::Instance::create(
      "GPUInfo", Illusion::Graphics::Instance::OptionBits::eHeadlessMode);
  instance->getPhysicalDevice()->printInfo();

  return 0;
}
