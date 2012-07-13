/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>

#include "PS_ALS_common.h"

#include <cutils/log.h>

#include "SkateLight.h"

/*****************************************************************************/

SkateLight::SkateLight(char *dev)
    : SensorBase(dev, "light"),
      mEnabled(0),
      mInputReader(4),
      mPendingMask(0)
{

    mPendingEvents.version = sizeof(sensors_event_t);
    mPendingEvents.sensor = ID_L;
    mPendingEvents.type = SENSOR_TYPE_LIGHT;

    open_device();

    if (!ioctl(dev_fd, PS_ALS_IOCTL_ALS_ON)) {
        mEnabled = ioctl(dev_fd, PS_ALS_IOCTL_ALS_GET_ENABLED);
        setInitialState();
    }
    if (!mEnabled) {
        close_device();
    }

}

SkateLight::~SkateLight() {
    if (mEnabled) {
        enable(ID_L, 0);
    }
}

int SkateLight::setInitialState() {
    struct input_absinfo absinfo;
    if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_LIGHT), &absinfo)) {
        mPendingEvents.light = absinfo.value;
    }
    return 0;
}

int SkateLight::enable(int32_t handle, int en) {

    if (handle != ID_L)
        return -EINVAL;

    int newState = en ? 1 : 0;
    int err = 0;

    if (uint32_t(newState) != mEnabled) {
        if (!mEnabled) {
            open_device();
        }
        int cmd;

	if (newState) {
            cmd = PS_ALS_IOCTL_ALS_ON;
            ALOGD_IF(DEBUG,"ALS ON");
        } else {
            cmd = PS_ALS_IOCTL_ALS_OFF;
            ALOGD_IF(DEBUG,"ALS OFF");
        }
        err = ioctl(dev_fd, cmd);
        err = err<0 ? -errno : 0;
        ALOGE_IF(err, "PS_ALS_IOCTL_XXX failed (%s)", strerror(-err));
        if (!err) {
            if (en) {
                setInitialState();
                mEnabled = 1;
            }
            else
                mEnabled = 0;
        }
        if (!mEnabled) {
            ALOGD_IF(DEBUG,"closing device");
            close_device();
        }
    }
    return err;
}

bool SkateLight::hasPendingEvents() const {
    return mPendingMask;
}

int SkateLight::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mInputReader.fill(data_fd);
    if (n < 0)
        return n;

    int numEventReceived = 0;
    input_event const* event;
    while (count && mInputReader.readEvent(&event)) {
        int type = event->type;
        if (type == EV_ABS) {
            if (event->code == EVENT_TYPE_LIGHT) {
                ALOGD_IF(DEBUG,"Light value=%i",event->value);
                mPendingEvents.light = event->value;
            }
        } else if (type == EV_SYN) {
             int64_t time = timevalToNano(event->time);
             if (mEnabled) {
                 *data++ = mPendingEvents;
                 mPendingEvents.timestamp=time;
                 count--;
                 numEventReceived++;
             }
        } else {
            ALOGE("SkateLight: unknown event (type=%d, code=%d)",type, event->code);
        }
        mInputReader.next();
    }
    return numEventReceived;
}
