#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys
from pathlib import Path

def main() -> None:
    # Since the script is located in the scripts/ subdirectory, 
    # project_root should point to the parent directory (AlopexOS root).
    project_root = Path(__file__).resolve().parent.parent
    build_dir = project_root / "build"
    image_path = build_dir / "AlopexOS.img"
    nvme_image = build_dir / "nvme.img"
    log_path = build_dir / "qemu.log"

    print("=========================================")
    print("  AlopexOS Clean Build & Run Script")
    print("=========================================")

    # 1. Clean Build Directory
    print("[1/5] Cleaning build directory...")
    if build_dir.exists():
        shutil.rmtree(build_dir)
        print("  - Removed old build directory.")
    else:
        print("  - No old build directory found.")

    # 2. Create New Build Directory
    print("[2/5] Creating new build directory...")
    build_dir.mkdir(parents=True, exist_ok=True)

    # 3. Configure with CMake (pointing back to project root from build/)
    print("[3/5] Configuring with CMake...")
    subprocess.run(["cmake", ".."], cwd=build_dir, check=True)

    # 4. Build Project
    print("[4/5] Building project...")
    subprocess.run(["cmake", "--build", "."], cwd=build_dir, check=True)

    # 5. Verify Image
    print("[5/5] Verifying disk image...")
    if not image_path.is_file():
        print(f"ERROR: Disk image not found at {image_path}", file=sys.stderr)
        sys.exit(1)
    
    print(f"  - Image created successfully: {image_path}")
    
    # Print file size info portably
    file_size_mb = image_path.stat().st_size / (1024 * 1024)
    print(f"  - Size: {file_size_mb:.2f} MB")

    # 6. Launch QEMU
    print("=========================================")
    print("  Launching QEMU...")
    print("  (Press Ctrl+A then X to exit)")
    print("=========================================")

    # Create NVMe backing image using qemu-img if available
    subprocess.run(["qemu-img", "create", "-f", "raw", str(nvme_image), "64M"], check=True)

    qemu_cmd = [
        "qemu-system-x86_64",
        "-M", "q35",
        "-drive", f"format=raw,file={image_path}",
        "-drive", f"file={nvme_image},id=nvm1,format=raw,if=none",
        "-device", "nvme,id=ctrl0,serial=ALOPEX_NVME_01",
        "-device", "nvme-ns,drive=nvm1,bus=ctrl0,nsid=1",
        "-m", "256M",
        "-boot", "c",
        "-no-reboot",
        "-chardev", "stdio,id=char0,mux=on",
        "-serial", "chardev:char0",
        "-monitor", "chardev:char0",
        "-d", "int,cpu_reset",
        "-D", str(log_path)
    ]

    try:
        subprocess.run(qemu_cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"ERROR: QEMU exited with code {e.returncode}", file=sys.stderr)
        sys.exit(e.returncode)

    print("")
    print("QEMU session ended.")
    print(f"Check {log_path} for debug output.")

if __name__ == "__main__":
    main()