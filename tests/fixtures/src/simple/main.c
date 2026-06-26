#include "pico/binary_info.h"

// A custom binary_info field
#define BI_TAG_APP        BINARY_INFO_MAKE_TAG('J', 'S')
#define BI_ID_APP_VERSION 0x00012345

bi_decl(bi_int(BI_TAG_APP, BI_ID_APP_VERSION, 0x00010203));

int main() {
  while (1) {}
}
