#include "litestl/util/rand.h"
#include "litestl/util/set.h"
#include "litestl/util/string.h"
#include "litestl/util/vector.h"
#include "test_util.h"
#include <cstdio>

test_init;

int test_remove()
{
  using namespace litestl::util;
  Random rand;
  Set<int> set;
  Vector<int> keys;
  constexpr int size = 4096;
  int retval = 0;

  for (int i = 0; i < size; i++) {
    int key = rand.get_int();
    set.add(key);
    if (!keys.contains(key)) {
      keys.append(key);
    }

    for (auto &key : keys) {
      test_assert(set.contains(key));
    }
    if (rand.get_float() > 0.75) {
      int r = rand.get_int() % keys.size();
      set.remove(keys[r]);
      keys.removeAt(r, true);
    }
  }

  return retval;
}

int main()
{
  using namespace litestl::util;

  {
    static constexpr int size = 512;

    Set<int, size + 10> set;
    Random rand;

    Vector<int> keys;

    for (int i = 0; i < size; i++) {
      int key = rand.get_int();

      set.add(key);
      keys.append(key);

      test_assert(set.contains(key));
    }

    for (int key : keys) {
      test_assert(set.contains(key));
    }

    for (int key : set) {
      test_assert(keys.contains(key));
    }

    const char *strings[] = {"sdfdsf", "bleh", "yay", "space123324", "hahah", "gagaga"};

    Set<string> strset;

    for (int i = 0; i < array_size(strings); i++) {
      strset.add(strings[i]);
    }

    test_assert(!strset["dsfdsf"]);

    for (const string &str : strset) {
      test_assert(strset[str]);
    }

    if (int ret = test_remove()) {
      return ret;
    }
  }

  return test_end();
}
