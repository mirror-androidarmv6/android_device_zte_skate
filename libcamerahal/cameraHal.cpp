/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2012 Raviprasad V Mummidi.
 * Copyright (C) 2012 Zhibin Wu, Simon Davie, Nico Kaiser
 * Copyright (C) 2012 QiSS ME Project Team
 * Copyright (C) 2012 Twisted, Sean Neeley
 * Copyright (C) 2012 Tobias Droste
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

#define LOG_TAG "CameraHAL"

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <linux/ioctl.h>
#include <linux/msm_mdp.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include <gralloc_priv.h>
#include <hardware/hardware.h>
#include <hardware/camera.h>
#include <camera/CameraParameters.h>
#include "CameraHardwareInterface.h"
#include <binder/IMemory.h>

#include "NoOpOverlay.h"

#define NO_ERROR 0
//#define LOGV LOGI
#define GRALLOC_USAGE_PMEM_PRIVATE_ADSP GRALLOC_USAGE_PRIVATE_0

using android::sp;
using android::Overlay;
using android::String8;
using android::IMemory;
using android::IMemoryHeap;
using android::CameraParameters;

using android::CameraInfo;
using android::HAL_getCameraInfo;
using android::HAL_getNumberOfCameras;
using android::CameraHardwareInterface;

struct qcom_mdp_rect {
   uint32_t x; 
   uint32_t y;
   uint32_t w;
   uint32_t h;
};

struct qcom_mdp_img {
   uint32_t width;
   uint32_t height;
   uint32_t format;
   uint32_t offset;
   int      memory_id; /* The file descriptor */
};

struct qcom_mdp_blit_req {
   struct   qcom_mdp_img src;
   struct   qcom_mdp_img dst;
   struct   qcom_mdp_rect src_rect;
   struct   qcom_mdp_rect dst_rect;
   uint32_t alpha;
   uint32_t transp_mask;
   uint32_t flags;
   int sharpening_strength;  /* -127 <--> 127, default 64 */
};

struct blitreq {
   uint32_t count;
   struct qcom_mdp_blit_req req;
};

/* Prototypes and extern functions. */
extern "C" android::sp<android::CameraHardwareInterface> openCameraHardware(int id);
static int camera_device_open(const hw_module_t* module, const char* name,
                              hw_device_t** device);
static int camera_device_close(hw_device_t* device);
static int camera_get_number_of_cameras(void);
static int camera_get_camera_info(int camera_id, struct camera_info *info);

android::String8          g_str;
android::CameraParameters camSettings;
android::sp<android::CameraHardwareInterface> qCamera;

static hw_module_methods_t camera_module_methods = {
   open: camera_device_open
};

camera_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: CAMERA_HARDWARE_MODULE_ID,
        name: "Camera HAL for ICS",
        author: "Raviprasad V Mummidi",
        methods: &camera_module_methods,
        dso: NULL,
        reserved: {0},
    },
    get_number_of_cameras: camera_get_number_of_cameras,
    get_camera_info: camera_get_camera_info,
};

typedef struct priv_camera_device {
    camera_device_t base;
    /* specific "private" data can go here (base.priv) */
    int cameraid;
    /* new world */
    preview_stream_ops *window;
    camera_notify_callback notify_callback;
    camera_data_callback data_callback;
    camera_data_timestamp_callback data_timestamp_callback;
    camera_request_memory request_memory;
    void *user;
    /* old world*/
    int preview_width;
    int preview_height;
    sp<Overlay> overlay;
    gralloc_module_t const *gralloc;
} priv_camera_device_t;


static struct {
    int type;
    const char *text;
} msg_map[] = {
    {0x0001, "CAMERA_MSG_ERROR"},
    {0x0002, "CAMERA_MSG_SHUTTER"},
    {0x0004, "CAMERA_MSG_FOCUS"},
    {0x0008, "CAMERA_MSG_ZOOM"},
    {0x0010, "CAMERA_MSG_PREVIEW_FRAME"},
    {0x0020, "CAMERA_MSG_VIDEO_FRAME"},
    {0x0040, "CAMERA_MSG_POSTVIEW_FRAME"},
    {0x0080, "CAMERA_MSG_RAW_IMAGE"},
    {0x0100, "CAMERA_MSG_COMPRESSED_IMAGE"},
    {0x0200, "CAMERA_MSG_RAW_IMAGE_NOTIFY"},
    {0x0400, "CAMERA_MSG_PREVIEW_METADATA"},
    {0x0000, "CAMERA_MSG_ALL_MSGS"}, //0xFFFF
    {0x0000, "NULL"},
};

