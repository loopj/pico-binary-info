#include "pico_binary_info.h"
#include "unity.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// structure.h's BINARY_INFO_MAKE_TAG uses `uint`; supply it for a standalone
// build, mirroring src/pico_binary_info.c. (A duplicate identical typedef is
// permitted where `uint` already exists.)
typedef unsigned int uint;
#include "pico/binary_info/structure.h"

// A custom application tag and id, mirroring tests/fixtures/src/simple/main.c.
#define BI_TAG_APP        BINARY_INFO_MAKE_TAG('J', 'S')
#define BI_ID_APP_VERSION 0x00012345u
#define APP_VERSION       0x00010203u  // (1 << 16) | (2 << 8) | 3

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

  // Records are keyed on (tag, id), so the program_name id under a wrong tag must not match.
  rc = pico_binary_info_get_string(info, BINARY_INFO_MAKE_TAG('X', 'X'),
                                   BINARY_INFO_ID_RP_PROGRAM_NAME, buf, sizeof buf);
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

  // Decode the custom-tag int field the fixture emits.
  uint32_t app_version = 0;
  rc = pico_binary_info_get_int(info, BI_TAG_APP, BI_ID_APP_VERSION, &app_version);
  TEST_ASSERT_EQUAL_INT(0, rc);
  TEST_ASSERT_EQUAL_HEX32(APP_VERSION, app_version);

  // The same id under the wrong tag must not match (records are keyed on tag+id).
  rc = pico_binary_info_get_int(info, BINARY_INFO_TAG_RASPBERRY_PI, BI_ID_APP_VERSION, &app_version);
  TEST_ASSERT_EQUAL_INT(-PICO_BI_ENOTFOUND, rc);

  // It is an int record, so requesting it as a string is the wrong type.
  rc = pico_binary_info_get_string(info, BI_TAG_APP, BI_ID_APP_VERSION, buf, sizeof buf);
  TEST_ASSERT_EQUAL_INT(-PICO_BI_EWRONGTYPE, rc);
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
