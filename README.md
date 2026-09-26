# AlopexOS

AlopexOS is an advanced, high-performance, proprietary operating system engineered from the ground up for absolute technical excellence, determinism, and low-level control. Designed to bridge modern high-level abstractions with zero-cost bare-metal execution, AlopexOS incorporates custom subsystems—such as the high-integrity AbtrFS storage filesystem—to deliver maximum reliability and performance.

---

## Table of Contents
1. [Project Documentation & Governance](#project-documentation--governance)
2. [What is AlopexOS?](#what-is-alopexos)
3. [Prerequisites & Dependencies](#prerequisites--dependencies)
4. [Compilation & Build Instructions](#compilation--build-instructions)
5. [Running via QEMU](#running-via-qemu)
6. [Installation Instructions](#installation-instructions)
7. [Contributors](#contributors)

---

## 1. Project Documentation & Governance

To maintain absolute structural clarity, legal control, and architectural purity, the project documentation is partitioned into distinct, authoritative files:

* **[AlopexOSCxxPhilosophy.md](AlopexOSCxxPhilosophy.md)**: Dictates the strict C++ programming standards, architectural core tenets, memory management paradigms, and syntax constraints required across the entire codebase.
* **[LICENSE.md](LICENSE.md)**: Establishes the legal ownership, proprietary licensing tiers (consumer free-use vs. commercial enterprise/government mandates), anti-piracy restrictions, and intellectual property jurisdiction under Project Director Fenn Vinter.
* **[Contributors.md](Contributors.md)**: Formally acknowledges and credits developers and contributors who support the development and expansion of the AlopexOS ecosystem.

---

## 2. What is AlopexOS?

AlopexOS is a closed-source, proprietary operating system project built on strict architectural constraints. It rejects conventional bloat and embraces modern, zero-cost C++ principles to ensure supreme execution speed, hardware predictability, and type safety down to the bare metal. 

* **Licensing Model:** Free for individual end-user consumers for personal applications, while commercial entities, corporations, and government organizations are legally required to purchase a commercial enterprise license. Refer to [LICENSE.md](LICENSE.md) for full legal terms.
* **IP Protection:** All code, documentation, and architecture are exclusively owned and governed by Project Director Fenn Vinter / AurenFox.Studio.

---

## 3. Prerequisites & Dependencies

To build and run AlopexOS from source, your development environment must include the following native tools and dependencies:

* **Compiler Toolchain:** A modern Clang environment enforcing strict C++ standards.
* **Build System:** CMake (compatible with out-of-source builds).
* **Virtualization & Emulation:** QEMU (`qemu-system-x86_64`) and `qemu-img` for creating disk and NVMe backing images.
* **Execution Environment:** Python 3 (required to execute the cross-platform `scripts/build_and_run.py` automation script).

---

## 4. Compilation & Build Instructions

AlopexOS utilizes a automated Python build script (`scripts/build_and_run.py`) to handle directory clean-up, CMake configuration, compilation, image validation, and QEMU execution across platforms.

1. Open a terminal in the root directory of the project.
2. Execute the build script:
   ```bash
   python3 scripts/build_and_run.py
   ```