#pragma once

#include "SortImpl.hpp"

namespace ABench {
namespace Math {

template <typename T>
void QuickSort(std::vector<T>& arr)
{
    Impl::QuickSortInternal(arr, 0, arr.size() - 1);
}

template <typename T>
void BitonicSort(std::vector<T>& arr, bool ascending)
{
    unsigned int origSize = static_cast<unsigned int>(arr.size());
    unsigned int size = origSize;
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;

    arr.resize(size);
    Impl::BitonicSortInternal(arr, ascending);
    arr.resize(origSize);
}

} // namespace Math
} // namespace ABench
