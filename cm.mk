# Correct bootanimation size for the screen
TARGET_SCREEN_HEIGHT := 800
TARGET_SCREEN_WIDTH := 480

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/gsm.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/tiny.mk)

# Inherit device configuration
$(call inherit-product, device/zte/skate/skate.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=skate BUILD_ID=JRO03C BUILD_FINGERPRINT=ZTE/N880E_JB/atlas40:4.1.1/JRO03C/eng.songsy.20120718.233441:eng/test-keys PRIVATE_BUILD_DESC="N880E_JB-eng 4.1.1 JRO03C eng.songsy.20120718.233441 test-keys" BUILD_NUMBER=eng.songsy.20120718.233441

# Set this variable here to overwrite the inherited value.
PRODUCT_NAME := cm_skate
