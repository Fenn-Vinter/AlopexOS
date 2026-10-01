import os
import sys
import subprocess
import struct

IMAGE_SIZE_MB = 64
SECTOR_SIZE = 512
PARTITION_START_SECTOR = 2048  # 1 MiB alignment offset

def run(cmd, **kwargs):
    print("[CMD]", " ".join(cmd))
    subprocess.run(cmd, check=True, **kwargs)

def short_name_for(filename):
    basename = filename.replace("\\", "/").split("/")[-1]
    parts = basename.upper().split(".", 1)
    base = parts[0]
    ext = parts[1] if len(parts) > 1 else ""

    if len(base) <= 8 and len(ext) <= 3:
        return base.ljust(8) + ext.ljust(3)

    return (base[:6] + "~1").ljust(8) + ext[:3].ljust(3)

def lfn_checksum(short_name):
    checksum = 0
    for value in short_name.encode("ascii"):
        checksum = ((checksum >> 1) | ((checksum & 1) << 7))
        checksum = (checksum + value) & 0xFF
    return checksum

def make_directory_entries(filename, short_name, is_subdir=False, cluster=0, size=0):
    parts = filename.upper().split(".", 1)
    extension = parts[1] if len(parts) > 1 else ""
    
    attr = 0x10 if is_subdir else 0x20
    clean_short_name = short_name.ljust(11)[:11]
    
    if len(parts[0]) <= 8 and len(extension) <= 3:
        entry = bytearray(32)
        entry[0:11] = clean_short_name.encode("ascii")
        entry[11] = attr
        struct.pack_into("<H", entry, 26, cluster)
        struct.pack_into("<I", entry, 28, size)
        return [entry]

    encoded = filename.encode("utf-16le")
    characters = [encoded[index:index + 2] for index in range(0, len(encoded), 2)]
    characters.append(b"\x00\x00")
    chunks = [characters[index:index + 13] for index in range(0, len(characters), 13)]
    checksum = lfn_checksum(clean_short_name)
    entries = []

    for chunk_index in range(len(chunks) - 1, -1, -1):
        chunk = chunks[chunk_index]
        entry = bytearray(32)
        entry[0] = (chunk_index + 1) | (0x40 if chunk_index == len(chunks) - 1 else 0)
        entry[11] = 0x0F
        entry[13] = checksum
        entry[26:28] = b"\x00\x00"

        padded = chunk + [b"\xFF\xFF"] * (13 - len(chunk))
        for offset, value in zip((1, 3, 5, 7, 9), padded[:5]):
            entry[offset:offset + 2] = value
        for offset, value in zip((14, 16, 18, 20, 22, 24), padded[5:11]):
            entry[offset:offset + 2] = value
        for offset, value in zip((28, 30), padded[11:13]):
            entry[offset:offset + 2] = value
        entries.append(entry)

    short_entry = bytearray(32)
    short_entry[0:11] = clean_short_name.encode("ascii")
    short_entry[11] = attr
    struct.pack_into("<H", short_entry, 26, cluster)
    struct.pack_into("<I", short_entry, 28, size)
    entries.append(short_entry)
    return entries

