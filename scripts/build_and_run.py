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

    
    build_path = Path(build_dir)
    if build_path.exists():
        print("[*] Removing old build/ directory for a clean build...")
        shutil.rmtree(build_path)

    
    print("[*] Configuring CMake project with Clang toolchain...")
    run_cmd([
        "cmake",
        "-B", build_dir,
        "-G", "Ninja",
        "-DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/clang.exe",
        "-DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/clang++.exe",
        "-DCMAKE_LINKER=C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/ld.lld.exe",
        "-DCMAKE_OBJCOPY=C:/msys64/ucrt64/bin/llvm-objcopy.exe"
    ])

    
    print("[*] Building project...")
    run_cmd(["cmake", "--build", build_dir])

    
    print("[*] Launching QEMU...")
    run_cmd(["cmake", "--build", build_dir, "--target", "run"])

if __name__ == "__main__":
    main()