#include "RiakTileStore.h"
#include "RiakFS.h"
#include "RiakTile.h"
#include <stdio.h>
#include <string.h>

#include <fstream>

namespace radi
{
#define RADI_GEO_META_KEY "geo_meta.xml"
#define RADI_CONF_XML_KEY "Layers[conf.xml"
#define RADI_CONF_CDI_KEY "Layers[conf.cdi"

	RiakTileStore::RiakTileStore()
	{
		m_riak_fs = NULL;
	}

	RiakTileStore::RiakTileStore(const char* name, const char* key, RiakFS* riak_fs)
	{
		if (name != NULL)
		{
			m_name = name;
		}
		if (key != NULL)
		{
			m_key = key;
		}
		m_riak_fs = riak_fs;
	}

	RiakTileStore::~RiakTileStore()
	{

	}

	void RiakTileStore::Release()
	{
		delete this;
	}

	const char* RiakTileStore::GetName()
	{
		return m_name.c_str();
	}

	const char* RiakTileStore::GetKey()
	{
		return m_key.c_str();
	}

	RiakTile* RiakTileStore::GetTile(const char* t_key)
	{
		if (t_key == NULL)
		{
			return NULL;
		}

		riack_client* client = m_riak_fs->GetConnection();

		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_key.c_str(), t_key);
		if (robj == NULL)
		{
			return NULL;
		}

		int count = robj->object.content_count;
		if (!count)
		{
			riack_free_get_object_p(client, &robj);
			return NULL;
		}

		riack_content& r_content = robj->object.content[0];

		size_t size = r_content.data_len;
		unsigned char* data = r_content.data;

		RiakTile* tile = new RiakTile();
		tile->Create(data, size);

