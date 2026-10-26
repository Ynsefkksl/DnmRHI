# DnmRHI

A small, low-level RHI (Render Hardware Interface) targeting D3D12 Ultimate-class GPUs.

## Status

Under active development and not stable. The API may change without notice.

## Targets

- GPUs supporting D3D12 Ultimate
- Windows and Linux
- A Low-level API with thin abstractions

## Backends

| Backend | Status  |
|---------|---------|
| Vulkan  | Working |
| D3D12   | Planned |

## Features

Not yet implemented:

- Ray tracing
- Variable rate shading (VRS)
- Mesh shaders
- Dynamic library build

## Known Gaps

- The code was extracted from a game engine, so some infrastructure (such as error handling) is not present yet.
- No documentation or examples yet.

## Building

TODO

## License

MIT License

Copyright (c) 2026 Ynsefkksl

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
