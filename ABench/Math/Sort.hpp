#pragma once

#include "SortImpl.hpp"

namespace ABench {
namespace Math {

template <typename T>
void QuickSort(std::vector<T>& arr)
{
    Impl::QuickSortInternal(arr, 0, arr.size() - 1);
}

} // namespace Math
} // namespace ABench
