cmake_minimum_required(VERSION 3.5)
project(plc)

include_directories("./inc/")
include_directories("/usr/include/glib-2.0")

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
include_directories("/usr/lib/arm-linux-gnueabihf/glib-2.0/include/")
ELSE(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
include_directories("/usr/lib/x86_64-linux-gnu/glib-2.0/include/")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")

set(CMAKE_CXX_COMPILER clang)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Werror -O2")

set(SRC_LIST ${SRC_LIST} src/utils/log.c)
set(SRC_LIST ${SRC_LIST} src/utils/utils.c)
set(SRC_LIST ${SRC_LIST} src/utils/configs.c)
set(SRC_LIST ${SRC_LIST} src/net/response/response.c)
set(SRC_LIST ${SRC_LIST} src/net/server.c)
set(SRC_LIST ${SRC_LIST} src/net/notifier.c)
set(SRC_LIST ${SRC_LIST} src/net/handlers/securityh.c)
set(SRC_LIST ${SRC_LIST} src/core/gpio.c)
set(SRC_LIST ${SRC_LIST} src/core/lcd.c)
set(SRC_LIST ${SRC_LIST} src/core/extenders.c)
set(SRC_LIST ${SRC_LIST} src/core/onewire.c)
set(SRC_LIST ${SRC_LIST} src/controllers/controllers.c)
set(SRC_LIST ${SRC_LIST} src/controllers/security.c)
set(SRC_LIST ${SRC_LIST} src/ftest/ftest.c)
set(SRC_LIST ${SRC_LIST} src/main.c)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} -lfcgi -ljansson -lcurl -lm -lglib-2.0 -pthread) 


IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 
target_link_libraries(${PROJECT_NAME} -lwiringPi -lwiringPiDev)
ENDIF()