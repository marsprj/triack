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
	//CPPUNIT_TEST(CreateFile);


	//CPPUNIT_TEST(GetGeoMeta);

	//CPPUNIT_TEST(PutTile);
	//CPPUNIT_TEST(PutCDI);

	//CPPUNIT_TEST(CreateTileStore);
	//CPPUNIT_TEST(ListTileStore);

	//CPPUNIT_TEST(GetTileStoreVolume);

	CPPUNIT_TEST(UpdateTileStoreVolume);

	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

private:
	void GetRoot();
	void GetFolders();
	void GetTestKeys();

	void CreateFolder();
	//void CreateFile();

	void GetGeoMeta();

	void PutTile();
	void PutCDI();

	void CreateTileStore();
	void ListTileStore();

	void GetTileStoreVolume();

	void UpdateTileStoreVolume();

private:
	radi::RiakFS m_riak;
};

#endif //__RADI_RIAK_FS_TEST_H__
