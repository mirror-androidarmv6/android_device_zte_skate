CyanogenMod 11.0 device configuration for ZTE Skate.

How to build:
-------------

Initialize repo:

    repo init -u git://github.com/androidarmv6/android.git -b cm-11.0
    repo sync

Compile:

    . build/envsetup.sh
    brunch skate
