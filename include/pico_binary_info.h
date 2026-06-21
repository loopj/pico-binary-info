#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * Pico Binary Info error codes. Functions return the negation of these, so 0
 * is success and a failure is e.g. -PICO_BI_ENOTFOUND.
 */
enum pico_binary_info_error {
  /** No record found matching that id */
  PICO_BI_ENOTFOUND = 1,

  /** No binary_info header found in the image */
  PICO_BI_ENOHEADER,

  /** Record found but not the requested type */
  PICO_BI_EWRONGTYPE,

  /** Output buffer too small */
  PICO_BI_ERANGE,

  /** A record's address could not be resolved to data within the image */
  PICO_BI_EBADADDR,
};

// Common record ids
#define BINARY_INFO_ID_RP_PROGRAM_NAME            0x02031c86u
#define BINARY_INFO_ID_RP_PROGRAM_VERSION_STRING  0x11a9bc3au
#define BINARY_INFO_ID_RP_PROGRAM_BUILD_DATE      0x9da22254u
#define BINARY_INFO_ID_RP_BINARY_END              0x68f465deu
#define BINARY_INFO_ID_RP_PROGRAM_URL             0x1856239au
#define BINARY_INFO_ID_RP_PROGRAM_DESCRIPTION     0xb6a07c19u
#define BINARY_INFO_ID_RP_PROGRAM_FEATURE         0xa1f4b453u
#define BINARY_INFO_ID_RP_PROGRAM_BUILD_ATTRIBUTE 0x4275f0d3u
#define BINARY_INFO_ID_RP_SDK_VERSION             0x5360b3abu
#define BINARY_INFO_ID_RP_PICO_BOARD              0xb63cffbbu
#define BINARY_INFO_ID_RP_BOOT2_NAME              0x7f8882e1u

/**
 * Struct representing a parsed pico binary info header.
 */
struct pico_binary_info {
  // Pointer to the "in-memory" image, NULL if using file descriptors
  const uint8_t *image;

  // File handle for a "file descriptor" image, or -1 if using the in-memory buffer
  int fd;

  // Total size of the image
  size_t image_len;

  // File offset for the start of the binary info record table
  size_t table_off;

  // End of the binary info record table
  size_t table_end;

  // File offset for the start of the copy-mapping table
  long mapping_off;
};

/*
 * Initialize a pico_binary_info struct using a pointer to a pico binary image
 * which is in memory, or a memory mapped file.
 *
 * Use this on systems where mmap is available, or where loading the entire
 * binary into memory is feasible.
 *
 * @param info the binary info struct to initialize
 * @param image pointer to the start of the binary image
 * @param len size of the binary image
 * @returns 0 for success, negative error code on failure
 */
int pico_binary_info_init(struct pico_binary_info *info, const uint8_t *image, size_t len);

/*
 * Initialize a pico_binary_info struct using a file handle.
 *
 * Use this on systems where posix/newlib `open  and `read` are available.
 *
 * @param info the binary info struct to initialize
 * @param fd an open, seekable descriptor positioned anywhere
 * @param len the total length of the image (e.g. from fstat/lseek)
 * @returns 0 for success, negative error code on failure
 */
int pico_binary_info_init_fd(struct pico_binary_info *info, int fd, size_t len);

/*
 * Fetch the integer value of an id-and-int record.
 *
 * @param info the initialized binary info struct
 * @param id the record id
 * @param out where to store the integer
 * @returns 0 for success, negative error code on failure
 */
int pico_binary_info_get_int(struct pico_binary_info *info, uint32_t id, uint32_t *out);

/*
 * Fetch the string value of an id-and-string record.
 *
 * @param info the initialized binary info struct
 * @param id the record id
 * @param out where to store the string
 * @param out_len the size of `out` string buffer
 * @returns 0 for success, negative error code on failure
 */
int pico_binary_info_get_string(struct pico_binary_info *info, uint32_t id, uint8_t *out, size_t out_len);
