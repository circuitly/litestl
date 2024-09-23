#include "string.h"
#include "map.h"

namespace litestl::util {

static Map<stringref, StringKey> keymap;
static StringKey stringkey_idgen = 1;

StringKey get_stringkey(const stringref str)
{
  return keymap.add_callback(
      str,
      [](stringref key) {
        /* Do not use debug allocator. */
        return stringref((new string(key))->c_str());
      },
      []() {
        return stringkey_idgen++; //
      });
}
} // namespace litestl::util
