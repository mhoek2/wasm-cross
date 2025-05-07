# wasm-cross

[![build](https://github.com/mhoek2/wasm-cross/actions/workflows/build.yml/badge.svg)](https://github.com/mhoek2/wasm-cross/actions/workflows/build.yml)

This demo shows a core principles of WebAssembly compiling using **Emscripten**. \
I am using **[Github CI](https://github.com/mhoek2/wasm-cross/blob/main/.github/workflows/build.yml)** to cross-compile for **WebAssembly** and native a **[Windows x86 & x64](https://github.com/mhoek2/wasm-cross/releases)** application.

> Emscripten compiles C and C++ to [WebAssembly](https://webassembly.org/) using
[LLVM](https://en.wikipedia.org/wiki/LLVM) and
[Binaryen](https://github.com/WebAssembly/binaryen/). The output can run
on the Web, in Node.js, and in
[wasm runtimes](https://v8.dev/blog/emscripten-standalone-wasm#running-in-wasm-runtimes).
It provides Web support for popular portable APIs such as OpenGL and
SDL2.