		riack_free_get_object_p(client, &robj);
		return tile;
	}

	bool RiakTileStore::PutTile(const char* t_key, const unsigned char* t_data, size_t t_size, const char* content_type)
	{
		if (t_key == NULL || t_data == NULL || content_type == NULL)
		{
			return false;
		}

		riack_client* client = m_riak_fs->GetConnection();
		
		//riack_object *robj = NULL;
		//robj = riack_object_alloc(client);
		//robj->bucket.value = (char*)m_key.c_str();
		//robj->bucket.len = m_key.length();
		//robj->key.value = (char*)t_key;
		//robj->key.len = strlen(t_key);
		//robj->content_count = 1;
		//robj->content = (riack_content*)malloc(sizeof(riack_content));
		//memset(robj->content, 0, sizeof(riack_content));
		//robj->content[0].content_type.value = (char*)content_type;
		//robj->content[0].content_type.len = strlen(content_type);
		//robj->content[0].data = (unsigned char*)t_data;
		//robj->content[0].data_len = t_size;

		unsigned char* data = (unsigned char*)malloc(t_size);
		memcpy(data, t_data, t_size);

		riack_object *robj = NULL;
		robj = riack_object_alloc(client);
		robj->bucket.value = strdup(m_key.c_str());
		robj->bucket.len = m_key.length();
		robj->key.value = strdup(t_key);
		robj->key.len = strlen(t_key);
		robj->content_count = 1;
		robj->content = (riack_content*)malloc(sizeof(riack_content));
		memset(robj->content, 0, sizeof(riack_content));
		robj->content[0].content_type.value = strdup(content_type);
		robj->content[0].content_type.len = strlen(content_type);
		robj->content[0].data = (unsigned char*)data;
		robj->content[0].data_len = t_size;

		riack_put(client, robj, NULL, NULL);

		riack_free_object_p(client, &robj);

		return true;
	}

	bool RiakTileStore::PutTile(const char* t_key, const char* t_path)
	{
		if (t_key == NULL || t_path == NULL)
		{
			return false;
		}

		const char* img_type = "image/png";

		FILE*fp = fopen(t_path, "rb");
		if (fp == NULL)
		{
			return false;
		}

		fseek(fp, 0, SEEK_SET);
		fseek(fp, 0, SEEK_END);

		size_t t_size = ftell(fp);
		unsigned char* t_data = (unsigned char*)malloc(t_size);
		memset(t_data, 0, t_size);

		fseek(fp, 0, SEEK_SET);
		size_t nread = fread(t_data, sizeof(unsigned char), t_size, fp);
		if (nread < t_size)
		{
			free(t_data);
			fclose(fp);
			return false;
		}

		bool ret = true;
		ret = PutTile(t_key, t_data, t_size, img_type);

		free(t_data);
		fclose(fp);
		return ret;
	}

	void RiakTileStore::GetTiles()
	{
		riack_client* client = m_riak_fs->GetConnection();
		if (!client)
		{
			return;
		}

		riack_string_linked_list *list;
		riack_string_linked_list *rkey;
		riack_string bucket;
		bucket.value = (char*)m_key.c_str();
		bucket.len = m_key.length();

		int ret = riack_list_keys(client, &bucket, &list);

		std::ofstream os("g:\\temp\\_aaa.txt");

		char key[RADI_PATH_MAX];
		for (rkey = list; rkey != NULL; rkey = rkey->next)
		{
			memset(key, 0, RADI_PATH_MAX);
			memcpy(key, rkey->string.value, rkey->string.len);
			//printf("[key]:%s\n", key);
			os << key << std::endl;
		}

		os.close();

		riack_free_string_linked_list_p(client, &list);
	}

	const char* RiakTileStore::GetGeoMeta()
	{
		const char* c_key = RADI_GEO_META_KEY;

		riack_client* client = m_riak_fs->GetConnection();

		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_key.c_str(), c_key);
		if (robj == NULL)
		{
			return NULL;
		}

		int count = robj->object.content_count;
		if (!count)
		{
			riack_free_get_object_p(client, &robj);
			return NULL;
		}

		riack_content& r_content = robj->object.content[0];

		size_t size = r_content.data_len;
		unsigned char* data = r_content.data;

		m_geo_meta.assign((const char*)data, size);
		riack_free_get_object_p(client, &robj);

		return m_geo_meta.c_str();
	}

	const char* RiakTileStore::GetConfXML()
	{
		const char* c_key = RADI_CONF_XML_KEY;

		riack_client* client = m_riak_fs->GetConnection();

		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_key.c_str(), c_key);
		if (robj == NULL)
		{
			return NULL;
		}

		int count = robj->object.content_count;
		if (!count)
		{
			riack_free_get_object_p(client, &robj);
			return NULL;
		}

		riack_content& r_content = robj->object.content[0];

		size_t size = r_content.data_len;
		unsigned char* data = r_content.data;

		m_conf_xml.assign((const char*)data, size);
		riack_free_get_object_p(client, &robj);

		return m_conf_xml.c_str();
	}

	const char* RiakTileStore::GetConfCDI()
	{
		const char* c_key = RADI_CONF_CDI_KEY;

		riack_client* client = m_riak_fs->GetConnection();

		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_key.c_str(), c_key);
		if (robj == NULL)
		{
			return NULL;
		}

		int count = robj->object.content_count;
		if (!count)
		{
			riack_free_get_object_p(client, &robj);
			return NULL;
		}

		riack_content& r_content = robj->object.content[0];

		size_t size = r_content.data_len;
		unsigned char* data = r_content.data;

		m_conf_cdi.assign((const char*)data, size);
		riack_free_get_object_p(client, &robj);

		return m_conf_cdi.c_str();
	}

	bool RiakTileStore::PutGeoMeta(const char* data)
	{
		if (!data)
		{
			return false;
		}

		return PutTile(RADI_GEO_META_KEY, (unsigned char*)data, strlen(data), "text/plain");
	}

	bool RiakTileStore::PutConfXML(const char* data)
	{
		if (!data)
		{
			return false;
		}

		return PutTile(RADI_CONF_XML_KEY, (unsigned char*)data, strlen(data), "text/plain");
	}

	bool RiakTileStore::PutConfCDI(const char* data)
	{
		if (!data)
		{
			return false;
		}

		return PutTile(RADI_CONF_CDI_KEY, (unsigned char*)data, strlen(data), "text/plain");
	}

	bool RiakTileStore::DeleteTile(const char* t_key)
	{
		if (!t_key)
		{
			return false;
		}

		riack_client* client = m_riak_fs->GetConnection();

		riack_string bucket, key;
		bucket.value = (char*)m_key.c_str();
		bucket.len = m_key.length();
		key.value = (char*)t_key;
		key.len = strlen(t_key);

		int ret = riack_delete(client, &bucket, &key, NULL);

		return (ret==RIACK_SUCCESS);
	}

	bool RiakTileStore::PutStoreMetaPGIS()
	{
		const char* cdi = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><EnvelopeN xsi:type='typens:EnvelopeN' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><XMin>-198.00000610351563</XMin><YMin>-99.000006103515631</YMin><XMax>198.00012817382813</XMax><YMax>99.000128173828131</YMax></EnvelopeN>";
		const char* xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:GeographicCoordinateSystem'><WKT>GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]]</WKT><XOrigin>-399.99999999999989</XOrigin><YOrigin>-399.99999999999989</YOrigin><XYScale>11258999068426.24</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>8.983152841195215e-009</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><LeftLongitude>-180</LeftLongitude><WKID>4326</WKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-256</X><Y>256</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>90</DPI><LODInfos xsi:type='typens:ArrayOfLODInfo'><LODInfo xsi:type='typens:LODInfo'><LevelID>0</LevelID><Scale>196999200.3297804</Scale><Resolution>0.50000204305542406</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>1</LevelID><Scale>98499600.1648902</Scale><Resolution>0.25000102152771203</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>2</LevelID><Scale>49249800.0824451</Scale><Resolution>0.12500051076385602</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>3</LevelID><Scale>24624900.04122255</Scale><Resolution>0.062500255381928008</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>4</LevelID><Scale>12312450.020611275</Scale><Resolution>0.031250127690964004</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>5</LevelID><Scale>6156225.0103056375</Scale><Resolution>0.015625063845482002</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>6</LevelID><Scale>3078112.5051528187</Scale><Resolution>0.007812531922741001</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>7</LevelID><Scale>1539056.2525763882</Scale><Resolution>0.0039062659613704467</Resolution></LODInfo></LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG32</CacheTileFormat><CompressionQuality>0</CompressionQuality><Antialiasing>true</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>";
		const char* meta= "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?><GeoBeans>  <TileStore>  <Name>wgs84_vector_2to9_Layers</Name>    <Description>wgs84_vector_2to9_Layers</Description>    <TilePath>G:\Data\tar\vector\wgs84_vector_2to9_Layers.tar</TilePath>    <IndexPath>.\wgs84_vector_2to9_Layers.idx\</IndexPath>    <Projection>PGIS</Projection>    <Bound>-256.0,-128.0,256.0,128.0</Bound>    <TileSize>  <Width>256</Width>    <Height>256</Height>  </TileSize>    <TileName>png</TileName>    <StartLevel>2</StartLevel>    <EndLevel>9</EndLevel>    <Nodes>a,b,c,d</Nodes>  </TileStore>  </GeoBeans>";
		PutConfCDI(cdi);
		PutConfXML(xml);
		PutGeoMeta(meta);
		return true;
	}
}