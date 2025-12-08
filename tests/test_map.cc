#include "litestl/util/map.h"
#include "litestl/util/rand.h"
#include "litestl/util/vector.h"
#include "test_util.h"
#include <cstdio>

test_init;

int test_remove()
{
  using namespace litestl::util;
  Random rand;
  Map<int, bool> set;
  Vector<int> keys;
  constexpr int size = 4096;
  int retval = 0;

  for (int i = 0; i < size; i++) {
    int key = rand.get_int();
    set.add(key, true);
    if (!keys.contains(key)) {
      keys.append(key);
    }

    for (auto &key : keys) {
      test_assert(set.contains(key));
    }
    if (rand.get_float() > 0.75) {
      int r = rand.get_int() % keys.size();
      set.remove(keys[r]);
      keys.remove_at(r, true);
    }
  }

  if (int ret = test_remove()) {
    return ret;
  }

  return retval;
}

int main()
{
  using namespace litestl::util;

  {
    Map<int, int> imap;
    Random rand;

    const int size = 512;
    Vector<int> keys, values;

    for (int i = 0; i < size; i++) {
      int key = rand.get_int();
      int value = rand.get_int();

      imap.add(key, value);
      keys.append(key);
      values.append(value);

      test_assert(imap.contains(key));
      test_assert(imap.lookup(key) == value);
    }

    for (auto &pair : imap) {
      test_assert(keys.contains(pair.key));
      test_assert(values.contains(pair.value));

      keys.remove(pair.key);

      for (auto &key : keys) {
        test_assert(imap.contains(key));
      }
    }

    test_assert(keys.size() == 0);
  }

  return test_end();
}
