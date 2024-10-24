#include <cstdlib>
#include <cstring>
#include <emscripten.h>

namespace litestl::util::wasm {
static const char *getStackTrace(const char *prefix)
{
  char *tag = static_cast<char *>(malloc(2050));
  strcpy(tag, prefix);
  int start = strlen(prefix);

  if (tag) {
    char *tag2 = tag + start;
    EM_ASM(
        {
          Error.stackTraceLimit = 120;
          let error;
          try {
            throw new Error();
          } catch (err) {
            error = err;
            const s = "" + error.stack.split("\n").slice(4, -1).join("\n");
            const len = Math.min(s.length, 2000);
            for (let i = 0; i < len; i++) {
              HEAPU8[$0 + i] = s.charCodeAt(i);
            }
            HEAPU8[$0 + len] = 0;
          }
        },
        tag2);
  }

  return tag;
}
} // namespace litestl::util::wasm
