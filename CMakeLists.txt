cmake_minimum_required(VERSION 3.25)

project(AlopexOS
    VERSION 0.1
    LANGUAGES CXX ASM
)


# --------------------------------------------------
# Tools
# --------------------------------------------------

find_package(Python3 REQUIRED COMPONENTS Interpreter)


# --------------------------------------------------
# C++ Configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# --------------------------------------------------
# Kernel Compiler Flags
# --------------------------------------------------

set(KERNEL_FLAGS
    # Freestanding environment
    -ffreestanding

    # No runtime
    -fno-exceptions
    -fno-rtti
    -fno-stack-protector
    -fno-stack-check
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables

    # No host binary assumptions
    -fno-pie
    -fno-pic

    # x86_64 kernel rules (Add mcmodel=kernel here)
    -mcmodel=kernel
    -mno-red-zone
    -mgeneral-regs-only
    -mno-sse
    -mno-sse2

    # Debug
    -fno-omit-frame-pointer

    # Warnings
    -Wall
    -Wextra
    -Wpedantic
)


# --------------------------------------------------
# Kernel ELF
# --------------------------------------------------

add_executable(AlopexOS
    src/main.cpp
    src/kernel/limine_requests.cpp
    src/kernel/display.cpp
    src/kernel/new.cpp
    src/entry.s
)


target_include_directories(AlopexOS PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/limine
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/limine

)


target_compile_options(AlopexOS PRIVATE
    ${KERNEL_FLAGS}
)


# --------------------------------------------------
# Linker
# --------------------------------------------------

target_link_options(AlopexOS PRIVATE

    -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld

    -nostdlib
    -static

    -Wl,--build-id=none
    -Wl,-z,max-page-size=0x1000

)


# --------------------------------------------------
# Remove unwanted sections after linking
# --------------------------------------------------

add_custom_command(
    TARGET AlopexOS POST_BUILD

    COMMAND
        ${CMAKE_OBJCOPY}
        --remove-section=.note.gnu.property
        --remove-section=.comment
        $<TARGET_FILE:AlopexOS>

    COMMENT
        "Cleaning ELF metadata..."
)


# --------------------------------------------------
# Disk Image
# --------------------------------------------------

set(DISK_IMAGE
    ${CMAKE_CURRENT_BINARY_DIR}/AlopexOS.img
)


set(PREPARE_DISK_SCRIPT
    ${CMAKE_CURRENT_SOURCE_DIR}/scripts/prepare_disk.py
)


set(LIMINE_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/limine.conf
)


set(LIMINE_BIOS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/limine/limine-bios.sys
)


add_custom_command(

    OUTPUT ${DISK_IMAGE}

    COMMAND
        ${Python3_EXECUTABLE}
        ${PREPARE_DISK_SCRIPT}
        ${DISK_IMAGE}
        $<TARGET_FILE:AlopexOS>


    DEPENDS

        AlopexOS
        ${PREPARE_DISK_SCRIPT}
        ${LIMINE_CONFIG}
        ${LIMINE_BIOS}


    COMMENT
        "Generating bootable AlopexOS disk image..."

    VERBATIM
)


add_custom_target(disk_image ALL
    DEPENDS ${DISK_IMAGE}
)


# --------------------------------------------------
# QEMU Run Target
# --------------------------------------------------

add_custom_target(run

    COMMAND
        qemu-system-x86_64
        -drive format=raw,file=${DISK_IMAGE}
        -m 256M
        -boot c
        -no-reboot
        -d guest_errors,unimp
        -D ${CMAKE_CURRENT_BINARY_DIR}/qemu.log

    DEPENDS disk_image

    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}

    COMMENT
        "Launching AlopexOS in QEMU..."

)