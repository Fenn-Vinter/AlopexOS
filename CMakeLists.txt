cmake_minimum_required(VERSION 3.25)

# Use Clang's freestanding ELF target during compiler and linker detection.
set(CMAKE_CXX_COMPILER_TARGET x86_64-elf CACHE STRING "Kernel compiler target")
set(CMAKE_ASM_COMPILER_TARGET x86_64-elf CACHE STRING "Kernel assembler target")

project(AlopexOS
    VERSION 0.1
    LANGUAGES CXX ASM
)

set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "No host libraries for the kernel" FORCE)

# --------------------------------------------------
# Tools
# --------------------------------------------------
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(KERNEL_LINKER "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/ld.lld.exe")
set(CMAKE_CXX_LINK_EXECUTABLE
    "\"${KERNEL_LINKER}\" <LINK_FLAGS> <OBJECTS> -o <TARGET>"
)

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
    -ffreestanding
    -fno-exceptions
    -fno-rtti
    -fno-stack-protector
    -fno-stack-check
    -fno-asynchronous-unwind-tables
    -fno-unwind-tables
    -fno-pie
    -fno-PIE
    -fno-pic
    -mcmodel=kernel
    -mno-red-zone
    -mgeneral-regs-only
    -mno-sse
    -mno-sse2
    -fno-omit-frame-pointer
    -Wall
    -Wextra
    -Wpedantic
    -D__limine__
    -D__AlopexOS__
    -D__x86_64__
)

# --------------------------------------------------
# Kernel ELF
# --------------------------------------------------
add_executable(AlopexOS
    src/main.cpp
    src/entry.s
    src/heap.cpp
    src/skeleton.cpp
    src/cxx_abi.cpp
)

set_target_properties(AlopexOS PROPERTIES
    PREFIX ""
    SUFFIX ""
)

target_include_directories(AlopexOS PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/limine
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/limine
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fennlib/include
)

target_compile_options(AlopexOS PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${KERNEL_FLAGS}>
)

# --------------------------------------------------
# Linker
# --------------------------------------------------
target_link_options(AlopexOS PRIVATE
    -m
    elf_x86_64
    "-T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
    --build-id=none
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
    VERBATIM
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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/boot/limine.conf")
    set(LIMINE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/boot/limine.conf)
else()
    set(LIMINE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/limine.conf)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/limine/limine-bios-hd.bin")
    set(LIMINE_BIOS ${CMAKE_CURRENT_SOURCE_DIR}/lib/limine/limine-bios-hd.bin)
else()
    set(LIMINE_BIOS ${CMAKE_CURRENT_SOURCE_DIR}/lib/limine/limine-bios.sys)
endif()

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