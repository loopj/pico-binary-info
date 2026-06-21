# Pico Binary Info Example: file descriptor

Reads the image binary through an open, seekable file descriptor with `pico_binary_info_init_fd()` instead of loading it into memory. Each lookup reads only the bytes it needs via `lseek`/`read`, so the image is never fully loaded into RAM.

## Building

```bash
cmake -Bbuild && cmake --build build
```

## Usage

```bash
./build/read_fd mybinary.bin
```
