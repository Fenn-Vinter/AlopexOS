# AlopexOS C++ & Project Philosophy

AlopexOS adopts modern C++ for the entire OS project, balancing high-level abstractions, safety, and zero-cost principles with strict determinism and low-level control.

## 1. Core Tenets

* **Zero-Overhead Abstraction:** C++ features must compile down to code that is as efficient as hand-written C. If an abstraction introduces hidden runtime costs, locks, or uncontrolled allocations, it has no place in the operating system.

* **Deterministic Resource Management (RAII):** Wherever possible, resource lifecycles—such as memory buffers, device locks, and DMA handles—are tied to object scopes. Resources are acquired on construction and safely released on destruction.

* **Type Safety over C-Style Casting:** Leverage strong typing, scoped enumerations, and explicit conversions (`static_cast`, `reinterpret_cast`) to catch structural errors at compile-time rather than runtime.

## 2. Language, Types, and Structure Constraints

* **Strict Ban on Typedef:** The `typedef` keyword is strictly prohibited in C++ code. Type aliases must be declared exclusively using type alias syntax (`using`).

* **Prohibition of Legacy C Features:** C-style features, idioms, and standard headers are banned in C++ source files unless no C++ equivalent or alternative has been provided.

* **Unified Primitives over Standard Headers:** Standard library headers like `<cstddef>`, `<stdint.h>`, or `<stdint>` are banned. All fundamental type definitions and platform primitives must utilize the project-provided `primitives.h`, which is fully compatible with both C and C++ environments.

* **Generalized Dataflow:** Abstractions and interfaces must prioritize generalized, decoupled dataflow to maintain clean boundaries between kernel subsystems, drivers, and filesystems.

## 3. Code Style, Headers, and Syntax Rules

* **Indentation Limit:** Developers are restricted to a maximum of **three levels of indentation (MAX)**. Exceeding three levels is considered bad practice and must be refactored.

* **Header Guards:** Instead of `#pragma once`, header files must explicitly use standard inclusion guards (`#if !defined(...)`) to enable files to check inclusion states for APIs and overall modularity.

* **Function Declaration Syntax:** C-style function declarations are strictly banned. Functions must be declared using the `fn` keyword from `primitives.h` (an alias for `auto`, used exclusively for functions) combined with trailing return types (`->`). 
  * Example: `fn math(uint x, opcode op, uint y) -> uint`

* **Separation of Interface and Implementation:** Implementation is strictly prohibited in `.hpp` or `.h` header files. All implementation logic must reside exclusively in `.cpp` files.

* **Namespaces and Classes:** Classes are not permitted to be defined directly inside a namespace; they must be forward declared, then defined outside the namespace. Additionally, developers are not permitted to declare a namespace in a `.cpp` file or define elements inside a namespace (only declarations are permitted).

* **The Rule of Five:** When defining classes that manage resources, developers must explicitly implement or explicitly delete the Rule of Five special member functions: the destructor, copy constructor, copy assignment operator, move constructor, and move assignment operator. This ensures proper, predictable resource ownership and prevents shallow copy bugs or memory leaks across the OS.

* **Anti-Lazy Macro Policy:** The use of preprocessor macros to avoid writing proper code, type-safe functions, or explicit logic is strictly forbidden and considered lazy development.

* **Early Failing and Elimination of Else:** Control flow must prioritize early failing (guard clauses that return or exit early on error states). The use of the `else` keyword is considered bad practice and is banned to maintain linear, highly readable execution paths.

* **Standardized Attribute Usage:** Standardized attribute specifiers using double square brackets (`[[]]`)—such as `[[noreturn]]`, `[[deprecated]]`, or `[[nodiscard]]`—must be utilized wherever applicable to enforce compiler warnings, optimizations, and API intent.

## 4. Tooling, Legal, Ownership, and Accountability

* **Prohibition of Third-Party Tools:** The reliance on third-party tools is considered bad practice. They introduce risks of version mismatches, build instability, and legal or copyright complications. Internal or tightly controlled native tooling must be prioritized.
