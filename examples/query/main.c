#include "pico_binary_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  // Check the arguments were passed
  if (argc != 5 || (strcmp(argv[2], "string") != 0 && strcmp(argv[2], "int") != 0) || strlen(argv[3]) != 2) {
    fprintf(stderr, "usage: %s file.bin string|int TAG ID\n", argv[0]);
    fprintf(stderr, "  TAG  two characters, e.g. RP\n");
    fprintf(stderr, "  ID   record id, e.g. 0x02031c86\n");
    return 1;
  }

  // Parse the field type, tag (two chars) and id
  const char *type = argv[2];
  uint16_t tag     = (uint16_t)(((uint8_t)argv[3][1] << 8) | (uint8_t)argv[3][0]);
  uint32_t id      = (uint32_t)strtoul(argv[4], NULL, 0);

  // Open the file
  FILE *f = fopen(argv[1], "rb");
  if (!f) {
    perror("open");
    return 1;
  }

  // Get the length of the file, then read it into an in-memory buffer
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  uint8_t *buf = malloc(n);
  if (fread(buf, 1, n, f) != (size_t)n) {
    perror("read");
    return 1;
  }
  fclose(f);

  // Initialize the binary info struct
  struct pico_binary_info info;
  int rc = pico_binary_info_init(&info, buf, n);
  if (rc < 0) {
    fprintf(stderr, "no binary_info (%d)\n", rc);
    return 1;
  }

  // Query the requested field and print it
  if (strcmp(type, "string") == 0) {
    char s[256];
    rc = pico_binary_info_get_string(&info, tag, id, s, sizeof(s));
    if (rc == 0)
      printf("%s\n", s);
  } else {
    uint32_t value;
    rc = pico_binary_info_get_int(&info, tag, id, &value);
    if (rc == 0)
      printf("0x%08x\n", value);
  }

  // Report a lookup failure
  if (rc < 0)
    fprintf(stderr, "not found (%d)\n", rc);

  // Free the in-memory copy of the file
  free(buf);

  return rc < 0 ? 1 : 0;
}
