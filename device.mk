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

DEVICE_PACKAGE_OVERLAYS := device/zte/skate/overlay

$(call inherit-product, device/common/gps/gps_eu_supl.mk)
$(call inherit-product, frameworks/base/build/phone-hdpi-512-dalvik-heap.mk)

PRODUCT_AAPT_CONFIG := normal hdpi
PRODUCT_AAPT_PREF_CONFIG := hdpi

PRODUCT_PACKAGES += \
    lights.skate \
    sensors.skate \
    librpc \
    SkateParts \
    libmm-omxcore \
    libOmxCore \
    libstagefrighthw \
    dexpreopt \
    camera.msm7x27 \
    gralloc.msm7x27 \
    copybit.msm7x27 \
    hwcomposer.msm7x27

PRODUCT_PACKAGES += \
    audio.primary.skate \
    audio_policy.skate \
    audio.a2dp.default \
    libaudioutils

PRODUCT_TAGS += dalvik.gc.type-precise
DISABLE_DEXPREOPT := false

# Init files
PRODUCT_COPY_FILES := \
    device/zte/skate/init.skate.rc:root/init.skate.rc \
    device/zte/skate/init.skate.usb.rc:root/init.skate.usb.rc \
    device/zte/skate/ueventd.skate.rc:root/ueventd.skate.rc

# USB
PRODUCT_COPY_FILES += \
    device/zte/skate/prebuilt/synaptics-rmi-touchscreen.idc:system/usr/idc/synaptics-rmi-touchscreen.idc \
    device/zte/skate/prebuilt/synaptics-rmi4-ts.idc:system/usr/idc/synaptics-rmi4-ts.idc \
    device/zte/skate/prebuilt/skate_keypad.kl:/system/usr/keylayout/skate_keypad.kl \

# Vold
PRODUCT_COPY_FILES += \
    device/zte/skate/vold.fstab:system/etc/vold.fstab

# Audio and media
PRODUCT_COPY_FILES += \
    device/zte/skate/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/zte/skate/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt \
    device/zte/skate/media_profiles.xml:system/etc/media_profiles.xml

# USB
PRODUCT_COPY_FILES += \
    device/zte/skate/prebuilt/start_usb0.sh:system/etc/start_usb0.sh

# GooManager
PRODUCT_COPY_FILES += \
    device/zte/skate/prebuilt/GooManager.apk:system/app/GooManager.apk

# WiFi
PRODUCT_COPY_FILES += \
    device/zte/skate/firmware/fw_4319.bin:system/etc/fw_4319.bin \
    device/zte/skate/firmware/fw_4319_apsta.bin:system/etc/fw_4319_apsta.bin \
    device/zte/skate/firmware/nv_4319.txt:system/etc/nv_4319.txt

# Bluetooth
PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.le.conf:system/etc/bluetooth/main.conf \
    device/zte/skate/init.qcom.bt.sh:system/etc/init.qcom.bt.sh

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/base/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml

$(call inherit-product-if-exists, vendor/zte/skate/skate-vendor.mk)
