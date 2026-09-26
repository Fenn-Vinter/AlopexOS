# DRAMA: <p>(Dynamic Random Access Memory Allocator)</p>

DRAMA is the backend of AlopexOS of which takes care of, and handles all Random Access Memory allocations, mounting, discovery, memory migration, lockdowns, dismounts and Memory compaction

## Memory Allocation

DRAMA takes care of all memory allocation. It does so by allocating an array of bytes, occupying the entire RAM-stick. Once done, programs can request a chunk of memory via malloc. The returned memory address and size is equivelant to the term virtual memory. As it is simply a cover up to how much there really is. This is to prevent any collisions with other programs and to ensure that the programs don't accidentally or maliciously modify existing memory outside its scope. Another use for this is to also prevent a memory crash or as famously reffered to within the windows operating system as the "blue screen of death" or just BSOD.

### Malloc Signature:
```cpp
inline auto AlopexOS::Kernel::DRAMA::malloc(fennlib::types::usize size_bytes, DeviceID ram_device_id = static_cast<DeviceID>(-1)) -> RAMCellID;
```

`size_bytes` specifies the amount of memory the program wishes to use, while `ram_device_id` takes in a DeviceID argument to specify which specific RAM module or device to target (with `-1` acting as a sentinel for automatic handling).

the return value of the malloc function is important for retrieving the requested and allocated VRAM as well as for terminating the VRAM to relieve the system.

The last 100 entries `static_cast<DeviceID>(-100)` in the RAMCellID is reserved for error codes and is not a real nor existing ID. The most typical error of which could be failiure to allocate memory due to lack of free room.

## RAM Mounting

Mounting a RAM module aka RAM-stick is very important before it is to be used at all. When a RAM is "mounted", it allocates memory, occupying the whole module using `fennlib::sequence`. This allows us to check the amount of RAM, the amount of used RAM, and safely retrieve and assign VRAM.

### Mount Signature:
```cpp
inline auto AlopexOS::Kernel::DRAMA::mount(DeviceID ram_device_id) -> AlopexOS::error_code;
```

`ram_device_id` takes in a DeviceID argument to specify which specific RAM module or device to mount.

The return value of the mount is an enumerator `AlopexOS::error_code`. It's good practice to check whether or not the mounting succeeded. If this fails, it could be due to an incompatibility issue with the RAM module or a RAM module failiure or corruption.

## RAM Unmounting

Before attempting to remove the physical RAM module, make sure to unmount the module to avoid an accidental BSOD-like error or accidental loss of critical data. Unmounting frees the underlying hardware tracking block and ensures no active allocations remain stranded on the device.

### Unmount Signature:
```cpp
inline auto AlopexOS::Kernel::DRAMA::unmount(DeviceID ram_device_id) -> AlopexOS::error_code;
```

`ram_device_id` specifies which hardware module to disconnect. The return value indicates success or failure (such as returning an error if active, locked cells are still residing on the stick).

## RAM Migration

Before attempting to unmount RAM or remove a physical module, it is wise to migrate existing VRAM cells over to another stick to avoid memory corruption and fatal crashes.

### Migration Signature:
```cpp
inline auto AlopexOS::Kernel::DRAMA::migrate(RAMCellID ram_cell_id, DeviceID ram_device_id = static_cast<DeviceID>(-1)) -> RAMCellID;
```

`ram_cell_id` targets the specific virtual memory chunk you want to move, while `ram_device_id` specifies the destination stick. Passing `-1` tells DRAMA to automatically find and select the best available target module to migrate the data into.

## RAM Module Discovery

DRAMA uses Synapse in order to scan for and filter trough different RAM Modules and mounts them automatically.

### Scan Signature
```cpp
inline auto AlopexOS::Kernel::DRAMA::scan() -> AlopexOS::error_code;
```

This function signature takes no parameter and simply returns its status