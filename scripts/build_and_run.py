import os
import sys
import subprocess
import shutil
from pathlib import Path

def run_cmd(cmd, **kwargs):
    print("[RUN]", " ".join(cmd))
    subprocess.run(cmd, check=True, **kwargs)

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    build_dir = os.path.join(project_root, "build")

    # Clean build directory to clear any stale CMake cache
    build_path = Path(build_dir)
    if build_path.exists():
        print("[*] Removing old build/ directory for a clean build...")
        shutil.rmtree(build_path)

    print("[*] Configuring CMake project with Clang toolchain...")
    
    # Use generic tool names so they resolve cleanly from the system PATH (e.g., MSYS2 UCRT64)
    cmake_cmd = [
        "cmake",
        "-B", build_dir,
        "-G", "Ninja",
        "-DCMAKE_C_COMPILER=clang",
        "-DCMAKE_CXX_COMPILER=clang++",
        "-DCMAKE_LINKER=ld.lld",
        "-DCMAKE_OBJCOPY=llvm-objcopy"
    ]

    run_cmd(cmake_cmd)

    print("[*] Building project...")
    run_cmd(["cmake", "--build", build_dir])

    print("[*] Launching QEMU...")
    run_cmd(["cmake", "--build", build_dir, "--target", "run"])

if __name__ == "__main__":
    main()