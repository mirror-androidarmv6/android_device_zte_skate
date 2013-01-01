CyanogenMod 10.1 device configuration for ZTE Skate.

How to build:
-------------

Initialize repo:

    repo init -u git://github.com/androidarmv6/android.git -b cm-10.1
    repo sync -j16

Compile:

    . build/envsetup.sh && lunch cm_skate-userdebug
    make bacon -j5
