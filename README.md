# fixed_vector

A header-only, allocation-free vector with a fixed capacity.

`phreak::fixed_vector<T, Capacity>` stores up to `Capacity` elements inside the object itself and never uses the heap. Its interface follows `std::vector`.

```cpp
#include <fixed_vector/fixed_vector.h>

phreak::fixed_vector<std::string, 8> names{"Ada", "Grace"};
names.emplace_back("Barbara");
names.insert(names.begin(), "Edsger");
```

## Requirements

C++17 or later. CMake 3.28 or later for the CMake integration.

## Integration

Copy `include/fixed_vector/fixed_vector.h`, or use CMake:

```cmake
include(FetchContent)
FetchContent_Declare(fixed_vector GIT_REPOSITORY https://github.com/DaPhreak/fixed_vector.git GIT_TAG main)
FetchContent_MakeAvailable(fixed_vector)

target_link_libraries(my_app PRIVATE fixed_vector)
```

## Overflow handling

| Variant | On a full vector |
|---|---|
| `push_back`, `emplace_back`, `insert`, `emplace`, `resize`, `assign` | throws `std::bad_alloc` |
| `try_push_back`, `try_emplace_back` | returns `nullptr` |
| `unchecked_push_back`, `unchecked_emplace_back` | undefined behavior (asserts in debug builds) |

## Notes

- Initializer lists construct elements in place: `{"a", "b"}` creates each `std::string` directly, without temporary copies.
- Arguments may refer to elements of the same vector, e.g. `v.insert(v.begin(), v[2])`.
- Appending never invalidates iterators to existing elements.
- Moving and swapping are linear in the number of elements.

## License

[MIT](LICENSE)
