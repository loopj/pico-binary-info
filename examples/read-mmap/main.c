#include "pico_binary_info.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define FLASH_BASE 0x10000000u

int main(int argc, char **argv)
{
  // Check a file argument was passed
  if (argc < 2) {
    fprintf(stderr, "usage: %s file.bin\n", argv[0]);
    return 1;
  }

  // Open the file
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  // Get the length of the file
  off_t n = lseek(fd, 0, SEEK_END);
  if (n < 0) {
    perror("lseek");
    return 1;
  }

  // Map the file into memory
  const uint8_t *buf = mmap(NULL, (size_t)n, PROT_READ, MAP_PRIVATE, fd, 0);
  if (buf == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  // We can close the descriptor now; the mapping stays valid.
  close(fd);

  // Initialize the binary info struct from the mapped image.
  struct pico_binary_info info;
  int rc = pico_binary_info_init(&info, buf, (size_t)n);
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
  if (pico_binary_info_get_program_name(&info, s, sizeof(s)) == 0)
    printf(" %-14s %s\n", "name:", s);

  if (pico_binary_info_get_program_version(&info, s, sizeof(s)) == 0)
    printf(" %-14s %s\n", "version:", s);

  printf(" %-14s 0x%08x\n", "binary start:", FLASH_BASE);

  if (pico_binary_info_get_binary_end(&info, &end) == 0)
    printf(" %-14s 0x%08x\n", "binary end:", end);

  // Release the mapping
  munmap((void *)buf, (size_t)n);

  return 0;
}
