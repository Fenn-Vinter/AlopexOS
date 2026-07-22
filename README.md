# AlopexOS

AlopexOS is an advanced, high-performance, proprietary operating system engineered from the ground up for absolute technical excellence, determinism, and low-level control. Designed to bridge modern high-level abstractions with zero-cost bare-metal execution, AlopexOS incorporates custom subsystems—such as the high-integrity AbtrFS storage filesystem—to deliver maximum reliability and performance.

---

## Table of Contents
1. [Project Documentation & Governance](#project-documentation--governance)
2. [What is AlopexOS?](#what-is-alopexos)
3. [Compilation Instructions](#compilation-instructions)
4. [Installation Instructions](#installation-instructions)
5. [Contributors](#contributors)

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

## 3. Compilation Instructions

Due to the strict prohibition against third-party tools, external dependency risks, and legacy patterns enforced in [AlopexOSCxxPhilosophy.md](AlopexOSCxxPhilosophy.md), building AlopexOS requires internal or tightly controlled native tooling:

* **Prerequisites:** Ensure your environment utilizes a modern C++ compiler enforcing strict modern standards compatible with project guidelines. Standard library headers like `<stdint.h>` or `<cstddef>` are banned in favor of the project-provided `primitives.h`.
* **Build Sequence:** 
  1. Verify your build environment relies exclusively on native or approved internal tools (third-party build automation frameworks introducing external version risks are prohibited).
  2. Compile interface implementations strictly located in `.cpp` files, ensuring no implementation logic resides inside header files.
  3. Ensure all compilation units adhere to early-failing control flow, the absence of the `else` keyword, and explicit application of the Rule of Five for resource-managing classes.

---

## 4. Installation Instructions

* **Consumer Systems:** Individual consumers running approved hardware setups can deploy AlopexOS utilizing native boot media generated through official project distribution channels.
* **Enterprise & Government Deployments:** Institutional and commercial deployments require verified acquisition of a commercial enterprise license before system integration, as mandated in [LICENSE.md](LICENSE.md).
* **Storage Configuration:** AlopexOS utilizes its native high-integrity block-level storage filesystem (**AbtrFS**) to manage file structures and data integrity post-installation.

---

## 5. Contributors

* **Project Director:** FoxGaming208 (Fenn Vinter)

View the complete list of maintainers and developers in [Contributors.md](Contributors.md).

---

Copyright © Fenn Vinter / AurenFox.Studio. All Rights Reserved.