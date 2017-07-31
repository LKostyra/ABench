#include "PCH.hpp"
#include "Math/Sort.hpp"
#include "Common/Timer.hpp"


TEST(Sort, SimpleIntArray)
{
    const std::vector<int> simpleSorted{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> unsorted = simpleSorted;
    std::shuffle(unsorted.begin(), unsorted.end(), gen);

    ABench::Math::QuickSort(unsorted);
    ASSERT_TRUE(unsorted == simpleSorted);
}

TEST(Sort, SimpleFloatArray)
{
    const std::vector<float> simpleSorted{1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<float> unsorted = simpleSorted;
    std::shuffle(unsorted.begin(), unsorted.end(), gen);

    ABench::Math::QuickSort(unsorted);
    ASSERT_TRUE(unsorted == simpleSorted);
}


TEST(Sort, FloatPerfTest)
{
    ABench::Common::Timer t;
    std::random_device rd;
    std::mt19937 gen(rd());
    const uint32_t REPEAT_COUNT = 10;
    double averageTime = 0.0;

    for (size_t i = 10; i <= 100000; i *= 2)
    {
        std::vector<float> sorted;
        sorted.resize(i);
        for (uint32_t j = 0; j < i; ++j)
            sorted[j] = static_cast<float>(j);

        std::vector<float> unsorted = sorted;

        std::cout << "#" << i << ": ";
        averageTime = 0.0;
        for (uint32_t j = 0; j < REPEAT_COUNT; ++j)
        {
            std::shuffle(unsorted.begin(), unsorted.end(), gen);

            t.Start();
            ABench::Math::QuickSort(unsorted);
            averageTime += t.Stop();

            ASSERT_TRUE(unsorted == sorted);
        }

        std::cout << averageTime / static_cast<double>(REPEAT_COUNT) << std::endl;
    }
}
