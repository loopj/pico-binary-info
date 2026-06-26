# pico-binary-info

Portable C library to extract data from the "binary info" fields of a Pi Pico binary.

This is intended to be used on host systems which may wish to verify/validate binary images, for example before a custom DFU process. Data is parsed in an endianness-independent manner.

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

See the [API Reference](http://loopj.com/pico-binary-info/topics.html) for full documentation.

## Examples

See [`examples/`](examples/) for example usage.

## Running Tests

```bash
cmake -Bbuild && cmake --build build
ctest --test-dir build --output-on-failure
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Vendored code in [`include/pico/binary_info/`](include/pico/binary_info/) is copyright Raspberry Pi (Trading) Ltd., and is licensed under the BSD-3-Clause license.