static void dump_msg(const char *tag, int msg_type)
{
    int i;
    for (i = 0; msg_map[i].type; i++) {
        if (msg_type & msg_map[i].type) {
            LOGV("%s: %s", tag, msg_map[i].text);
        }
    }
}

int
camera_get_number_of_cameras(void)
{
   LOGV("CameraHAL_GetNum_Cameras:\n");
   return 1;
}

int
camera_get_camera_info(int camera_id, struct camera_info *info)
{
   LOGV("CameraHAL_GetCam_Info:\n");
   info->facing      = CAMERA_FACING_BACK;
   info->orientation = 90;
   return NO_ERROR;
}

void CameraHAL_CopyBuffers_Hw(int srcFd, int destFd,
                              size_t srcOffset, size_t destOffset,
                              int srcFormat, int destFormat,
                              int x, int y, int w, int h)
{
    struct blitreq blit;
    int fb_fd;

    if (srcFd < 0 || destFd < 0)
       return;

    fb_fd = open("/dev/graphics/fb0", O_RDWR);
    if (fb_fd < 0) {
       LOGE("CameraHAL_CopyBuffers_Hw: Error opening /dev/graphics/fb0\n");
       return;
    }

    LOGV("CameraHAL_CopyBuffers_Hw: srcFD:%d destFD:%d srcOffset:%#x"
         " destOffset:%#x x:%d y:%d w:%d h:%d\n", srcFd, destFd, srcOffset,
         destOffset, x, y, w, h);

    memset(&blit, 0, sizeof(blit));
    blit.count = 1;

    blit.req.flags               = 0;
    blit.req.alpha               = 0xff;
    blit.req.transp_mask         = 0xffffffff;
    blit.req.sharpening_strength = 64;  /* -127 <--> 127, default 64 */

    blit.req.src.width     = w;
    blit.req.src.height    = h;
    blit.req.src.offset    = srcOffset;
    blit.req.src.memory_id = srcFd;
    blit.req.src.format    = srcFormat;

    blit.req.dst.width     = w;
    blit.req.dst.height    = h;
    blit.req.dst.offset    = 0;
    blit.req.dst.memory_id = destFd;
    blit.req.dst.format    = destFormat;

    blit.req.src_rect.x = blit.req.dst_rect.x = x;
    blit.req.src_rect.y = blit.req.dst_rect.y = y;
    blit.req.src_rect.w = blit.req.dst_rect.w = w;
    blit.req.src_rect.h = blit.req.dst_rect.h = h;

    if (ioctl(fb_fd, MSMFB_BLIT, &blit)) {
       LOGE("CameraHAL_CopyBuffers_Hw: MSMFB_BLIT failed = %d %s\n",
            errno, strerror(errno));
    }
    close(fb_fd);
}

void CameraHAL_HandlePreviewData(const android::sp<android::IMemory>& dataPtr,
                                 preview_stream_ops_t *mWindow,
                                 int32_t previewWidth, int32_t previewHeight)
{
   if (mWindow != NULL && dataPtr != NULL) {
      ssize_t  offset;
      size_t   size;
      int32_t  previewFormat = MDP_Y_CBCR_H2V2;
      int32_t  destFormat    = MDP_RGBA_8888;

      android::status_t retVal;
      android::sp<android::IMemoryHeap> mHeap = dataPtr->getMemory(&offset,
                                                                   &size);

      LOGV("CameraHAL_HandlePreviewData: previewWidth:%d previewHeight:%d "
           "offset:%#x size:%#x base:%p\n", previewWidth, previewHeight,
           (unsigned)offset, size, mHeap != NULL ? mHeap->base() : 0);

      retVal = mWindow->set_buffers_geometry(mWindow,
                                             previewWidth, previewHeight,
                                             HAL_PIXEL_FORMAT_RGBA_8888);
      if (retVal == NO_ERROR && mHeap != NULL) {
         int32_t          stride;
         buffer_handle_t *bufHandle = NULL;

         LOGV("CameraHAL_HandlePreviewData: dequeueing buffer\n");
         retVal = mWindow->dequeue_buffer(mWindow, &bufHandle, &stride);
         if (retVal == NO_ERROR && bufHandle != NULL) {
            retVal = mWindow->lock_buffer(mWindow, bufHandle);
            if (retVal == NO_ERROR) {
               private_handle_t const *privHandle =
                  reinterpret_cast<private_handle_t const *>(*bufHandle);
               if (privHandle != NULL) {
                  CameraHAL_CopyBuffers_Hw(mHeap->getHeapID(), privHandle->fd,
                                           offset, privHandle->offset,
                                           previewFormat, destFormat,
                                           0, 0, previewWidth, previewHeight);
               }
               mWindow->enqueue_buffer(mWindow, bufHandle);
               LOGV("CameraHAL_HandlePreviewData: enqueued buffer\n");
            } else {
               LOGE("CameraHAL_HandlePreviewData: ERROR locking the buffer\n");
               mWindow->cancel_buffer(mWindow, bufHandle);
            }
         } else {
            LOGE("CameraHAL_HandlePreviewData: ERROR dequeueing the buffer\n");
         }
      }
   }
}

