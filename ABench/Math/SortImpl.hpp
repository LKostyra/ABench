#pragma once

namespace ABench {
namespace Math {
namespace Impl {

template <typename T>
size_t QuickSortPartition(std::vector<T>& arr, size_t low, size_t high)
{
    T pivot = arr[low];
    size_t i = low;
    size_t j = high;

    while (true)
    {
        while (arr[i] < pivot)
            i++;

        while (arr[j] > pivot)
            j--;

        if (i >= j)
            return j;

        // swap
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

template <typename T>
void QuickSortInternal(std::vector<T>& arr, size_t low, size_t high)
{
    if (low < high)
    {
        size_t pivot = QuickSortPartition(arr, low, high);
        QuickSortInternal(arr, low, pivot);
        QuickSortInternal(arr, pivot+1, high);
    }
}

} // namespace Impl
} // namespace Math
} // namespace ABench
