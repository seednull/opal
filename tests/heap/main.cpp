#include <gtest/gtest.h>

extern "C"
{
#include "heap.h"
}

constexpr uint32_t heap_size = 64 * 1024 * 1024;
constexpr uint32_t max_allocations = 1000000;

class HeapTest : public testing::Test
{
protected:
	void SetUp() override
	{
		Opal_Result result = opal_heapInitialize(&heap, heap_size, max_allocations);
		ASSERT_EQ(result, OPAL_SUCCESS);
	}

	void TearDown() override
	{
		Opal_Result result = opal_heapShutdown(&heap);
		ASSERT_EQ(result, OPAL_SUCCESS);
	}

	Opal_Heap heap;
};

TEST_F(HeapTest, AlignedAllocsSameAlignment)
{
	constexpr uint32_t alignment = 16;
	Opal_HeapAllocation allocs[2];

	EXPECT_EQ(opal_heapAllocAligned(&heap, 9, alignment, &allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[0].offset % alignment, 0);
	EXPECT_EQ(allocs[0].offset, 0);

	EXPECT_EQ(opal_heapAllocAligned(&heap, 16, alignment, &allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[1].offset % alignment, 0);
	EXPECT_EQ(allocs[1].offset, 16);

	EXPECT_EQ(opal_heapFree(&heap, allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapAllocAligned(&heap, 16, alignment, &allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[0].offset % alignment, 0);
	EXPECT_EQ(allocs[0].offset, 0);

	EXPECT_EQ(opal_heapFree(&heap, allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapFree(&heap, allocs[1]), OPAL_SUCCESS);
}

TEST_F(HeapTest, AlignedAllocsDifferentAlignments)
{
	constexpr uint32_t alignment1 = 16;
	constexpr uint32_t alignment2 = 32;

	Opal_HeapAllocation allocs[3];

	EXPECT_EQ(opal_heapAllocAligned(&heap, 9, alignment1, &allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[0].offset % alignment1, 0);
	EXPECT_EQ(allocs[0].offset, 0);

	EXPECT_EQ(opal_heapAllocAligned(&heap, 16, alignment1, &allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[1].offset % alignment1, 0);
	EXPECT_EQ(allocs[1].offset, 16);

	EXPECT_EQ(opal_heapAllocAligned(&heap, 32, alignment2, &allocs[2]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[2].offset % alignment2, 0);
	EXPECT_EQ(allocs[2].offset, 32);

	EXPECT_EQ(opal_heapFree(&heap, allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapFree(&heap, allocs[1]), OPAL_SUCCESS);

	EXPECT_EQ(opal_heapAllocAligned(&heap, 32, alignment2, &allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[1].offset % alignment2, 0);
	EXPECT_EQ(allocs[1].offset, 0);

	EXPECT_EQ(opal_heapFree(&heap, allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapFree(&heap, allocs[2]), OPAL_SUCCESS);
}

TEST_F(HeapTest, NonFitAlignedAlloc)
{
	constexpr uint32_t alignment = 16;
	Opal_HeapAllocation allocs[3];

	EXPECT_EQ(opal_heapAlloc(&heap, 3, &allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[0].offset, 0);

	EXPECT_EQ(opal_heapAlloc(&heap, 16, &allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[1].offset, 3);

	EXPECT_EQ(opal_heapAlloc(&heap, 16, &allocs[2]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[2].offset, 19);

	EXPECT_EQ(opal_heapFree(&heap, allocs[1]), OPAL_SUCCESS);

	EXPECT_EQ(opal_heapAllocAligned(&heap, 16, alignment, &allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(allocs[1].offset % alignment, 0);
	EXPECT_EQ(allocs[1].offset, 48);

	EXPECT_EQ(opal_heapFree(&heap, allocs[0]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapFree(&heap, allocs[1]), OPAL_SUCCESS);
	EXPECT_EQ(opal_heapFree(&heap, allocs[2]), OPAL_SUCCESS);
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
