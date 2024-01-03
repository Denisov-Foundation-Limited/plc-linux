#!/bin/bash

###################################################################
#
#                          INSTALL COMMANDS
#
###################################################################

for param in "$*"
do
    if [[ $param == "--install" ]] ; then
        mkdir -p /var/log/plc
        mkdir -p /usr/share/plc
        mkdir -p /usr/share/plc/cam
        mkdir -p /usr/share/plc/db
        cp -r ./data/configs /etc/plc
        cp plc /bin/
        exit 0
    fi
done

###################################################################
#
#                          FIND LIBS
#
###################################################################

function find_libs() {
    if [[ ! -e "/usr/include/arm-linux-gnueabihf/curl/curl.h" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/include/sqlite3.h" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/include/fcgiapp.h" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/include/glib-2.0/glib.h" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/include/jansson.h" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/bin/cmake" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/bin/clang" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/bin/make" ]] ; then
        return 0
    fi
    if [[ ! -e "/usr/bin/git" ]] ; then
        return 0
    fi
    return 1
}

find_libs
if [[ $? -eq 0 ]] ; then
    apt update && apt install --yes libfcgi-dev libglib2.0-dev libcurl4-openssl-dev \
        libjansson-dev cmake clang make libglib2.0-dev libsqlite3-dev git nginx ffmpeg
    cp -r ./data/system/default /etc/nginx/sites-enabled/
    cp ./data/system/plc.service /etc/systemd/system/
    systemctl enable plc
    armbian-config
    reboot
fi

if [[ ! -e "/usr/lib/libwiringPiLite.a" ]] ; then
    git clone https://github.com/LittleBuster/wiringPiLite.git
    cd wiringPiLite
    chmod +x build.sh && ./build.sh
    cd -
    rm -rf wiringPiLite
fi

cat /etc/modules | grep -r "w1-therm" > /dev/null
if [[ ! $? -eq 0 ]] ; then
    echo "w1-sunxi" >> /etc/modules
    echo "w1-gpio" >> /etc/modules
    echo "w1-therm" >> /etc/modules
fi

cat /boot/armbianEnv.txt | grep "param_w1_pin" > /dev/null
if [[ ! $? -eq 0 ]] ; then
    echo "overlays=w1-gpio" >> /boot/armbianEnv.txt
    echo "param_w1_pin=PA10" >> /boot/armbianEnv.txt
fi

cat /etc/modprobe.d/w1.conf | grep "slave_ttl=" > /dev/null
if [[ ! $? -eq 0 ]] ; then
    echo "options wire timeout=1 slave_ttl=1" > /etc/modprobe.d/w1.conf
fi

###################################################################
#
#                          BUILD FUNCTIONS
#
###################################################################

cmake .
cmake --build . -j4