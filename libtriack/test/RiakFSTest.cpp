#include "RiakFSTest.h"
#include "RiakTile.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"
#include "RiakTileStoreSet.h"

CPPUNIT_TEST_SUITE_REGISTRATION(RiakFSTest);

void RiakFSTest::setUp()
{
	printf("setUp\n");

	//m_riak.SetServer("192.168.111.104");
	//m_riak.SetServer("192.168.111.151");
	//m_riak.SetPort(8087);
	
	// ����Riack
	if (!m_riak.Connect())
	{
		printf("connect fail\n");
	}
}

void RiakFSTest::tearDown()
{
	// �ر�����
	m_riak.Close();
	printf("tearDown\n");
}

void RiakFSTest::GetRoot()
{
	// ��ȡRiakFS�ļ�ϵͳ��Ŀ¼
	radi::RiakFile* rf = m_riak.GetRoot();
	CPPUNIT_ASSERT(rf != NULL);

	rf->Release();
}

void RiakFSTest::GetFolders()
{
	radi::RiakFile* rf = m_riak.GetRoot();
	CPPUNIT_ASSERT(rf != NULL);
	
	// �г����ڵ��������Ŀ¼
	radi::RiakFileSet* files = rf->GetFiles();
	CPPUNIT_ASSERT(files != NULL);

	// ��ʾĿ¼�����ƺ�key
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

	// �������ƻ�ȡtar����File
	radi::RiakFile* rf = tf->GetRiakFile("wgs84_vector_2to9_Layers");
	CPPUNIT_ASSERT(rf != NULL);

	// ÿ��tar������һ��TileStore
	radi::RiakTileStore* store = rf->GetTileStore();
	//store->GetTiles();

	// ����Key��tilestore�л�ȡtile
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
	//����Ŀ¼
	radi::RiakFile* rfile = root->CreateFolder("addoo");
	rfile->Release();
	root->Release();
}

void RiakFSTest::CreateFile()
{
	radi::RiakFile* root = m_riak.GetRoot();
	radi::RiakFile* rdir = root->GetRiakFile("bbbb");

	radi::RiakFile* rfile = rdir->CreateFile("eeee");
	rfile->Release();
	rdir->Release();
	root->Release();
}

/*
 * RiakFS�е��ļ���Ŀ¼��ʹ��RiakFile�ࡣ�����ļ����͵�RiakFile�����Թ�ȥһ��RiakTileStore���󣬸ö������tile��
 */
void RiakFSTest::GetGeoMeta()
{
	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	radi::RiakFile* rf = root->GetRiakFile("wgs84_vector_2to9_Layers");
	CPPUNIT_ASSERT(rf != NULL);

	radi::RiakTileStore* store = rf->GetTileStore();
	//��ȡtilestore�������ļ�
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

void RiakFSTest::PutCDI()
{
	radi::RiakFile* root = m_riak.GetRoot();
	CPPUNIT_ASSERT(root != NULL);

	radi::RiakFile* rf = root->GetRiakFile("world_bbb");
	CPPUNIT_ASSERT(rf != NULL);

	radi::RiakTileStore* store = rf->GetTileStore();

	//д��CDI�ļ�
	store->PutConfCDI("<?xml  version=\"1.0\"  encoding=\"UTF-8\"?><CacheInfo  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"  xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"  xmlns:typens=\"http://www.esri.com/schemas/ArcGIS/10.1\"  xsi:type=\"typens:CacheInfo\">    </CacheInfo>");

	//store->Release();
	rf->Release();
	root->Release();
}

void RiakFSTest::CreateTileStore()
{
	char uuid[RADI_PATH_MAX];
	memset(uuid, 0, RADI_PATH_MAX);
	radi::radi_uuid_generate(uuid, RADI_PATH_MAX);

	radi::RiakTileStore* store = m_riak.CreateTileStore(uuid, radi::radiTileStorePIGS);
	store->PutGeoMeta("<aa></aa>");
	store->PutGeoMeta("<bb></bb>");
	store->PutConfCDI("<cc></cc>");

	store->Release();
}

void RiakFSTest::ListTileStore()
{
	radi::RiakTileStore* store = NULL;
	radi::RiakTileStoreSet* stores = m_riak.GetTileStores();
	stores->Reset();
	while ((store = stores->Next()) != NULL)
	{
		printf("%s\n", store->GetKey());
	}

	stores->Release();
}
