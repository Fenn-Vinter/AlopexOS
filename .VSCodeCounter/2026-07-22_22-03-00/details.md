# Details

Date : 2026-07-22 22:03:00

Directory /home/fenn/Desktop/AlopexOS

Total : 42 files,  3297 codes, 47 comments, 803 blanks, all 4147 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [build_and_run.sh](/build_and_run.sh) | Shell Script | 50 | 8 | 10 | 68 |
| [include/AlopexOS/ACPI/acpi.hpp](/include/AlopexOS/ACPI/acpi.hpp) | C++ | 55 | 0 | 12 | 67 |
| [include/AlopexOS/ACPI/ecam.hpp](/include/AlopexOS/ACPI/ecam.hpp) | C++ | 19 | 0 | 8 | 27 |
| [include/AlopexOS/ACPI/mcfg.hpp](/include/AlopexOS/ACPI/mcfg.hpp) | C++ | 6 | 0 | 2 | 8 |
| [include/AlopexOS/AlopexIBus/AlopexIBus.hpp](/include/AlopexOS/AlopexIBus/AlopexIBus.hpp) | C++ | 81 | 0 | 19 | 100 |
| [include/AlopexOS/AlopexOS.hpp](/include/AlopexOS/AlopexOS.hpp) | C++ | 18 | 4 | 8 | 30 |
| [include/AlopexOS/AlopexOS_ErrorCodes.hpp](/include/AlopexOS/AlopexOS_ErrorCodes.hpp) | C++ | 37 | 5 | 10 | 52 |
| [include/AlopexOS/PCI/nvme/nvme.hpp](/include/AlopexOS/PCI/nvme/nvme.hpp) | C++ | 50 | 1 | 15 | 66 |
| [include/AlopexOS/PCI/nvme/nvme_regs.hpp](/include/AlopexOS/PCI/nvme/nvme_regs.hpp) | C++ | 113 | 0 | 11 | 124 |
| [include/AlopexOS/PCI/pci.hpp](/include/AlopexOS/PCI/pci.hpp) | C++ | 80 | 0 | 12 | 92 |
| [include/AlopexOS/PCI/pcie.hpp](/include/AlopexOS/PCI/pcie.hpp) | C++ | 31 | 0 | 9 | 40 |
| [include/AlopexOS/abtrfs/abtrfs.hpp](/include/AlopexOS/abtrfs/abtrfs.hpp) | C++ | 29 | 0 | 8 | 37 |
| [include/AlopexOS/abtrfs/format.hpp](/include/AlopexOS/abtrfs/format.hpp) | C++ | 67 | 0 | 13 | 80 |
| [include/AlopexOS/abtrfs/partition_header.hpp](/include/AlopexOS/abtrfs/partition_header.hpp) | C++ | 30 | 0 | 6 | 36 |
| [include/AlopexOS/display.hpp](/include/AlopexOS/display.hpp) | C++ | 42 | 0 | 9 | 51 |
| [include/AlopexOS/gaossd/gaossd.hpp](/include/AlopexOS/gaossd/gaossd.hpp) | C++ | 70 | 0 | 17 | 87 |
| [include/AlopexOS/gaossd/readme.md](/include/AlopexOS/gaossd/readme.md) | Markdown | 1 | 0 | 0 | 1 |
| [include/AlopexOS/limine_requests.hpp](/include/AlopexOS/limine_requests.hpp) | C++ | 8 | 0 | 3 | 11 |
| [include/arr.hpp](/include/arr.hpp) | C++ | 203 | 0 | 43 | 246 |
| [include/new.hpp](/include/new.hpp) | C++ | 12 | 0 | 6 | 18 |
| [include/primitives.h](/include/primitives.h) | C++ | 71 | 0 | 21 | 92 |
| [include/string.hpp](/include/string.hpp) | C++ | 72 | 0 | 18 | 90 |
| [limine.conf](/limine.conf) | Properties | 4 | 0 | 1 | 5 |
| [scripts/prepare_disk.py](/scripts/prepare_disk.py) | Python | 129 | 12 | 65 | 206 |
| [src/ACPI/acpi.cpp](/src/ACPI/acpi.cpp) | C++ | 23 | 0 | 9 | 32 |
| [src/ACPI/ecam.cpp](/src/ACPI/ecam.cpp) | C++ | 69 | 0 | 22 | 91 |
| [src/ACPI/mcfg.cpp](/src/ACPI/mcfg.cpp) | C++ | 4 | 0 | 1 | 5 |
| [src/AlopexIBus/AlopexIBus.cpp](/src/AlopexIBus/AlopexIBus.cpp) | C++ | 160 | 0 | 39 | 199 |
| [src/PCI/nvme/nvme.cpp](/src/PCI/nvme/nvme.cpp) | C++ | 463 | 14 | 113 | 590 |
| [src/PCI/pcie.cpp](/src/PCI/pcie.cpp) | C++ | 150 | 0 | 35 | 185 |
| [src/abtrfs/abtrfs.cpp](/src/abtrfs/abtrfs.cpp) | C++ | 112 | 0 | 29 | 141 |
| [src/drivers/nvme/nvme.cpp](/src/drivers/nvme/nvme.cpp) | C++ | 139 | 2 | 37 | 178 |
| [src/drivers/nvme/nvme_command.hpp](/src/drivers/nvme/nvme_command.hpp) | C++ | 30 | 0 | 5 | 35 |
| [src/drivers/nvme/nvme_completion.hpp](/src/drivers/nvme/nvme_completion.hpp) | C++ | 22 | 0 | 5 | 27 |
| [src/entry.s](/src/entry.s) | x86 and x86_64 Assembly | 17 | 0 | 10 | 27 |
| [src/gaossd/gaossd.cpp](/src/gaossd/gaossd.cpp) | C++ | 127 | 0 | 27 | 154 |
| [src/kernel/cxx_abi.cpp](/src/kernel/cxx_abi.cpp) | C++ | 29 | 0 | 9 | 38 |
| [src/kernel/display.cpp](/src/kernel/display.cpp) | C++ | 167 | 0 | 36 | 203 |
| [src/kernel/limine_requests.cpp](/src/kernel/limine_requests.cpp) | C++ | 25 | 0 | 4 | 29 |
| [src/kernel/new.cpp](/src/kernel/new.cpp) | C++ | 122 | 1 | 36 | 159 |
| [src/main.cpp](/src/main.cpp) | C++ | 107 | 0 | 24 | 131 |
| [src/string.cpp](/src/string.cpp) | C++ | 253 | 0 | 36 | 289 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)