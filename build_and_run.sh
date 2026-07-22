#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
IMAGE_PATH="$BUILD_DIR/AlopexOS.img"

echo "========================================="
echo "  AlopexOS Clean Build & Run Script"
echo "========================================="

# 1. Clean Build Directory
echo "[1/5] Cleaning build directory..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
    echo "  - Removed old build directory."
else
    echo "  - No old build directory found."
fi

# 2. Create New Build Directory
echo "[2/5] Creating new build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 3. Configure with CMake
echo "[3/5] Configuring with CMake..."
cmake ..

# 4. Build Project
echo "[4/5] Building project..."
cmake --build .

# 5. Verify Image
echo "[5/5] Verifying disk image..."# 6. Launch QEMU
if [ ! -f "$IMAGE_PATH" ]; then
    echo "ERROR: Disk image not found at $IMAGE_PATH"
    exit 1
fi
echo "  - Image created successfully: $IMAGE_PATH"
ls -lh "$IMAGE_PATH"

# 6. Launch QEMU
echo "========================================="
echo "  Launching QEMU..."
echo "  (Press Ctrl+A then X to exit)"
echo "========================================="
qemu-img create -f raw "$BUILD_DIR/nvme.img" 64M
qemu-system-x86_64 \
    -M q35 \
    -drive format=raw,file="$IMAGE_PATH" \
    -drive file="$BUILD_DIR/nvme.img",id=nvm1,format=raw,if=none \
    -device nvme,id=ctrl0,serial=ALOPEX_NVME_01 \
    -device nvme-ns,drive=nvm1,bus=ctrl0,nsid=1 \
    -m 256M \
    -boot c \
    -no-reboot \
    -chardev stdio,id=char0,mux=on \
    -serial chardev:char0 \
    -monitor chardev:char0 \
    -d int,cpu_reset \
    -D "$BUILD_DIR/qemu.log"

echo ""
echo "QEMU session ended."
echo "Check $BUILD_DIR/qemu.log for debug output."