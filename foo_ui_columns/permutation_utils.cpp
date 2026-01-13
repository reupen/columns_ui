#include "pch.h"

namespace cui::utils {

mmh::Permutation create_shift_item_permutation(size_t old_index, size_t new_index, size_t count)
{
    assert(old_index < count);
    assert(new_index < count);

    const int step = new_index > old_index ? 1 : -1;
    mmh::Permutation permutation(count);

    for (size_t index = old_index; index != new_index; index += step)
        std::swap(permutation[index], permutation[index + step]);

    return permutation;
}

} // namespace cui::utils
