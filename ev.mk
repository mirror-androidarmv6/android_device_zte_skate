TARGET_BOOTANIMATION_NAME := vertical-480x800

$(call inherit-product, device/zte/skate/skate.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/ev/config/common_small_phone.mk)

# Inherit some common CM stuff.
$(call inherit-product, vendor/ev/config/gsm.mk)

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=skate BUILD_ID=IMM76D BUILD_FINGERPRINT=google/soju/crespo:4.0.4/IMM76D/299849:user/release-keys PRIVATE_BUILD_DESC="soju-user 4.0.4 IMM76D 299849 release-keys" BUILD_NUMBER=299849

PRODUCT_VERSION_DEVICE_SPECIFIC := a2

PRODUCT_MOTD :="\n\n\n--------------------MESSAGE---------------------\nThank you for choosing Evervolv\nVisit us at \#evervolv on irc.freenode.net\nFollow @preludedrew for the latest updates\n------------------------------------------------\n"

PRODUCT_PACKAGES += \
    Camera

# Hot reboot
PRODUCT_PACKAGE_OVERLAYS += vendor/ev/overlay/hot_reboot

# Copy compatible prebuilt files
PRODUCT_COPY_FILES += \
    vendor/ev/prebuilt/wvga/media/bootanimation.zip:system/media/bootanimation.zip

PRODUCT_NAME := ev_skate
PRODUCT_DEVICE := skate
PRODUCT_RELEASE_NAME := Skate
