#include "PCH.hpp"
#include "Common/FBXFile.hpp"

using namespace ABench::Common;

TEST(FBXFile, Constructor)
{
    FBXFile f;
    EXPECT_FALSE(f);
}

TEST(FBXFile, NonExistingFile)
{
    FBXFile f;
    EXPECT_FALSE(f);

    EXPECT_FALSE(f.Open("rubbish"));
    EXPECT_FALSE(f);
}

TEST(FBXFile, OpenCloseFile)
{
    // TODO navigate to common root directory
    FBXFile f;
    EXPECT_FALSE(f);

    EXPECT_TRUE(f.Open("Data/TestData/test.fbx"));
    EXPECT_TRUE(f);

    f.Close();
    EXPECT_FALSE(f);
}

// TODO test data extraction