/*******************************************************************
 * camera interface callback
 *******************************************************************/

static camera_memory_t *wrap_memory_data(priv_camera_device_t *dev,
                                         const sp<IMemory>& dataPtr)
{
    void *data;
    size_t size;
    ssize_t offset;
    sp<IMemoryHeap> heap;
    camera_memory_t *mem;

    LOGV("%s+++,dev->request_memory %p", __FUNCTION__,dev->request_memory);

    if (!dev->request_memory)
        return NULL;

    heap = dataPtr->getMemory(&offset, &size);
    data = (void *)((char *)(heap->base()) + offset);

    LOGV("%s: data: %p size: %i", __FUNCTION__, data, size);

    LOGV(" offset:0x%x ",  (uint)offset);

#if 0
    static int frameCnt = 0;
    int written;
    char path[128];
    snprintf(path, sizeof(path), "/data/%d_capture.jpg", frameCnt);
    int file_fd = open(path, O_RDWR | O_CREAT, 0666);
    LOGI("dumping capture jpeg %d", frameCnt);
    if (file_fd < 0) {
        LOGE("cannot open file:%s (error:%i)\n", path, file_fd);
    }
    else
    {
        LOGV("dumping jpeg");
        written = write(file_fd, (char *)data,
                        size);
        if(written < 0)
            LOGE("error in data write");
    }
    close(file_fd);
    frameCnt++;
#endif

    mem = dev->request_memory(-1, size, 1, dev->user);

    LOGV(" mem:%p,mem->data%p ",  mem,mem->data);

    memcpy(mem->data, data, size);
    LOGV("%s---", __FUNCTION__);

    return mem;
}

static void wrap_notify_callback(int32_t msg_type, int32_t ext1,
                                 int32_t ext2, void* user)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: type %i user %p", __FUNCTION__, msg_type,user);
    dump_msg(__FUNCTION__, msg_type);

    if (user == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) user;

    if (dev->notify_callback) {
        dev->notify_callback(msg_type, ext1, ext2, dev->user);
    }
    LOGV("%s---", __FUNCTION__);

}
//QiSS ME for capture
static void wrap_data_callback(int32_t msg_type, const sp<IMemory>& dataPtr,
                               void* user)
{
    camera_memory_t *data = NULL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: type %i user %p", __FUNCTION__, msg_type,user);
    dump_msg(__FUNCTION__, msg_type);

    if (user == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) user;

    if(msg_type ==CAMERA_MSG_RAW_IMAGE) {
        qCamera->disableMsgType(CAMERA_MSG_RAW_IMAGE);
        return;
    }

    if (msg_type == CAMERA_MSG_PREVIEW_FRAME) {
        int32_t previewWidth, previewHeight;

        android::CameraParameters hwParameters = qCamera->getParameters();
        hwParameters.getPreviewSize(&previewWidth, &previewHeight);
        CameraHAL_HandlePreviewData(dataPtr, dev->window, previewWidth, previewHeight);

        return;
    }

    data = wrap_memory_data(dev, dataPtr);

    if (dev->data_callback) {
        dev->data_callback(msg_type, data, 0, NULL, dev->user);
    }

    LOGV("%s---", __FUNCTION__);

}

//QiSS ME for record
static void wrap_data_callback_timestamp(nsecs_t timestamp, int32_t msg_type,
                                         const sp<IMemory>& dataPtr, void* user)
{
    priv_camera_device_t* dev = NULL;
    camera_memory_t *data = NULL;

    LOGV("%s+++: type %i user %p", __FUNCTION__, msg_type,user);
    dump_msg(__FUNCTION__, msg_type);

    if (user == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) user;

    data = wrap_memory_data(dev, dataPtr);

    if (dev->data_timestamp_callback) {
        dev->data_timestamp_callback(timestamp,msg_type, data, 0, dev->user);
    }

    qCamera->releaseRecordingFrame(dataPtr);//QiSS ME need release or record will stop

    if (data != NULL) {
        data->release(data);
    }

    LOGV("%s---", __FUNCTION__);

}

