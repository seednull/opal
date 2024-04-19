#include <gtest/gtest.h>

extern "C"
{
#include "pool.h"
}

struct TestData
{
	uint64_t handle;
	uint64_t size;
	uint64_t extra0;
	uint64_t extra1;
};

constexpr uint32_t pool_element_size = sizeof(TestData);
constexpr uint32_t pool_capacity = 0;

class PoolTest : public testing::Test
{
protected:
	void SetUp() override
	{
		Opal_Result result = opal_poolInitialize(&pool, pool_element_size, pool_capacity);
		ASSERT_EQ(result, OPAL_SUCCESS);

		for (uint32_t i = 0; i < 16; ++i)
		{
			payload[i].handle = i;
			payload[i].size = 16 + i;
			payload[i].extra0 = 32 + i;
			payload[i].extra1 = 48 + i;
		}
	}

	void TearDown() override
	{
		Opal_Result result = opal_poolShutdown(&pool);
		ASSERT_EQ(result, OPAL_SUCCESS);
	}

	Opal_Pool pool;
	TestData payload[16];
};

TEST_F(PoolTest, SequentialAddValidateRemove)
{
	Opal_PoolHandle handles[16];
	for (uint32_t i = 0; i < 16; ++i)
	{
		handles[i] = opal_poolAddElement(&pool, &payload[i]);
		EXPECT_NE(handles[i], OPAL_POOL_HANDLE_NULL);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		TestData *data = reinterpret_cast<TestData *>(opal_poolGetElement(&pool, handles[i]));
		EXPECT_NE(data, nullptr);

		EXPECT_EQ(data->handle, i);
		EXPECT_EQ(data->size, 16 + i);
		EXPECT_EQ(data->extra0, 32 + i);
		EXPECT_EQ(data->extra1, 48 + i);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		Opal_Result result = opal_poolRemoveElement(&pool, handles[i]);
		EXPECT_EQ(result, OPAL_SUCCESS);
	}
}

TEST_F(PoolTest, ReuseFreeSlots)
{
	Opal_PoolHandle handles[16];
	for (uint32_t i = 0; i < 16; ++i)
	{
		handles[i] = opal_poolAddElement(&pool, &payload[i]);
		EXPECT_NE(handles[i], OPAL_POOL_HANDLE_NULL);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		TestData *data = reinterpret_cast<TestData *>(opal_poolGetElement(&pool, handles[i]));
		EXPECT_NE(data, nullptr);

		EXPECT_EQ(data->handle, i);
		EXPECT_EQ(data->size, 16 + i);
		EXPECT_EQ(data->extra0, 32 + i);
		EXPECT_EQ(data->extra1, 48 + i);
	}

	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[4]), OPAL_SUCCESS);
	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[7]), OPAL_SUCCESS);

	uint32_t old_capacity = pool.capacity;
	EXPECT_EQ(pool.size, 14);

	handles[4] = opal_poolAddElement(&pool, &payload[4]);
	EXPECT_NE(handles[4], OPAL_POOL_HANDLE_NULL);

	handles[7] = opal_poolAddElement(&pool, &payload[7]);
	EXPECT_NE(handles[7], OPAL_POOL_HANDLE_NULL);

	EXPECT_EQ(pool.size, 16);
	EXPECT_EQ(pool.capacity, old_capacity);

	for (uint32_t i = 0; i < 16; ++i)
	{
		Opal_Result result = opal_poolRemoveElement(&pool, handles[i]);
		EXPECT_EQ(result, OPAL_SUCCESS);
	}
}

TEST_F(PoolTest, DoubleDeleteSingle)
{
	Opal_PoolHandle handle = opal_poolAddElement(&pool, &payload[0]);
	EXPECT_NE(handle, OPAL_POOL_HANDLE_NULL);

	EXPECT_EQ(opal_poolRemoveElement(&pool, handle), OPAL_SUCCESS);
	EXPECT_EQ(opal_poolRemoveElement(&pool, handle), OPAL_INTERNAL_ERROR);
}

