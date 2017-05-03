#include "PCH.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

#if defined(WIN32)

#if defined(_CRTDBG_MAP_ALLOC)
    _CrtDumpMemoryLeaks();
#endif // defined(_CRTDBG_MAP_ALLOC)

    system("PAUSE");
#endif // defined(WIN32)

    return result;
}
