# Pi Pico Binary Info Parser

Portable C library to extract data from the "binary info" fields of a Pi Pico binary.

## Usage

```c
#include <pico_binary_info.h>

int main() {
  // Load the binary image into memory
  // uint8_t *image = ...
  
  struct pico_binary_info info;
  int rc = pico_binary_info_init(&info, image, sizeof(image));
  if (rc == 0) {
    char buf[256];
    
    if (pico_binary_info_get_string(&info, BINARY_INFO_ID_RP_PROGRAM_NAME, (uint8_t *)buf, sizeof(buf)) == 0)
      printf("Name: %s\n", buf);

    if (pico_binary_info_get_string(&info, BINARY_INFO_ID_RP_PROGRAM_VERSION, (uint8_t *)buf, sizeof(buf)) == 0)
      printf("Version: %s\n", buf);
  } else {
    printf("Could not parse binary info");
  }
}
```

There's also a `pico_binary_info_init_fd()` version that takes a file descriptor instead of a memory buffer.

See [`examples/`](examples/) for example usage.

## Running Tests

```bash
cmake -Bbuild && cmake --build build
ctest --test-dir build --output-on-failure
```
