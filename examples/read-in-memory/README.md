# Pico Binary Info Example: In-memory Buffer

Reads the whole image binary into a malloc'd buffer and hands it to `pico_binary_info_init()`.

## Building

```bash
cmake -Bbuild && cmake --build build
```

## Usage

```bash
./build/read_in_memory mybinary.bin
```
