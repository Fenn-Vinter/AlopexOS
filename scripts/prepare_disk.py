import os
import sys
import subprocess
import tempfile


IMAGE_SIZE_MB = 64
PARTITION_OFFSET = 1024 * 1024  # 1 MiB = sector 2048


def run(cmd, **kwargs):
    print("[CMD]", " ".join(cmd))
    subprocess.run(cmd, check=True, **kwargs)


def prepare_disk(img_path, kernel_path):

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    limine_dir = os.path.join(project_root, "lib", "limine")
    conf_path = os.path.join(project_root, "limine.conf")

    limine_bin = os.path.join(limine_dir, "limine")
    limine_sys = os.path.join(limine_dir, "limine-bios.sys")


    if not os.path.exists(conf_path):
        print("ERROR: limine.conf missing")
        sys.exit(1)

    if not os.path.exists(limine_bin):
        print("ERROR: Limine binary missing")
        sys.exit(1)

    if not os.path.exists(limine_sys):
        print("ERROR: limine-bios.sys missing")
        sys.exit(1)


    print(f"[*] Creating {IMAGE_SIZE_MB}MB disk image...")

    with open(img_path, "wb") as f:
        f.write(
            b"\0" *
            (IMAGE_SIZE_MB * 1024 * 1024)
        )


    # --------------------------------------------------
    # Partition table
    # --------------------------------------------------

    print("[*] Creating MBR partition table...")

    run([
        "parted",
        "-s",
        img_path,
        "mklabel",
        "msdos"
    ])


    print("[*] Creating FAT16 partition...")

    run([
        "parted",
        "-s",
        img_path,
        "mkpart",
        "primary",
        "fat16",
        "2048s",
        "100%"
    ])


    print("[*] Setting boot flag...")

    run([
        "parted",
        "-s",
        img_path,
        "set",
        "1",
        "boot",
        "on"
    ])


    # --------------------------------------------------
    # FAT16
    # --------------------------------------------------

    print("[*] Formatting FAT16 filesystem...")

    run([
        "mkfs.fat",
        "-F",
        "16",
        "-n",
        "ALOPEX",
        "--offset",
        "2048",
        img_path
    ])


    # --------------------------------------------------
    # mtools
    # --------------------------------------------------

    with tempfile.NamedTemporaryFile(
        mode="w",
        delete=False,
        suffix=".mtools"
    ) as f:

        mtools_path = f.name

        f.write(
            f'drive c: file="{img_path}" offset={PARTITION_OFFSET}\n'
        )


    env = os.environ.copy()
    env["MTOOLSRC"] = mtools_path


    try:

        print("[*] Creating boot directory...")

        run([
            "mmd",
            "C:/boot"
        ], env=env)


        print("[*] Copying kernel...")

        run([
            "mcopy",
            "-o",
            kernel_path,
            "C:/boot/alopexos.elf"
        ], env=env)


        print("[*] Copying Limine BIOS loader...")

        run([
            "mcopy",
            "-o",
            limine_sys,
            "C:/limine-bios.sys"
        ], env=env)


        print("[*] Copying Limine config...")

        run([
            "mcopy",
            "-o",
            conf_path,
            "C:/limine.conf"
        ], env=env)


        # --------------------------------------------------
        # Install Limine MBR stages
        # --------------------------------------------------

        print("[*] Installing Limine BIOS bootloader...")

        run([
            limine_bin,
            "bios-install",
            img_path
        ])


        print("[SUCCESS] AlopexOS image created!")


    finally:

        if os.path.exists(mtools_path):
            os.unlink(mtools_path)



if __name__ == "__main__":

    if len(sys.argv) < 3:
        print(
            "Usage: prepare_disk.py <image> <kernel>"
        )
        sys.exit(1)


    prepare_disk(
        sys.argv[1],
        sys.argv[2]
    )