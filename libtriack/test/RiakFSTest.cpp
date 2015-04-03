#include "RiakFSTest.h"
#include "RiakTile.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"

CPPUNIT_TEST_SUITE_REGISTRATION(RiakFSTest);

void RiakFSTest::setUp()
{
	printf("setUp\n");

	//m_riak.SetServer("192.168.111.104");
	//m_riak.SetServer("192.168.111.151");
	//m_riak.SetPort(8087);
	if (!m_riak.Connect())
	{
		printf("connect fail\n");
	}
}

void RiakFSTest::tearDown()
{
	m_riak.Close();
	printf("tearDown\n");
}

void RiakFSTest::GetRoot()
{
	radi::RiakFile* rf = m_riak.GetRoot();
	CPPUNIT_ASSERT(rf != NULL);

	rf->Release();
}

void RiakFSTest::GetFolders()
{
	radi::RiakFile* rf = m_riak.GetRoot();
	CPPUNIT_ASSERT(rf != NULL);
	
	radi::RiakFileSet* files = rf->GetFiles();
	CPPUNIT_ASSERT(files != NULL);

	int count = files->GetCount();
	for (int i = 0; i < count; i++)
	{
		radi::RiakFile* f = files->GetRiakFile(i);
		printf("[%s]:%s\n", f->GetName(), f->GetKey());
	}

	files->Release();

	rf->Release();
}

void RiakFSTest::GetTestKeys()
{
	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	radi::RiakFile* tf = root->GetRiakFile("test");
	CPPUNIT_ASSERT(tf != NULL);

	radi::RiakFile* rf = tf->GetRiakFile("wgs84_vector_2to9_Layers");
	CPPUNIT_ASSERT(rf != NULL);

	radi::RiakTileStore* store = rf->GetTileStore();
	//store->GetTiles();

	radi::RiakTile* tile = store->GetTile("2x1x2");
	tile->Save("g:\\temp\\riack\\tile.png");

	tile->Release();
	rf->Release();
	tf->Release();
	root->Release();
}

void RiakFSTest::CreateFolder()
{
	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	root->CreateRiakFolder("test2");

	root->Release();
}

void RiakFSTest::GetGeoMeta()
{
	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	radi::RiakFile* rf = root->GetRiakFile("wgs84_vector_2to9_Layers");
	CPPUNIT_ASSERT(rf != NULL);

	radi::RiakTileStore* store = rf->GetTileStore();

	const char* meta = store->GetGeoMeta();
	printf(meta);

	const char* xml = store->GetConfXML();
	printf(xml);

	const char* cdf = store->GetConfCDI();
	printf(cdf);

	//store->Release();
	rf->Release();
	root->Release();
}

void RiakFSTest::PutTile()
{
	//const char* path = "G:\\temp\\riack\\tile.png";
	const char* path = "G:\\temp\\riack\\9x305x175.png";
	

	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	radi::RiakFile* rf = root->GetRiakFile("world_bbb");
	CPPUNIT_ASSERT(rf != NULL);

	radi::RiakTileStore* store = rf->GetTileStore();

	char key[_MAX_PATH];
	int count = 1000000;
	for (int i = 1; i < count; i++)
	{
		printf("\r%d", i);
		sprintf(key, "%010d", i);
		store->PutTile(key, path);
	}
	

	//store->Release();
	rf->Release();
	root->Release();
}