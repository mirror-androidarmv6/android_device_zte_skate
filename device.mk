#
# Copyright (C) 2011 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This file includes all definitions that apply to ALL skate devices, and
# are also specific to skate devices
#
# Everything in this directory will become public

DEVICE_PACKAGE_OVERLAYS := device/zte/skate/overlay

PRODUCT_PACKAGES += \
        lights.skate \
        sensors.skate \
        SkateParts \
        copybit.skate \
        audio.primary.skate \
        audio_policy.skate

# Enable repeatable keys in CWM
PRODUCT_PROPERTY_OVERRIDES += \
        ro.cwm.enable_key_repeat=true

PRODUCT_COPY_FILES += \
        device/zte/skate/init.skate.rc:root/init.skate.rc \
        device/zte/skate/init.skate.usb.rc:root/init.skate.usb.rc \
        device/zte/skate/ueventd.skate.rc:root/ueventd.skate.rc \
        device/zte/skate/prebuilt/skate_keypad.kl:/system/usr/keylayout/skate_keypad.kl \
        device/zte/skate/prebuilt/synaptics-rmi4-ts.idc:system/usr/idc/synaptics-rmi4-ts.idc \
        device/zte/skate/prebuilt/gralloc.skate.so:system/lib/hw/gralloc.skate.so \
        device/zte/skate/prebuilt/synaptics-rmi-touchscreen.idc:system/usr/idc/synaptics-rmi-touchscreen.idc \
        device/zte/skate/init.qcom.bt.sh:system/etc/init.qcom.bt.sh

# WiFi
PRODUCT_COPY_FILES += \
        device/zte/skate/firmware/fw_4319.bin:system/etc/fw_4319.bin \
        device/zte/skate/firmware/fw_4319_apsta.bin:system/etc/fw_4319_apsta.bin \
        device/zte/skate/firmware/nv_4319.txt:system/etc/nv_4319.txt

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
         frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
         frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

-include device/zte/msm7x27-common/common.mk
$(call inherit-product-if-exists, vendor/zte/skate/skate-vendor.mk)
