
# Jitify

A single-header C++ library for simplifying the use of CUDA Runtime Compilation (NVRTC).

## Rationale

Integrating NVRTC into existing and/or templated CUDA code can be
tricky. Jitify aims to simplify this process by hiding the
complexities behind a simple, high-level interface.

## Quick example

```c++
const char* program_source = "my_program\n"
    "template<int N, typename T>\n"
    "__global__\n"
    "void my_kernel(T* data) {\n"
    "    T data0 = data[0];\n"
    "    for( int i=0; i<N-1; ++i ) {\n"
    "        data[0] *= data0;\n"
    "    }\n"
    "}\n";
static jitify::JitCache kernel_cache;
jitify::Program program = kernel_cache.program(program_source);
// ...set up data etc.
dim3 grid(1);
dim3 block(1);
using jitify::reflection::type_of;
program.kernel("my_kernel")
       .instantiate(3, type_of(*data))
       .configure(grid, block)
       .launch(data);
```

## Features

Jitify provides/takes care of the following things:

 * All NVRTC and CUDA Driver API calls
 * Simple kernel instantiation and launch syntax
 * Caching compiled kernels
 * Loading source code from strings, files, or embedded in an executable
 * Ignoring host code in runtime-compiled sources
 * Skipping unneeded headers
 * Support for JIT-safe standard library headers (e.g., float.h, stdint.h etc.)
 * Dealing with kernel name mangling
 * Reflecting kernel template parameters into strings
 * Compiling specifically for the current device's compute capability
 * Linking to pre-compiled PTX/CUBIN/FATBIN/object/library files
 * Support for CUDA versions 7.0, 7.5, 8.0, 9.x, 10.x, on both Linux and Windows
 * Convenient parallel_for function and lambda support
 * \*New\* jitify::experimental API provides serialization capabilities to enable [user-managed hashing and caching](https://github.com/rapidsai/cudf/blob/v0.12.0/cpp/src/jit/cache.h)

Things you can do with Jitify and NVRTC:

 * *Rapidly port existing code* to use CUDA Runtime Compilation
 * *Dramatically reduce code volume* and offline-compilation times
 * *Increase kernel performance* by baking in runtime constants and autotuning

## How to build

Jitify is just a single header file:

```c++
#include <jitify.hpp>
```

Compile with: `-pthread` (not needed if JITIFY_THREAD_SAFE is defined to 0)

Link with: `-lcuda -lcudart -lnvrtc`

A small utility called stringify is included for converting text files into
C string literals, which provides a convenient way to integrate JIT-compiled
sources into a build.

### Running tests

Tests can be run with the following command:

```shell
$ make test
```

This will automatically download and build the
[GoogleTest](https://github.com/google/googletest) library, which
requires [CMake](https://cmake.org) to be available on the system.

## Documentation

### Examples

See [jitify_example.cpp](jitify_example.cpp) for some examples of how to use the library.
The [Makefile](Makefile) also demonstrates how to use the provided stringify utility.

[GTC 2017 Talk by Ben Barsdell and Kate Clark](https://on-demand.gputechconf.com/gtc/2017/videos/s7716-barsdell-ben-jitify.mp4)

### API documentation

Doxygen documentation can be generated by running:

```shell
$ make doc
```

The HTML and LaTeX results are placed into the doc/ subdirectory.

## License

BSD-3-Clause

## Authors

Ben Barsdell (NVIDIA, bbarsdell at nvidia dot com)

Kate Clark (NVIDIA, mclark at nvidia dot com)
