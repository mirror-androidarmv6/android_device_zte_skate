#!/sbin/sh
#
# Check if there is enough space on /system to install full ROM.
#

sys_size=$(df -h system|grep dev/block|awk '{ print substr($2,1,length($2)-3) }')
echo /system is ${sys_size}MB

if [ $sys_size -ge 210 ]; then
    echo install extras
    setprop install.extras 1
else
    echo /system is too small for extras
    setprop install.extras 0
fi

