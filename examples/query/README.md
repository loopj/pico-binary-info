# Pico Binary Info Example: query

A tiny CLI that looks up a single binary_info field by type (`string` or `int`), tag, and id, using the low-level `pico_binary_info_get_string()` / `pico_binary_info_get_int()` functions. Handy for reading custom fields that the convenience helpers don't cover.

## Building

```bash
cmake -Bbuild && cmake --build build
```

## Usage

```bash
./build/query file.bin string|int TAG ID
```

- `TAG` is two characters, e.g. `RP` (Raspberry Pi).
- `ID` is the record id, in decimal or hex.

### Examples

```bash
# Program name (string, Raspberry Pi tag)
./build/query mybinary.bin string RP 0x02031c86

# A custom int field under the 'JS' tag
./build/query mybinary.bin int JS 0x00012345
```
