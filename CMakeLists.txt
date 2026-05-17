# Copyright (c) 2026 Semtech Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/usp_preview/usp_zephyr)
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/usp_preview/usp)

set(SHIELD semtech_wio_lr2021)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(swdm031_img_xfer)

# Add the application files
if(CONFIG_TRANSMITTER)
file(GLOB IMAGE_DEMO_SRC src/transmitter/*.c)
target_include_directories(app PRIVATE src/transmitter )
elseif(CONFIG_RECEIVER)
file(GLOB IMAGE_DEMO_SRC src/receiver/*.c)
target_include_directories(app PRIVATE src/receiver )
endif()

target_include_directories(app PRIVATE
    src/common
)
file(GLOB COMMON src/common/*.c)

target_sources(app PRIVATE
    ${COMMON}
    ${IMAGE_DEMO_SRC}
    src/main.c
    src/xiaosync/xiaosync.c
)
