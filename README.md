# Desolation Engine

**Desolation Engine** is a game engine built as part of a Computer Graphics course at ITMO University. It runs on **Windows** and **Linux**, utilizing **DirectX 11** and **SDL3** for rendering and window management.

On Linux, DirectX 11 calls are translated to Vulkan via **DXVK**, achieving near-native performance.

---

## Build Instructions

### Prerequisites

Make sure you have the following installed:

- [CMake](https://cmake.org/)
- [Clang](https://clang.llvm.org/)
- [just](https://github.com/casey/just) - a command runner
- All dependencies required for building **SDL3**  
  See the official guide: [SDL3 Linux build instructions](https://wiki.libsdl.org/SDL3/README-linux)

### Building and Running

To build and run the project in **debug mode**:

```bash
just r
```

To create a **release build**:

```bash
just br
```