/*******************************************************************
 * implementation of priv_camera_device_ops functions
 *******************************************************************/

int camera_set_preview_window(struct camera_device *device,
                              struct preview_stream_ops *window)
{
    int rv = -EINVAL;
    int min_bufs = -1;
    int kBufferCount = 4;
    int preview_width;
    int preview_height;
    int hal_pixel_format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
    const char *str_preview_format;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++,device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    dev->window = window;

    if (window == NULL) {
        LOGV("%s---: window is NULL", __FUNCTION__);
        return 0;
    }

    if (!dev->gralloc) {
        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID,
                          (const hw_module_t **)&(dev->gralloc))) {
            LOGE("%s: Fail on loading gralloc HAL", __FUNCTION__);
        }
    }

    if (window->get_min_undequeued_buffer_count(window, &min_bufs)) {
        LOGE("%s---: could not retrieve min undequeued buffer count", __FUNCTION__);
        return -1;
    }

    LOGV("%s: bufs:%i", __FUNCTION__, min_bufs);

    if (min_bufs >= kBufferCount) {
        LOGE("%s: min undequeued buffer count %i is too high (expecting at most %i)",
             __FUNCTION__, min_bufs, kBufferCount - 1);
    }

    LOGI("%s: setting buffer count to %i", __FUNCTION__, kBufferCount);
    if (window->set_buffer_count(window, kBufferCount)) {
        LOGE("%s---: could not set buffer count", __FUNCTION__);
        return -1;
    }

    CameraParameters params(qCamera->getParameters());
    params.getPreviewSize(&preview_width, &preview_height);

    str_preview_format = params.getPreviewFormat();
    LOGI("%s: preview format %s", __FUNCTION__, str_preview_format);

    window->set_usage(window, GRALLOC_USAGE_PMEM_PRIVATE_ADSP | GRALLOC_USAGE_SW_READ_OFTEN);

    if (window->set_buffers_geometry(window, preview_width,
                                     preview_height, hal_pixel_format)) {
        LOGE("%s---: could not set buffers geometry to %s",
             __FUNCTION__, str_preview_format);
        return -1;
    }

    dev->preview_width = preview_width;
    dev->preview_height = preview_height;

    dev->overlay = new Overlay();

    LOGV("%s---,rv %d", __FUNCTION__,rv);

    return rv;
}

void camera_set_callbacks(struct camera_device *device,
                          camera_notify_callback notify_cb,
                          camera_data_callback data_cb,
                          camera_data_timestamp_callback data_cb_timestamp,
                          camera_request_memory get_memory, void *user)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++,device %p", __FUNCTION__,device);

    if (device == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) device;

    dev->notify_callback = notify_cb;
    dev->data_callback = data_cb;
    dev->data_timestamp_callback = data_cb_timestamp;
    dev->request_memory = get_memory;
    dev->user = user;

    qCamera->setCallbacks(wrap_notify_callback, wrap_data_callback,
                          wrap_data_callback_timestamp, (void *)dev);

    LOGV("%s---", __FUNCTION__);
}

void camera_enable_msg_type(struct camera_device *device, int32_t msg_type)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: type %i device %p", __FUNCTION__, msg_type,device);
    if (msg_type & CAMERA_MSG_RAW_IMAGE_NOTIFY) {
        msg_type &= ~CAMERA_MSG_RAW_IMAGE_NOTIFY;
        msg_type |= CAMERA_MSG_RAW_IMAGE;
    }

    dump_msg(__FUNCTION__, msg_type);

    if (device == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) device;

    qCamera->enableMsgType(msg_type);
    LOGV("%s---", __FUNCTION__);
}

void camera_disable_msg_type(struct camera_device *device, int32_t msg_type)
{
   if (msg_type == 0xfff) {
      msg_type = 0x1ff;
   }

    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: type %i device %p", __FUNCTION__, msg_type,device);
    dump_msg(__FUNCTION__, msg_type);

    if(!device)
        return;

    dev = (priv_camera_device_t*) device;

    qCamera->disableMsgType(msg_type);
    LOGV("%s---", __FUNCTION__);
}

