/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Overlay"

#include "NoOpOverlay.h"

namespace android {

Overlay::Overlay()
{
    //NOOP
}

Overlay::~Overlay()
{
      //NOOP
}

status_t Overlay::dequeueBuffer(void** buffer)
{
    return NO_ERROR;
}

status_t Overlay::queueBuffer(void* buffer)
{
    return NO_ERROR;
}

status_t Overlay::resizeInput(uint32_t width, uint32_t height)
{
    return NO_ERROR;
}

status_t Overlay::setParameter(int param, int value)
{
    return NO_ERROR;
}

status_t Overlay::setCrop(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    return NO_ERROR;
}

status_t Overlay::getCrop(uint32_t* x, uint32_t* y, uint32_t* w, uint32_t* h)
{
    return NO_ERROR;
}

status_t Overlay::setFd(int fd)
{
    return NO_ERROR;
}

int32_t Overlay::getBufferCount() const
{
    return 0;
}

void* Overlay::getBufferAddress(void* buffer)
{
    return 0;
}

void Overlay::destroy() {
    //NOOP
}

status_t Overlay::getStatus() const {
    return NO_ERROR;
}

void* Overlay::getHandleRef() const {
    return 0;
}

uint32_t Overlay::getWidth() const {
    return 0;
}

uint32_t Overlay::getHeight() const {
    return 0;
}

int32_t Overlay::getFormat() const {
    return 0;
}

int32_t Overlay::getWidthStride() const {
    return 0;
}

int32_t Overlay::getHeightStride() const {
    return 0;
}

// ----------------------------------------------------------------------------

}; // namespace android
