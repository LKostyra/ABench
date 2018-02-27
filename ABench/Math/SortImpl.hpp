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

template <typename T>
void BitonicSortBox(std::vector<T>& arr, uint32_t boxSize, uint32_t boxOffset, bool greaterComp)
{
    for (uint32_t pass = boxSize; pass > 1; pass >>=1)
    {
        for (uint32_t j = 0; j < boxSize; j += pass)
        {
            uint32_t hp = pass >> 1;
            for (uint32_t i = 0; i < hp; ++i)
            {
                uint32_t a = boxOffset + j + i;
                uint32_t b = boxOffset + j + i + hp;
                if ((arr[a] < arr[b]) == greaterComp)
                {
                    T temp = arr[a];
                    arr[a] = arr[b];
                    arr[b] = temp;
                }
            }
        }
    }
}

// simulation approach, just to verify sequentially that this will work
template <typename T>
void BitonicSortBoxGPU(std::vector<T>& arr, uint32_t invocationID, uint32_t boxSize, uint32_t compareDist, bool)
{
    uint32_t size = static_cast<uint32_t>(arr.size());

    // assumes compute shaders have 2 cores per thread group
    for (uint32_t n = invocationID; n < size; n += 2)
    {
        uint32_t x = n ^ compareDist;

        if (x > n)
        {
            if (n & boxSize)
            {
                if (arr[n] > arr[x])
                {
                    T temp = arr[n];
                    arr[n] = arr[x];
                    arr[x] = temp;
                }
            }
            else
            {
                if (arr[n] < arr[x])
                {
                    T temp = arr[n];
                    arr[n] = arr[x];
                    arr[x] = temp;
                }
            }
        }
    }
}

// small-data POC of bitonic mergesort on GPU
template <typename T>
void BitonicSortInternal(std::vector<T>& arr, bool ascending)
{
    uint32_t size = static_cast<uint32_t>(arr.size());

    for (uint32_t boxSize = 2; boxSize <= size; boxSize <<= 1)
    {
        bool greaterComp = !ascending;

        for (uint32_t compareDist = (boxSize >> 1); compareDist > 0; compareDist >>= 1)
        {
            // Assumes GPUs today have 4 compute shaders per thread group
            for (uint32_t invocationID = 0; invocationID < 2; ++invocationID)
                BitonicSortBoxGPU(arr, invocationID, boxSize, compareDist, greaterComp);
        }
    }
}

} // namespace Impl
} // namespace Math
} // namespace ABench
