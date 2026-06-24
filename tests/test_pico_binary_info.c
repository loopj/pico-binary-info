#include "pico_binary_info.h"
#include "unity.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static void check_simple(struct pico_binary_info *info)
{
  char buf[64];
  int rc;

  rc = pico_binary_info_get_program_name(info, buf, sizeof buf);
  TEST_ASSERT_EQUAL_INT(0, rc);
  TEST_ASSERT_EQUAL_STRING("simple", buf);

  rc = pico_binary_info_get_program_version(info, buf, sizeof buf);
  TEST_ASSERT_EQUAL_INT(0, rc);
  TEST_ASSERT_EQUAL_STRING("1.2.3", buf);

  // Records are keyed on (tag, id): the program_name id (0x02031c86) exists
  // under the Raspberry Pi tag ('R','P'), so querying it under a different tag
  // must not match. 0x5858 is the tag built from ('X','X').
  rc = pico_binary_info_get_string(info, 0x5858, 0x02031c86u, buf, sizeof buf);
  TEST_ASSERT_EQUAL_INT(-PICO_BI_ENOTFOUND, rc);

  // The SDK auto-adds a pico_board string; it begins with "pico".
  rc = pico_binary_info_get_pico_board(info, buf, sizeof buf);
  TEST_ASSERT_EQUAL_INT(0, rc);
  TEST_ASSERT_EQUAL_STRING_LEN("pico", buf, 4);

  // The SDK auto-adds a binary_end int pointing into flash (>= FLASH_BASE).
  uint32_t binary_end = 0;
  rc = pico_binary_info_get_binary_end(info, &binary_end);
  TEST_ASSERT_EQUAL_INT(0, rc);
  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(0x10000000u, binary_end);
}

struct fixture {
  const char *app;
  const char *board;
  const char *type;
  void (*check)(struct pico_binary_info *info);
};

static const struct fixture *fixture;

static const struct fixture FIXTURES[] = {
  {"simple", "pico", "default", check_simple},
  {"simple", "pico", "copy_to_ram", check_simple},
  {"simple", "pico2", "default", check_simple},
  {"simple", "pico2", "copy_to_ram", check_simple},
};

static void fixture_path(char *out, size_t out_len)
{
  snprintf(out, out_len, FIXTURES_DIR "/bin/%s-%s-%s.bin", fixture->app, fixture->board, fixture->type);
}

static void test_buffer(void)
{
  // Get the path of the current fixture's image
  char path[256];
  fixture_path(path, sizeof path);

  // Open the image
  FILE *f = fopen(path, "rb");
  char msg[320];
  snprintf(msg, sizeof msg, "could not open %s", path);
  TEST_ASSERT_NOT_NULL_MESSAGE(f, msg);

  // Get the size of the image
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, (int)len, "could not size fixture");

  // Read the image into a buffer
  uint8_t *buf = malloc((size_t)len);
  TEST_ASSERT_NOT_NULL(buf);
  TEST_ASSERT_EQUAL_size_t(len, fread(buf, 1, (size_t)len, f));
  fclose(f);

  // Run this fixture's check function on the image
  struct pico_binary_info info;
  TEST_ASSERT_EQUAL_INT(0, pico_binary_info_init(&info, buf, (size_t)len));
  fixture->check(&info);

  // Clean up
  free(buf);
}

// Parameterized over fixture: exercises the file-descriptor backend.
static void test_fd(void)
{
  // Get the path of the current fixture's image
  char path[256];
  fixture_path(path, sizeof path);

  // Open the image file
  int fd = open(path, O_RDONLY);
  char msg[320];
  snprintf(msg, sizeof msg, "could not open %s", path);
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0, fd, msg);

  // Get the size of the image
  off_t len = lseek(fd, 0, SEEK_END);
  TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, (int)len, "could not size fixture");
  TEST_ASSERT_EQUAL_INT(0, lseek(fd, 0, SEEK_SET));

  // Run this fixture's check function on the image
  struct pico_binary_info info;
  TEST_ASSERT_EQUAL_INT(0, pico_binary_info_init_fd(&info, fd, (size_t)len));
  fixture->check(&info);

  // Clean up
  close(fd);
}

int main(void)
{
  UNITY_BEGIN();

  for (size_t i = 0; i < sizeof(FIXTURES) / sizeof(FIXTURES[0]); i++) {
    char label[80];
    fixture = &FIXTURES[i];

    // Run buffer tests
    snprintf(label, sizeof label, "%s-%s-%s:buffer", fixture->app, fixture->board, fixture->type);
    UnityDefaultTestRun(test_buffer, label, __LINE__);

    // Run file-descriptor tests
    snprintf(label, sizeof label, "%s-%s-%s:fd", fixture->app, fixture->board, fixture->type);
    UnityDefaultTestRun(test_fd, label, __LINE__);
  }

  return UNITY_END();
}
