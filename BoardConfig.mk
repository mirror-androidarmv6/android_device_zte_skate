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

# Define BOARD_HAVE_BLUETOOTH_BLUEZ before device/qcom/msm7x27/BoardConfigCommon.mk
# Bluetooth
BOARD_HAVE_BLUETOOTH_BLUEZ := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/zte/skate/bluetooth

# Use the non-open-source parts, if they're present
include device/zte/msm7x27-common/BoardConfigCommon.mk
include vendor/zte/skate/BoardConfigVendor.mk

# Kernel and target stuff
# TARGET_RECOVERY_INITRC := device/zte/skate/recovery/recovery.rc
TARGET_RECOVERY_FSTAB := device/zte/skate/fstab.skate
TARGET_KERNEL_CONFIG := skate_jb_defconfig
BOARD_KERNEL_CMDLINE := androidboot.hardware=skate console=null
TARGET_BOOTLOADER_NAME := skate
TARGET_OTA_ASSERT_DEVICE := skate

# Graphics
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200
TARGET_DOESNT_USE_FENCE_SYNC := true
TARGET_QCOM_HDMI_OUT := false
TARGET_QCOM_LEGACY_OMX := true
BOARD_EGL_NEEDS_LEGACY_FB := true
TARGET_NO_HW_VSYNC := false
COMMON_GLOBAL_CFLAGS += -DANCIENT_GL
COMMON_GLOBAL_CFLAGS += -DREFRESH_RATE=60

# Wifi
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WLAN_DEVICE := bcmdhd
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
WIFI_DRIVER_MODULE_PATH := "/system/lib/modules/dhd.ko"
WIFI_DRIVER_FW_PATH_AP := "/system/etc/fw_4319.bin"
WIFI_DRIVER_FW_PATH_STA := "/system/etc/fw_4319_apsta.bin"
WIFI_DRIVER_MODULE_ARG := "firmware_path=/system/etc/fw_4319.bin nvram_path=/system/etc/nv_4319.txt iface_name=wlan"
WIFI_DRIVER_MODULE_NAME := "dhd"
WIFI_DRIVER_LOADER_REUSE := true

# dev:    size   erasesize  name
# mtd0: 00500000 00020000 "recovery"
# mtd1: 00500000 00020000 "boot"
# mtd2: 00180000 00020000 "splash"
# mtd3: 00080000 00020000 "misc"
# mtd4: 00200000 00020000 "cache"
# mtd5: 08a00000 00020000 "system"
# mtd6: 13ba0000 00020000 "userdata"
# mtd7: 00020000 00020000 "oem"
# mtd8: 00180000 00020000 "persist"
BOARD_BOOTIMAGE_PARTITION_SIZE     := 0x00500000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x00600000
BOARD_SYSTEMIMAGE_PARTITION_SIZE   := 0x0ee00000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 0x0a280000
BOARD_FLASH_BLOCK_SIZE := 131072
