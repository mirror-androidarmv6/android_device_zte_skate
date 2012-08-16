# Inherit device configuration
$(call inherit-product, device/zte/skate/skate.mk)

# Correct bootanimation size for the screen
TARGET_BOOTANIMATION_NAME := vertical-480x800

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/gsm.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_mini_phone.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=skate BUILD_ID=JRO03C BUILD_FINGERPRINT=ZTE/N880E_JB/atlas40:4.1.1/JRO03C/eng.songsy.20120718.233441:eng/test-keys PRIVATE_BUILD_DESC="N880E_JB-eng 4.1.1 JRO03C eng.songsy.20120718.233441 test-keys" BUILD_NUMBER=eng.songsy.20120718.233441

ifneq ($(CFX_BUILD),)
PRODUCT_NAME := cfx_skate
else
PRODUCT_NAME := cm_skate
endif
PRODUCT_DEVICE := skate
PRODUCT_BRAND := ZTE
PRODUCT_MANUFACTURER := ZTE
PRODUCT_MODEL := Skate
PRODUCT_RELEASE_NAME := Skate

ifdef I_AM_TILAL
PRODUCT_PROPERTY_OVERRIDES += \
    ro.goo.board=skate \
    ro.goo.developerid=tilal6991 \
    ro.goo.version=$(shell date -u +%Y%m%d) \
    ro.goo.rom=cfx2skate

PRODUCT_PROPERTY_OVERRIDES += \
    otaupdater.otaid=cfx2skate \
    otaupdater.otatime=$(shell date -u +%Y%m%d-%H%m) \
    otaupdater.otaver=$(shell date -u +%Y%m%d)
endif
