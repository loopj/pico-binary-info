#include <string.h>
#include <unistd.h>

#include "pico_binary_info.h"

// Start/end magic numbers for binary info section
#define BINARY_INFO_MARKER_START        0x7188ebf2u
#define BINARY_INFO_MARKER_END          0xe71aa390u

// Record types and tags
#define BINARY_INFO_TAG_RASPBERRY_PI    0x5052
#define BINARY_INFO_TYPE_ID_AND_INT     0x0005
#define BINARY_INFO_TYPE_ID_AND_STRING  0x0006

// Flash
#define FLASH_BASE  0x10000000u

// Read a uint16_t, in little endian byte order, from a byte array
static inline uint16_t get_le16(const uint8_t *bytes)
{
  return (uint16_t)(bytes[0] | (bytes[1] << 8));
}

// Read a uint32_t, in little endian byte order, from a byte array
static inline uint32_t get_le32(const uint8_t *bytes)
{
  return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

// Read `count` bytes from the image at `offset` into `buf`
static int read_image_bytes(const struct pico_binary_info *info, size_t offset, uint8_t *buf, size_t count)
{
  // Reject reads that fall outside the image
  if (offset > info->image_len || count > info->image_len - offset)
    return -1;

  // If we're using a buffer, read from it directly
  if (info->image) {
    memcpy(buf, info->image + offset, count);
    return 0;
  }

  // Otherwise, seek to the offset and read from the file descriptor
  if (lseek(info->fd, (off_t)offset, SEEK_SET) < 0)
    return -1;

  size_t total = 0;
  while (total < count) {
    ssize_t bytes_read = read(info->fd, buf + total, count - total);
    if (bytes_read <= 0)
      return -1;
    total += (size_t)bytes_read;
  }

  return 0;
}

// Turn a runtime address into an image offset, or (size_t)-1 if unresolvable.
static size_t addr_to_image_offset(const struct pico_binary_info *info, uint32_t addr)
{
  // Try direct flash translation first
  if (addr >= FLASH_BASE) {
    uint32_t offset = addr - FLASH_BASE;
    if (offset < info->image_len)
      return offset;
  }

  // Can only walk the mapping table if the mapping offset is known
  if (info->mapping_off < 0)
    return (size_t)-1;

  // Walk the mapping table to resolve the address
  for (long entry_off = info->mapping_off, steps = 0; steps < 16; entry_off += 12, steps++) {
    uint8_t entry[12];
    if (read_image_bytes(info, (size_t)entry_off, entry, 12) != 0)
      break;

    uint32_t source = get_le32(entry);
    uint32_t start  = get_le32(entry + 4);
    uint32_t end    = get_le32(entry + 8);
    if (source == 0 && start == 0 && end == 0)
      break;

    if (addr >= start && addr < end) {
      uint32_t phys_addr = source + (addr - start);
      if (phys_addr >= FLASH_BASE) {
        uint32_t offset = phys_addr - FLASH_BASE;
        if (offset < info->image_len)
          return offset;
      }
    }
  }
  return (size_t)-1;
}

// Find the record with the given id, writing its type to *type_out; returns its image offset or (size_t)-1.
static size_t find_record(const struct pico_binary_info *info, uint32_t id, uint16_t *type_out)
{
  for (size_t table_pos = info->table_off; table_pos + 4 <= info->table_end; table_pos += 4) {
    // Read the address bytes from the table
    uint8_t addr_bytes[4];
    if (read_image_bytes(info, table_pos, addr_bytes, 4) != 0)
      continue;

    // Convert the address to an image offset and check it resolved
    size_t offset = addr_to_image_offset(info, get_le32(addr_bytes));
    if (offset == (size_t)-1)
      continue;

    // Read the record header
    uint8_t header[8]; /* u16 type, u16 tag, u32 id */
    if (read_image_bytes(info, offset, header, 8) != 0)
      continue;

    // Check the tag and id match
    uint16_t type = get_le16(header);
    uint16_t tag  = get_le16(header + 2);
    if (tag != BINARY_INFO_TAG_RASPBERRY_PI)
      continue;
    if (get_le32(header + 4) != id)
      continue;

    *type_out = type;

    return offset;
  }

  return (size_t)-1;
}

// Locate the binary_info header and fill in the table/mapping offsets; returns 0 or -PICO_BI_ENOHEADER.
static int parse_header(struct pico_binary_info *info)
{
  // The SDK emits the binary_info header near the start of the image
  uint8_t window[1024];
  size_t len = info->image_len < sizeof window ? info->image_len : sizeof window;
  if (read_image_bytes(info, 0, window, len) != 0)
    return -PICO_BI_ENOHEADER;

  // Search for the header marker within the first kilobyte.
  for (size_t offset = 0; offset + 20 <= len; offset += 4) {
    if (get_le32(window + offset) != BINARY_INFO_MARKER_START)
      continue;
    if (get_le32(window + offset + 16) != BINARY_INFO_MARKER_END)
      continue;

    // Found the header marker: extract the table and mapping addresses.
    uint32_t table_start_addr = get_le32(window + offset + 4);
    uint32_t table_end_addr   = get_le32(window + offset + 8);
    uint32_t mapping_addr     = get_le32(window + offset + 12);

    // Store the start/end offsets of the table
    info->table_off = table_start_addr - FLASH_BASE;
    info->table_end = table_end_addr - FLASH_BASE;

    // Store the mapping offset if the address is valid
    if (mapping_addr >= FLASH_BASE)
      info->mapping_off = (long)(mapping_addr - FLASH_BASE);

    return 0;
  }

  return -PICO_BI_ENOHEADER;
}

int pico_binary_info_init(struct pico_binary_info *info, const uint8_t *image, size_t len)
{
  info->image       = image;
  info->fd          = -1;
  info->image_len   = len;
  info->table_off   = 0;
  info->table_end   = 0;
  info->mapping_off = -1;

  return parse_header(info);
}

int pico_binary_info_init_fd(struct pico_binary_info *info, int fd, size_t len)
{
  info->image       = NULL;
  info->fd          = fd;
  info->image_len   = len;
  info->table_off   = 0;
  info->table_end   = 0;
  info->mapping_off = -1;

  return parse_header(info);
}

int pico_binary_info_get_int(struct pico_binary_info *info, uint32_t id, uint32_t *out)
{
  // Look up the record for the given id
  uint16_t type;
  size_t rec_off = find_record(info, id, &type);
  if (rec_off == (size_t)-1)
    return -PICO_BI_ENOTFOUND;

  // Verify the record type is ID_AND_INT
  if (type != BINARY_INFO_TYPE_ID_AND_INT)
    return -PICO_BI_EWRONGTYPE;

  // Read the bytes from the record and convert to a little-endian 32-bit int
  uint8_t bytes[4];
  if (read_image_bytes(info, rec_off + 8, bytes, 4) != 0)
    return -PICO_BI_EBADADDR;
  *out = get_le32(bytes);

  return 0;
}

int pico_binary_info_get_string(struct pico_binary_info *info, uint32_t id, uint8_t *out, size_t out_len)
{
  // Look up the record for the given id
  uint16_t type;
  size_t rec_off = find_record(info, id, &type);
  if (rec_off == (size_t)-1)
    return -PICO_BI_ENOTFOUND;

  // Verify the record type is ID_AND_STRING
  if (type != BINARY_INFO_TYPE_ID_AND_STRING)
    return -PICO_BI_EWRONGTYPE;

  // Read the string address from the record
  uint8_t addr_bytes[4];
  if (read_image_bytes(info, rec_off + 8, addr_bytes, 4) != 0)
    return -PICO_BI_EBADADDR;

  // Resolve the string offset from the address
  size_t str_off = addr_to_image_offset(info, get_le32(addr_bytes));
  if (str_off == (size_t)-1)
    return -PICO_BI_EBADADDR;

  // Copy the string out of the image, truncating to fit, and NUL-terminate.
  size_t count = info->image_len - str_off;
  if (count > out_len - 1)
    count = out_len - 1;
  if (read_image_bytes(info, str_off, out, count) != 0)
    return -PICO_BI_EBADADDR;
  out[count] = '\0';

  return 0;
}
