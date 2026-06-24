# pico-binary-info

Portable C library to extract data from the "binary info" fields of a Pi Pico binary.

Currently supports string and integer field type, and provides wrappers for common field types such as program version, app name, and board name.

## Usage

First, initialize a binary info struct from a memory buffer or file descriptor:

```c
// From an in-memory buffer (or memory mapped file)
struct pico_binary_info info;
pico_binary_info_init(&info, image, size);

// From a file descriptor (posix and newlib friendly)
struct pico_binary_info info;
pico_binary_info_init_fd(&info, fd, size);
```

Then, use the `pico_binary_info_*()` functions to extract data from the binary info struct, for example:

```c
char buf[256];
if (pico_binary_info_get_program_version(&info, buf, sizeof(buf)) == 0)
  printf("Version: %s\n", buf);
```

## Examples

See [`examples/`](examples/) for example usage.

## Running Tests

```bash
cmake -Bbuild && cmake --build build
ctest --test-dir build --output-on-failure
```