TEST_F(PoolTest, DoubleDeleteFull)
{
	Opal_PoolHandle handles[16];

	for (uint32_t i = 0; i < 16; ++i)
	{
		handles[i] = opal_poolAddElement(&pool, &payload[i]);
		EXPECT_NE(handles[i], OPAL_POOL_HANDLE_NULL);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		TestData *data = reinterpret_cast<TestData *>(opal_poolGetElement(&pool, handles[i]));
		EXPECT_NE(data, nullptr);

		EXPECT_EQ(data->handle, i);
		EXPECT_EQ(data->size, 16 + i);
		EXPECT_EQ(data->extra0, 32 + i);
		EXPECT_EQ(data->extra1, 48 + i);
	}
	
	for (uint32_t i = 0; i < 16; ++i)
	{
		EXPECT_EQ(opal_poolRemoveElement(&pool, handles[i]), OPAL_SUCCESS);
		EXPECT_EQ(opal_poolRemoveElement(&pool, handles[i]), OPAL_INTERNAL_ERROR);
	}
}

TEST_F(PoolTest, ProhibitOldHandleAccess)
{
	Opal_PoolHandle handles[16];
	Opal_PoolHandle old_handles[2];

	for (uint32_t i = 0; i < 16; ++i)
	{
		handles[i] = opal_poolAddElement(&pool, &payload[i]);
		EXPECT_NE(handles[i], OPAL_POOL_HANDLE_NULL);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		TestData *data = reinterpret_cast<TestData *>(opal_poolGetElement(&pool, handles[i]));
		EXPECT_NE(data, nullptr);

		EXPECT_EQ(data->handle, i);
		EXPECT_EQ(data->size, 16 + i);
		EXPECT_EQ(data->extra0, 32 + i);
		EXPECT_EQ(data->extra1, 48 + i);
	}

	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[4]), OPAL_SUCCESS);
	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[7]), OPAL_SUCCESS);

	old_handles[0] = handles[4];
	old_handles[1] = handles[7];

	handles[4] = opal_poolAddElement(&pool, &payload[0]);
	EXPECT_NE(handles[4], OPAL_POOL_HANDLE_NULL);

	handles[7] = opal_poolAddElement(&pool, &payload[1]);
	EXPECT_NE(handles[7], OPAL_POOL_HANDLE_NULL);

	EXPECT_EQ(opal_poolGetElement(&pool, old_handles[0]), nullptr);
	EXPECT_EQ(opal_poolGetElement(&pool, old_handles[1]), nullptr);

	for (uint32_t i = 0; i < 16; ++i)
	{
		Opal_Result result = opal_poolRemoveElement(&pool, handles[i]);
		EXPECT_EQ(result, OPAL_SUCCESS);
	}
}

TEST_F(PoolTest, ProhibitOldHandleRemove)
{
	Opal_PoolHandle handles[16];
	Opal_PoolHandle old_handles[2];

	for (uint32_t i = 0; i < 16; ++i)
	{
		handles[i] = opal_poolAddElement(&pool, &payload[i]);
		EXPECT_NE(handles[i], OPAL_POOL_HANDLE_NULL);
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		TestData *data = reinterpret_cast<TestData *>(opal_poolGetElement(&pool, handles[i]));
		EXPECT_NE(data, nullptr);

		EXPECT_EQ(data->handle, i);
		EXPECT_EQ(data->size, 16 + i);
		EXPECT_EQ(data->extra0, 32 + i);
		EXPECT_EQ(data->extra1, 48 + i);
	}

	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[3]), OPAL_SUCCESS);
	EXPECT_EQ(opal_poolRemoveElement(&pool, handles[5]), OPAL_SUCCESS);

	old_handles[0] = handles[3];
	old_handles[1] = handles[5];

	handles[3] = opal_poolAddElement(&pool, &payload[0]);
	EXPECT_NE(handles[3], OPAL_POOL_HANDLE_NULL);

	handles[5] = opal_poolAddElement(&pool, &payload[1]);
	EXPECT_NE(handles[5], OPAL_POOL_HANDLE_NULL);

	EXPECT_EQ(opal_poolRemoveElement(&pool, old_handles[0]), OPAL_INTERNAL_ERROR);
	EXPECT_EQ(opal_poolRemoveElement(&pool, old_handles[1]), OPAL_INTERNAL_ERROR);

	for (uint32_t i = 0; i < 16; ++i)
	{
		Opal_Result result = opal_poolRemoveElement(&pool, handles[i]);
		EXPECT_EQ(result, OPAL_SUCCESS);
	}
}

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