int camera_msg_type_enabled(struct camera_device *device, int32_t msg_type)
{
    priv_camera_device_t* dev = NULL;
    int rv = -EINVAL;

    LOGV("%s+++: type %i device %p", __FUNCTION__, msg_type,device);

    if(!device)
        return 0;

    dev = (priv_camera_device_t*) device;

    rv = qCamera->msgTypeEnabled(msg_type);
    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

int camera_start_preview(struct camera_device *device)
{
    int rv = 0;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL)
        return rv;

    dev = (priv_camera_device_t*) device;

    if (!qCamera->msgTypeEnabled(CAMERA_MSG_PREVIEW_FRAME)) {
       qCamera->enableMsgType(CAMERA_MSG_PREVIEW_FRAME);

    rv = qCamera->startPreview();
    }

    LOGV("%s--- rv %d", __FUNCTION__,rv);

    return rv;
}

void camera_stop_preview(struct camera_device *device)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) device;

    qCamera->stopPreview();

    LOGV("%s---", __FUNCTION__);
}

int camera_preview_enabled(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->previewEnabled();

    LOGV("%s--- rv %d", __FUNCTION__,rv);

    return rv;
}

int camera_store_meta_data_in_buffers(struct camera_device *device, int enable)
{
   LOGV("camera_store_meta_data_in_buffers:\n");
   return NO_ERROR;
}

int camera_start_recording(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    qCamera->enableMsgType(CAMERA_MSG_VIDEO_FRAME);

    rv = qCamera->startRecording();

    LOGV("%s--- rv %d", __FUNCTION__,rv);

    return NO_ERROR;
}

void camera_stop_recording(struct camera_device *device)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) device;

    qCamera->disableMsgType(CAMERA_MSG_VIDEO_FRAME);

    qCamera->stopRecording();

    qCamera->startPreview();

    LOGV("%s---", __FUNCTION__);
}

