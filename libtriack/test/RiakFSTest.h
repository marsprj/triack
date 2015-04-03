#ifndef __RADI_RIAK_FS_TEST_H__
#define __RADI_RIAK_FS_TEST_H__

#include "RiakFS.h"
#include "cppunit/extensions/HelperMacros.h" 

class RiakFSTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(RiakFSTest);
	//CPPUNIT_TEST(GetRoot);
	
	//CPPUNIT_TEST(GetFolders);
	//CPPUNIT_TEST(GetTestKeys);
	//CPPUNIT_TEST(CreateFolder);

	//CPPUNIT_TEST(GetGeoMeta);

	CPPUNIT_TEST(PutTile);

	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

private:
	void GetRoot();
	void GetFolders();
	void GetTestKeys();
	void CreateFolder();

	void GetGeoMeta();

	void PutTile();

private:
	radi::RiakFS m_riak;
};

#endif //__RADI_RIAK_FS_TEST_H__
