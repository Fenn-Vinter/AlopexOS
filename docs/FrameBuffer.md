# The AlopexOS FrameBuffer / FrameBuffer.hpp

## This is responsible for
Managing the core software back buffer (screen buffer) to enable smooth double-buffered rendering across the entire AlopexOS lifecycle, completely eliminating screen flicker and visual tearing during pre-boot initialization and runtime.

### Core Responsibilities
* **Back Buffer Allocation:** Maintains a dedicated pixel array in RAM so all drawing operations occur off-screen without slamming live video memory directly.
* **Flicker-Free Rendering:** Isolates draw calls, text flushing, and graphic indicators entirely to memory before a single atomic swap or copy to the physical Limine hardware framebuffer (`fb->address`).
* **Resolution Independence:** Dynamically scales or configures based on the width, height, and pitch provided by the bootloader environment.
* **Long-Term Persistence:** Acts as the foundational display pipeline that transitions seamlessly from early boot diagnostics into the main kernel window compositor.

### Technical Overview
* **Front Buffer (Hardware):** The physical memory address mapped by the GPU/UEFI via Limine.
* **Back Buffer (Software):** A local pixel buffer stored safely in RAM to aggregate all visual changes per frame.
* **The Swap / Flush:** A fast memory transfer step that pushes the finalized back buffer onto the front buffer during the vertical blanking or frame boundary.