int camera_recording_enabled(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->recordingEnabled();
    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

void camera_release_recording_frame(struct camera_device *device,
                                    const void *opaque)
{
   /*
    * We release the frame immediately in CameraHAL_DataTSCb after making a
    * copy. So, this is just a NOP.
    */
   LOGV("camera_release_recording_frame: opaque: %p\n", opaque);
}

int camera_auto_focus(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->autoFocus();

    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

int camera_cancel_auto_focus(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->cancelAutoFocus();
    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

int camera_take_picture(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    /* TODO: Remove hack. */
    qCamera->enableMsgType(CAMERA_MSG_SHUTTER |
                         CAMERA_MSG_POSTVIEW_FRAME |
                         CAMERA_MSG_RAW_IMAGE |
                         CAMERA_MSG_COMPRESSED_IMAGE);

    rv = qCamera->takePicture();

    LOGV("%s--- rv %d", __FUNCTION__,rv);
    //return rv;
    return NO_ERROR;
}

int camera_cancel_picture(struct camera_device *device)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->cancelPicture();

    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

int camera_set_parameters(struct camera_device * device, const char *params)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;
    CameraParameters camParams;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    String8 params_str8(params);
    camParams.unflatten(params_str8);
#if 0
    camParams.dump();
#endif

    rv = qCamera->setParameters(camParams);

#if 0
    camParams.dump();
#endif
    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

char *camera_get_parameters(struct camera_device *device)
{
    char *params = NULL;
    priv_camera_device_t *dev = NULL;
    String8 params_str8;
    CameraParameters camParams;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return NULL;
    }

    dev = (priv_camera_device_t*) device;

    camParams = qCamera->getParameters();
#if 0
    camParams.dump();
#endif

    camParams.set("orientation", "landscape");

    params_str8 = camParams.flatten();
    params = (char*) malloc(sizeof(char) * (params_str8.length()+1));
    strcpy(params, params_str8.string());

#if 0
    camParams.dump();
#endif
    LOGV("%s---", __FUNCTION__);
    return params;
}

void camera_put_parameters(struct camera_device *device, char *params)
{
    LOGV("%s+++", __FUNCTION__);
    free(params);
    LOGV("%s---", __FUNCTION__);
}

int camera_send_command(struct camera_device *device, int32_t cmd,
                        int32_t arg0, int32_t arg1)
{
    int rv = -EINVAL;
    priv_camera_device_t *dev = NULL;

    LOGV("%s: cmd %i,device %p", __FUNCTION__, cmd,device);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    rv = qCamera->sendCommand(cmd, arg0, arg1);
    LOGV("%s--- rv %d", __FUNCTION__,rv);
    return rv;
}

void camera_release(struct camera_device *device)
{
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    if (device == NULL) {
        return;
    }

    dev = (priv_camera_device_t*) device;

    qCamera->release();
    LOGV("%s---", __FUNCTION__);
}

int camera_dump(struct camera_device *device, int fd)
{
    int rv = -EINVAL;
    priv_camera_device_t* dev = NULL;
    android::Vector<android::String16> args;
    LOGV("%s", __FUNCTION__);

    if (device == NULL) {
        return rv;
    }

    dev = (priv_camera_device_t*) device;

    return qCamera->dump(fd, args);
}

int camera_device_close(hw_device_t *device)
{
    int ret = 0;
    priv_camera_device_t* dev = NULL;

    LOGV("%s+++: device %p", __FUNCTION__, device);

    //android::Mutex::Autolock lock(gCameraDeviceLock);

    if (device == NULL) {
        ret = -EINVAL;
        goto done;
    }

    dev = (priv_camera_device_t*) device;

    if (dev) {
        qCamera = NULL;

        if (dev->base.ops) {
            free(dev->base.ops);
        }
        free(dev);
    }
done:
#ifdef HEAPTRACKER
    heaptracker_free_leaked_memory();
#endif
    LOGV("%s--- ret %d", __FUNCTION__,ret);

    return ret;
}

int camera_device_open(const hw_module_t *module, const char *name,
                       hw_device_t **device)
{

    int rv = 0;
    int cameraid;
    int num_cameras = 0;
    priv_camera_device_t *priv_camera_device = NULL;
    camera_device_ops_t *camera_ops = NULL;
    sp<CameraHardwareInterface> camera = NULL;

#if 0
    android::Mutex::Autolock lock(gCameraDeviceLock);
#endif

    LOGI("camera_device open+++");

    if (name != NULL) {
        cameraid = atoi(name);

        priv_camera_device = (priv_camera_device_t*)malloc(sizeof(*priv_camera_device));
        if(!priv_camera_device)
        {
            LOGE("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if(!camera_ops)
        {
            LOGE("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(priv_camera_device, 0, sizeof(*priv_camera_device));
        memset(camera_ops, 0, sizeof(*camera_ops));

        priv_camera_device->base.common.tag = HARDWARE_DEVICE_TAG;
        priv_camera_device->base.common.version = 0;
        priv_camera_device->base.common.module = (hw_module_t *)(module);
        priv_camera_device->base.common.close = camera_device_close;
        priv_camera_device->base.ops = camera_ops;

        camera_ops->set_preview_window = camera_set_preview_window;
        camera_ops->set_callbacks = camera_set_callbacks;
        camera_ops->enable_msg_type = camera_enable_msg_type;
        camera_ops->disable_msg_type = camera_disable_msg_type;
        camera_ops->msg_type_enabled = camera_msg_type_enabled;
        camera_ops->start_preview = camera_start_preview;
        camera_ops->stop_preview = camera_stop_preview;
        camera_ops->preview_enabled = camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers = camera_store_meta_data_in_buffers;
        camera_ops->start_recording = camera_start_recording;
        camera_ops->stop_recording = camera_stop_recording;
        camera_ops->recording_enabled = camera_recording_enabled;
        camera_ops->release_recording_frame = camera_release_recording_frame;
        camera_ops->auto_focus = camera_auto_focus;
        camera_ops->cancel_auto_focus = camera_cancel_auto_focus;
        camera_ops->take_picture = camera_take_picture;
        camera_ops->cancel_picture = camera_cancel_picture;
        camera_ops->set_parameters = camera_set_parameters;
        camera_ops->get_parameters = camera_get_parameters;
        camera_ops->put_parameters = camera_put_parameters;
        camera_ops->send_command = camera_send_command;
        camera_ops->release = camera_release;
        camera_ops->dump = camera_dump;

        *device = &priv_camera_device->base.common;

        // -------- specific stuff --------

        priv_camera_device->cameraid = cameraid;

        camera = openCameraHardware(cameraid);

        if(camera == NULL)
        {
            LOGE("Couldn't create instance of CameraHal class");
            rv = -ENOMEM;
            goto fail;
        }

        qCamera = camera;
    }

    LOGI("%s---ok rv %d", __FUNCTION__,rv);

    return rv;

fail:
    if(priv_camera_device) {
        free(priv_camera_device);
        priv_camera_device = NULL;
    }
    if(camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    *device = NULL;
    LOGI("%s--- fail rv %d", __FUNCTION__,rv);

    return rv;
}
