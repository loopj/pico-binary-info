# Pico Binary Info Example: mmap'ed file

Maps the image binary into memory with `mmap()` and hands the mapped pointer to `pico_binary_info_init()`.

## Building

```bash
cmake -Bbuild && cmake --build build
```

## Usage

```bash
./build/read_mmap mybinary.bin
```
