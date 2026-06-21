#include "pico_binary_info.h"
#include <stdio.h>
#include <stdlib.h>

#define FLASH_BASE 0x10000000u

int main(int argc, char **argv)
{
  // Check a file argument was passed
  if (argc < 2) {
    fprintf(stderr, "usage: %s file.bin\n", argv[0]);
    return 1;
  }

  // Open the file
  FILE *f = fopen(argv[1], "rb");
  if (!f) {
    perror("open");
    return 1;
  }

  // Get the length of the file
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);

  // Allocate our in-memory buffer, and read the file into it
  uint8_t *buf = malloc(n);
  if (fread(buf, 1, n, f) != (size_t)n) {
    perror("read");
    return 1;
  }

  // Close the file
  fclose(f);

  // Initialize the binary info struct
  struct pico_binary_info info;
  int rc = pico_binary_info_init(&info, buf, n);
  if (rc < 0) {
    fprintf(stderr, "no binary_info (%d)\n", rc);
    return 1;
  }

  // Print header
  printf("File %s:\n\n", argv[1]);
  printf("Program Information\n");

  // Storage for reading values
  char s[256];
  uint32_t end;

  // Print the values
  if (pico_binary_info_get_string(&info, BINARY_INFO_ID_RP_PROGRAM_NAME, (uint8_t *)s, sizeof(s)) == 0)
    printf(" %-14s %s\n", "name:", s);

  if (pico_binary_info_get_string(&info, BINARY_INFO_ID_RP_PROGRAM_VERSION_STRING, (uint8_t *)s, sizeof(s)) == 0)
    printf(" %-14s %s\n", "version:", s);

  printf(" %-14s 0x%08x\n", "binary start:", FLASH_BASE);

  if (pico_binary_info_get_int(&info, BINARY_INFO_ID_RP_BINARY_END, &end) == 0)
    printf(" %-14s 0x%08x\n", "binary end:", end);

  // Free the in-memory copy of the file
  free(buf);

  return 0;
}
