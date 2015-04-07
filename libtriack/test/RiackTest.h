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
	//CPPUNIT_TEST(ListKeys);
	//CPPUNIT_TEST(GetKey);
	//CPPUNIT_TEST(AddKey_1);	
	CPPUNIT_TEST(AddKey_2);
	//CPPUNIT_TEST(AddKey_3);
	//CPPUNIT_TEST(AddKey_4);
	//CPPUNIT_TEST(AddKey_1_Link);
	//CPPUNIT_TEST(Add_Link_test_111);
	//CPPUNIT_TEST(Add_Link_test_111_v2);

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

	void AddKey_1();
	void AddKey_2();
	void AddKey_3();
	void AddKey_4();
	void AddKey_1_Link();
	void Add_Link_test_111();
	void Add_Link_test_111_v2 ();

	void GetKey();

private:
	riack_object*	riack_object_add_link_new(riack_object* sobj, const char* bucket, const char* key, const char* link_bucket, const char* link_key, const char* link_tag);
	void			riack_copy_string(riack_string& obj, riack_string& src);
	void			riack_copy_pair(riack_pair& obj, riack_pair& src);
	void			riack_copy_link(riack_link& obj, riack_link& src);
	void			riack_set_link(riack_link& r_link, const char* bucket, const char* key, const char* tag);
	void			raick_set_pair(riack_pair& rpair, const char* key, const char* data);

	bool			riack_copy_content(riack_content* n_content, riack_content* s_content);

private:
	riack_client	*m_riak;
	riack_allocator	m_riak_allocator;
};

#endif //__RADI_RIAK_TEST_H__
