#ifndef __RADI_RIAK_TEST_H__
#define __RADI_RIAK_TEST_H__

#include "RiakFS.h"
#include "cppunit/extensions/HelperMacros.h" 

#include "riack.h"

class RiakTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(RiakTest);
	//CPPUNIT_TEST(ListBuckets);
	//CPPUNIT_TEST(GetBucket);
	//CPPUNIT_TEST(GetBucketProperty);
	//CPPUNIT_TEST(CreateBucket);
	//CPPUNIT_TEST(PutFile);
	//CPPUNIT_TEST(GetObject);
	//CPPUNIT_TEST(AddLink);
	CPPUNIT_TEST(ListKeys);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

private:
	void ListBuckets();
	void GetBucketProperty();
	void CreateBucket();

	void PutFile();
	void GetBucket();

	void GetObject();
	void AddLink();

	void ListKeys();

private:
	riack_client	*m_riak;
	riack_allocator	m_riak_allocator;
};

#endif //__RADI_RIAK_TEST_H__