def format_fat16_and_write_files(f, start_sector, num_sectors, files_to_copy):
    """Formats the specified sector range as FAT16 and writes nested boot files into the raw image."""
    sectors_per_cluster = 4
    bytes_per_cluster = sectors_per_cluster * SECTOR_SIZE
    reserved_sectors = 1
    num_fats = 2
    root_entries = 512
    root_dir_sectors = (root_entries * 32) // SECTOR_SIZE
    
    tmp_data_sectors = num_sectors - reserved_sectors - root_dir_sectors
    sectors_per_fat = max(1, (tmp_data_sectors // sectors_per_cluster // (SECTOR_SIZE // 2)))
    
    data_start_sector = start_sector + reserved_sectors + (num_fats * sectors_per_fat) + root_dir_sectors

    # 1. Volume Boot Record (VBR)
    f.seek(start_sector * SECTOR_SIZE)
    vbr = bytearray(SECTOR_SIZE)
    vbr[0:3] = b"\xEB\x3C\x90"
    vbr[3:11] = b"ALOPEXOS"
    struct.pack_into("<H", vbr, 11, SECTOR_SIZE)
    vbr[13] = sectors_per_cluster
    struct.pack_into("<H", vbr, 14, reserved_sectors)
    vbr[16] = num_fats
    struct.pack_into("<H", vbr, 17, root_entries)
    struct.pack_into("<H", vbr, 19, 0 if num_sectors >= 65536 else num_sectors)
    vbr[21] = 0xF8
    struct.pack_into("<H", vbr, 22, sectors_per_fat)
    struct.pack_into("<I", vbr, 32, num_sectors if num_sectors >= 65536 else 0)
    vbr[36] = 0x80
    vbr[38] = 0x29
    vbr[39:43] = b"\x12\x34\x56\x78"
    vbr[43:54] = b"ALOPEXOS   "
    vbr[54:62] = b"FAT16   "
    vbr[510:512] = b"\x55\xAA"
    f.write(vbr)

    # 2. File Allocation Tables
    fat_bytes = bytearray(sectors_per_fat * SECTOR_SIZE)
    struct.pack_into("<H", fat_bytes, 0, 0xFFF8)
    struct.pack_into("<H", fat_bytes, 2, 0xFFFF)
    
    fat_start_sector = start_sector + reserved_sectors
    for i in range(num_fats):
        f.seek((fat_start_sector + (i * sectors_per_fat)) * SECTOR_SIZE)
        f.write(fat_bytes)

    # Organize files into directory hierarchy
    # Root directory entries list
    root_entries_list = []
    efi_entries_list = []
    boot_entries_list = []

    current_cluster = 2

    def allocate_file_clusters(file_path):
        nonlocal current_cluster, fat_bytes
        if not os.path.exists(file_path):
            print(f"[!] Error: Boot file missing at: {file_path}")
            sys.exit(1)
        with open(file_path, "rb") as sf:
            file_data = sf.read()
        file_size = len(file_data)
        clusters_needed = (file_size + bytes_per_cluster - 1) // bytes_per_cluster
        
        start_cl = current_cluster
        for cl_idx in range(clusters_needed):
            cluster_num = current_cluster + cl_idx
            next_cluster = (cluster_num + 1) if cl_idx < clusters_needed - 1 else 0xFFFF
            struct.pack_into("<H", fat_bytes, cluster_num * 2, next_cluster)

            cluster_sector = data_start_sector + ((cluster_num - 2) * sectors_per_cluster)
            f.seek(cluster_sector * SECTOR_SIZE)
            chunk = file_data[cl_idx * bytes_per_cluster : (cl_idx + 1) * bytes_per_cluster]
            f.write(chunk)
        
        current_cluster += clusters_needed
        return start_cl, file_size

    # Process files
    for virtual_path, src_path in files_to_copy.items():
        parts = virtual_path.replace("\\", "/").split("/")
        if len(parts) == 1:
            cl, sz = allocate_file_clusters(src_path)
            s_name = short_name_for(parts[0])
            for entry in make_directory_entries(parts[0], s_name, False, cl, sz):
                root_entries_list.append(entry)
        elif len(parts) == 3 and parts[0].upper() == "EFI" and parts[1].upper() == "BOOT":
            cl, sz = allocate_file_clusters(src_path)
            s_name = short_name_for(parts[2])
            for entry in make_directory_entries(parts[2], s_name, False, cl, sz):
                boot_entries_list.append(entry)

    # Create EFI directory cluster if we have boot files
    efi_cluster = 0
    if boot_entries_list:
        efi_cluster = current_cluster
        # Allocate cluster for EFI dir
        struct.pack_into("<H", fat_bytes, efi_cluster * 2, 0xFFFF)
        current_cluster += 1

        # BOOT subdirectory inside EFI
        boot_cluster = current_cluster
        struct.pack_into("<H", fat_bytes, boot_cluster * 2, 0xFFFF)
        current_cluster += 1

        # Write BOOT contents into its cluster sector
        boot_sector = data_start_sector + ((boot_cluster - 2) * sectors_per_cluster)
        f.seek(boot_sector * SECTOR_SIZE)
        boot_dir_bytes = bytearray(bytes_per_cluster)
        offset = 0
        for entry in boot_entries_list:
            boot_dir_bytes[offset:offset+32] = entry
            offset += 32
        f.write(boot_dir_bytes)

        # Write EFI contents (containing BOOT dir entry)
        efi_sector = data_start_sector + ((efi_cluster - 2) * sectors_per_cluster)
        f.seek(efi_sector * SECTOR_SIZE)
        efi_dir_bytes = bytearray(bytes_per_cluster)
        boot_dir_entries = make_directory_entries("BOOT", "BOOT    ", True, boot_cluster, 0)
        offset = 0
        for entry in boot_dir_entries:
            efi_dir_bytes[offset:offset+32] = entry
            offset += 32
        f.write(efi_dir_bytes)

        # Add EFI entry to root
        efi_dir_entries_root = make_directory_entries("EFI", "EFI     ", True, efi_cluster, 0)
        for entry in efi_dir_entries_root:
            root_entries_list.append(entry)

    # Write Root Directory
    root_dir_start_sector = fat_start_sector + (num_fats * sectors_per_fat)
    root_dir_bytes = bytearray(root_entries * 32)
    offset = 0
    for entry in root_entries_list:
        if offset < len(root_dir_bytes):
            root_dir_bytes[offset:offset+32] = entry
            offset += 32

    f.seek(root_dir_start_sector * SECTOR_SIZE)
    f.write(root_dir_bytes)
    
    # Write FAT tables back
    for i in range(num_fats):
        f.seek((fat_start_sector + (i * sectors_per_fat)) * SECTOR_SIZE)
        f.write(fat_bytes)

def write_mbr(f, total_sectors):
    f.seek(0x1BE)
    partition_entry = struct.pack(
        "<B3sB3sII",
        0x80,               # Bootable active partition
        b"\x00\x00\x00",
        0x0E,               # FAT16 LBA
        b"\x00\x00\x00",
        PARTITION_START_SECTOR,
        total_sectors - PARTITION_START_SECTOR
    )
    f.write(partition_entry)
    f.write(b"\0" * (16 * 3))
    f.write(b"\x55\xAA")

def prepare_disk(img_path, kernel_path):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    limine_dir = os.path.join(project_root, "lib", "limine")

    conf_path = os.path.join(project_root, "boot", "limine.conf")
    if not os.path.exists(conf_path):
        conf_path = os.path.join(project_root, "limine.conf")

    limine_sys = os.path.join(limine_dir, "limine-bios.sys")
    if not os.path.exists(limine_sys):
        limine_sys = os.path.join(limine_dir, "limine-bios-hd.bin")

    limine_efi = os.path.join(limine_dir, "BOOTX64.EFI")
    limine_bin = os.path.join(limine_dir, "limine-tool-windows-x86", "limine.exe")

    print(f"[*] Creating {IMAGE_SIZE_MB}MB raw image...")
    total_bytes = IMAGE_SIZE_MB * 1024 * 1024
    total_sectors = total_bytes // SECTOR_SIZE

    with open(img_path, "wb+") as f:
        f.write(b"\0" * total_bytes)
        write_mbr(f, total_sectors)

        files = {
            "limine-bios.sys": limine_sys,
            "limine.conf": conf_path,
            "AlopexOS": kernel_path
        }
        
        if os.path.exists(limine_efi):
            files["EFI/BOOT/BOOTX64.EFI"] = limine_efi

        print("[*] Formatting FAT partition and building UEFI directory structures...")
        format_fat16_and_write_files(f, PARTITION_START_SECTOR, total_sectors - PARTITION_START_SECTOR, files)

    print("[*] Deploying Limine BIOS bootloader...")
    if os.path.exists(limine_bin):
        run([limine_bin, "bios-install", img_path])
    
    print("[SUCCESS] Disk image ready for UEFI and BIOS via Ventoy!")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: prepare_disk.py <image> <kernel>")
        sys.exit(1)
    prepare_disk(sys.argv[1], sys.argv[2])