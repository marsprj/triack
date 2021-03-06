#include "RiakTileStore.h"
#include "RiakFS.h"
#include "RiakTile.h"
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <sstream>

#include <libxml/parser.h>
#include <libxml/parserInternals.h>

namespace radi
{
#define RADI_GEO_META_KEY "geo_meta.xml"
#define RADI_CONF_XML_KEY "Layers[conf.xml"
#define RADI_CONF_CDI_KEY "Layers[conf.cdi"

	char* tile_scales[] = { "1", "1",
		"393998400.6595608", "1.0000040861108481",
		"196999200.3297804", "0.50000204305542406",
		"98499600.1648902", "0.25000102152771203",
		"49249800.0824451", "0.12500051076385602",
		"24624900.04122255", "0.062500255381928008",
		"12312450.020611275", "0.031250127690964004",
		"6156225.0103056375", "0.015625063845482002",
		"3078112.5051528187", "0.007812531922741001",
		"1539056.2525763882", "0.0039062659613704467",
		"769528.12628819409", "0.0019531329806852234",
		"384764.063144113", "0.00097656649034265201",
		"192382.0315720379", "0.00048828324517127884",
		"96191.015786038872", "0.00024414162258569",
		"48095.507893019436", "0.000122070811292845",
		"24047.753946509718", "6.10354056464225e-005",
		"12023.876973235261", "3.0517702823161505e-005",
		"6011.9384866176306", "1.5258851411580753e-005",
		"3005.9692433283726", "7.6294257058400141e-006",
		"1502.9846216446081", "3.8147128528703159e-006",
		"751.49038700791209", "1.9073515436177414e-006" };

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

		int ret = riack_put(client, robj, NULL, NULL);
		if (ret != RIACK_SUCCESS)
		{
			char msg[RADI_PATH_MAX];
			sprintf(msg, "Imporing [%s]:%s", t_key, client->last_error);
			m_riak_fs->m_logger.Error(msg, __FILE__, __LINE__);
		}

		riack_free_object_p(client, &robj);

		return (ret==RIACK_SUCCESS);
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

	bool RiakTileStore::PutTile(int level, int row, int col, const char* t_path)
	{
		char key[RADI_PATH_MAX];
		sprintf(key, "%dx%dx%d", level, row, col);
		return PutTile(key, t_path);
	}

	bool RiakTileStore::PutTile(int level, int row, int col, const unsigned char* t_data, size_t size, const char* content_type)
	{
		char key[RADI_PATH_MAX];
		sprintf(key, "%dx%dx%d", level, row, col);
		return PutTile(key, t_data, size, content_type);
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

	bool RiakTileStore::PutGeoMeta(int start_level, int end_level)
	{
		//char start[RADI_PATH_MAX];
		//char end[RADI_PATH_MAX];
		//sprintf(start, "%d", start_level);
		//sprintf(end, "%d", end_level);
		//std::string meta = "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?><GeoBeans>  <TileStore>  <Name>wgs84_vector_2to9_Layers</Name>    <Description>wgs84_vector_2to9_Layers</Description>    <TilePath>G:\\Data\\tar\\vector\\wgs84_vector_2to9_Layers.tar</TilePath>    <IndexPath>.\\wgs84_vector_2to9_Layers.idx\\</IndexPath>    <Projection>PGIS</Projection>    <Bound>-256.0,-128.0,256.0,128.0</Bound>    <TileSize>  <Width>256</Width>    <Height>256</Height>  </TileSize>    <TileName>png</TileName>    <StartLevel>");
		//meta.append(start);
		//meta.append("< / StartLevel > <EndLevel>");
		//meta.append(end);
		//meta.append("< / EndLevel > <Nodes>a, b, c, d< / Nodes>  < / TileStore>  < / GeoBeans>");

		//return PutTile(RADI_GEO_META_KEY, (unsigned char*)meta.c_str, meta.length(), "text/plain");

		const char* name = m_name.c_str();
		std::stringstream ss;
		ss << "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?>";
		ss << "<GeoBeans>";
		ss << "    <TileStore>";
		ss << "        <Name>" << name << "</Name>";
		ss << "        <Description>" << name << "</Description>";
		ss << "        <TilePath>.\\" << name << ".tar</TilePath>";
		ss << "        <IndexPath>.\\" << name << ".idx\\</IndexPath>";
		ss << "        <Projection>PGIS</Projection>";
		ss << "        <Bound>-256.0,-256.0,256.0,256.0</Bound>";
		ss << "        <TileSize>";
		ss << "            <Width>256</Width>";
		ss << "            <Height>256</Height>";
		ss << "        </TileSize>";
		ss << "        <TileName>png</TileName>";
		ss << "        <StartLevel>" << start_level << "</StartLevel>";
		ss << "        <EndLevel>" << end_level << "</EndLevel>";
		ss << "        <Nodes>a,b,c,d</Nodes>";
		ss << "    </TileStore>";
		ss << "</GeoBeans>";

		std::string meta = ss.str();
		return PutTile(RADI_GEO_META_KEY, (unsigned char*)meta.c_str(), meta.length(), "text/plain");
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

	bool RiakTileStore::DeleteAllTiles()
	{
		riack_client* client = m_riak_fs->GetConnection();
		if (!client)
		{
			return false;
		}

		riack_string_linked_list *list;
		riack_string_linked_list *rkey;
		riack_string bucket;
		bucket.value = (char*)m_key.c_str();
		bucket.len = m_key.length();

		int ret = riack_list_keys(client, &bucket, &list);

		char key[RADI_PATH_MAX];
		for (rkey = list; rkey != NULL; rkey = rkey->next)
		{
			memset(key, 0, RADI_PATH_MAX);
			memcpy(key, rkey->string.value, rkey->string.len);
			DeleteTile(key);
		}

		riack_free_string_linked_list_p(client, &list);
		return true;
		return true;
	}

	bool RiakTileStore::PutStoreMeta()
	{
		return PutStoreMetaPGIS();
	}

	bool RiakTileStore::PutStoreMetaPGIS()
	{
		const char* cdi = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><EnvelopeN xsi:type='typens:EnvelopeN' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><XMin>-198.00000610351563</XMin><YMin>-99.000006103515631</YMin><XMax>198.00012817382813</XMax><YMax>99.000128173828131</YMax></EnvelopeN>";
		const char* xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:GeographicCoordinateSystem'><WKT>GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]]</WKT><XOrigin>-399.99999999999989</XOrigin><YOrigin>-399.99999999999989</YOrigin><XYScale>11258999068426.24</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>8.9831528411952133e-009</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><LeftLongitude>-180</LeftLongitude><WKID>4326</WKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-256</X><Y>256</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>90</DPI><LODInfos xsi:type='typens:ArrayOfLODInfo'><LODInfo xsi:type='typens:LODInfo'><LevelID>0</LevelID><Scale>393998400.6595608</Scale><Resolution>1.0000040861108481</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>1</LevelID><Scale>196999200.3297804</Scale><Resolution>0.50000204305542406</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>2</LevelID><Scale>98499600.1648902</Scale><Resolution>0.25000102152771203</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>3</LevelID><Scale>49249800.0824451</Scale><Resolution>0.12500051076385602</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>4</LevelID><Scale>24624900.04122255</Scale><Resolution>0.062500255381928008</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>5</LevelID><Scale>12312450.020611275</Scale><Resolution>0.031250127690964004</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>6</LevelID><Scale>6156225.0103056375</Scale><Resolution>0.015625063845482002</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>7</LevelID><Scale>3078112.5051528187</Scale><Resolution>0.007812531922741001</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>8</LevelID><Scale>1539056.2525763882</Scale><Resolution>0.0039062659613704467</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>9</LevelID><Scale>769528.12628819409</Scale><Resolution>0.0019531329806852234</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>10</LevelID><Scale>384764.063144113</Scale><Resolution>0.00097656649034265201</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>11</LevelID><Scale>192382.0315720379</Scale><Resolution>0.00048828324517127884</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>12</LevelID><Scale>96191.015786038872</Scale><Resolution>0.00024414162258569</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>13</LevelID><Scale>48095.507893019436</Scale><Resolution>0.000122070811292845</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>14</LevelID><Scale>24047.753946509718</Scale><Resolution>6.10354056464225e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>15</LevelID><Scale>12023.876973235261</Scale><Resolution>3.0517702823161505e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>16</LevelID><Scale>6011.9384866176306</Scale><Resolution>1.5258851411580753e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>17</LevelID><Scale>3005.9692433283726</Scale><Resolution>7.6294257058400141e-006</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>18</LevelID><Scale>1502.9846216446081</Scale><Resolution>3.8147128528703159e-006</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>19</LevelID><Scale>751.49038700791209</Scale><Resolution>1.9073515436177414e-006</Resolution></LODInfo></LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG</CacheTileFormat><CompressionQuality>75</CompressionQuality><Antialiasing>false</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>";
		const char* meta = "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?><GeoBeans>  <TileStore>  <Name>wgs84_vector_2to9_Layers</Name>    <Description>wgs84_vector_2to9_Layers</Description>    <TilePath>G:\\Data\\tar\\vector\\wgs84_vector_2to9_Layers.tar</TilePath>    <IndexPath>.\\wgs84_vector_2to9_Layers.idx\\</IndexPath>    <Projection>PGIS</Projection>    <Bound>-256.0,-128.0,256.0,128.0</Bound>    <TileSize>  <Width>256</Width>    <Height>256</Height>  </TileSize>    <TileName>png</TileName>    <StartLevel>12</StartLevel>    <EndLevel>14</EndLevel>    <Nodes>a,b,c,d</Nodes>  </TileStore>  </GeoBeans>";
		PutConfCDI(cdi);
		PutConfXML(xml);
		PutGeoMeta(meta);
		return true;
	}

	bool RiakTileStore::PutStoreMetaGoogle()
	{
		const char* cdi = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><EnvelopeN xsi:type='typens:EnvelopeN' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.1'><XMin>-180</XMin><YMin>-180</YMin><XMax>180</XMax><YMax>180</YMax><SpatialReference xsi:type='typens:ProjectedCoordinateSystem'><WKT>PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",3857]]</WKT><XOrigin>-20037700</XOrigin><YOrigin>-30241100</YOrigin><XYScale>10000</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>0.001</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><WKID>102100</WKID><LatestWKID>3857</LatestWKID></SpatialReference></EnvelopeN>";
		const char* xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.1'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:ProjectedCoordinateSystem'><WKT>PROJCS[&quot;WGS_1984_Web_Mercator_Auxiliary_Sphere&quot;,GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]],PROJECTION[&quot;Mercator_Auxiliary_Sphere&quot;],PARAMETER[&quot;False_Easting&quot;,0.0],PARAMETER[&quot;False_Northing&quot;,0.0],PARAMETER[&quot;Central_Meridian&quot;,0.0],PARAMETER[&quot;Standard_Parallel_1&quot;,0.0],PARAMETER[&quot;Auxiliary_Sphere_Type&quot;,0.0],UNIT[&quot;Meter&quot;,1.0],AUTHORITY[&quot;EPSG&quot;,3857]]</WKT><XOrigin>-22041196.5579357</XOrigin><YOrigin>-30241100</YOrigin><XYScale>144148177.11745703</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>0.001</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><WKID>102100</WKID><LatestWKID>3857</LatestWKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-180</X><Y>90</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>96</DPI><PreciseDPI>96</PreciseDPI><LODInfos xsi:type='typens:ArrayOfLODInfo'><LODInfo xsi:type='typens:LODInfo'><LevelID>0</LevelID><Scale>591657527.591555</Scale><Resolution>156543.03392800014</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>1</LevelID><Scale>295828763.79577702</Scale><Resolution>78271.516963999937</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>2</LevelID><Scale>147914381.89788899</Scale><Resolution>39135.758482000092</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>3</LevelID><Scale>73957190.948944002</Scale><Resolution>19567.879240999919</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>4</LevelID><Scale>36978595.474472001</Scale><Resolution>9783.9396204999593</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>5</LevelID><Scale>18489297.737236001</Scale><Resolution>4891.9698102499797</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>6</LevelID><Scale>9244648.8686180003</Scale><Resolution>2445.9849051249898</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>7</LevelID><Scale>4622324.4343090001</Scale><Resolution>1222.9924525624949</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>8</LevelID><Scale>2311162.2171550002</Scale><Resolution>611.49622628137968</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>9</LevelID><Scale>1155581.108577</Scale><Resolution>305.74811314055756</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>10</LevelID><Scale>577790.55428899999</Scale><Resolution>152.87405657041106</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>11</LevelID><Scale>288895.27714399999</Scale><Resolution>76.437028285073239</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>12</LevelID><Scale>144447.638572</Scale><Resolution>38.21851414253662</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>13</LevelID><Scale>72223.819285999998</Scale><Resolution>19.10925707126831</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>14</LevelID><Scale>36111.909642999999</Scale><Resolution>9.5546285356341549</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>15</LevelID><Scale>18055.954822</Scale><Resolution>4.7773142679493699</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>16</LevelID><Scale>9027.9774109999998</Scale><Resolution>2.3886571339746849</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>17</LevelID><Scale>4513.9887049999998</Scale><Resolution>1.1943285668550503</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>18</LevelID><Scale>2256.994353</Scale><Resolution>0.59716428355981721</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>19</LevelID><Scale>1128.4971760000001</Scale><Resolution>0.29858214164761665</Resolution></LODInfo></LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG32</CacheTileFormat><CompressionQuality>0</CompressionQuality><Antialiasing>false</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>";
		const char* meta = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GeoBeans>  <TileStore>    <Name>esri</Name>    <Description>esri</Description>    <TilePath>esri.tar</TilePath>    <IndexPath>.\esri.idx\</IndexPath>    <Projection>srid=80004,PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",3857]]</Projection>    <TotalBound>-39135.7578125,6692214.5859375,.0,6731350.34375</TotalBound>    <Bound>-39135.7578125,6692214.5859375,.0,6731350.34375</Bound>    <TileSize>      <Width>256</Width>      <Height>256</Height>    </TileSize>    <TileName>google.png</TileName>    <StartLevel>11</StartLevel>    <EndLevel>13</EndLevel>    <Nodes>a,b,c,d</Nodes>  </TileStore></GeoBeans>";
		PutConfCDI(cdi);
		PutConfXML(xml);
		PutGeoMeta(meta);
		return true;
	}

	bool RiakTileStore::PutStoreMetaGoogle(int start_level, int end_level, double xmin, double ymin, double xmax, double ymax)
	{
		const char* cdi = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><EnvelopeN xsi:type='typens:EnvelopeN' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.1'><XMin>-180</XMin><YMin>-180</YMin><XMax>180</XMax><YMax>180</YMax><SpatialReference xsi:type='typens:ProjectedCoordinateSystem'><WKT>PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",3857]]</WKT><XOrigin>-20037700</XOrigin><YOrigin>-30241100</YOrigin><XYScale>10000</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>0.001</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><WKID>102100</WKID><LatestWKID>3857</LatestWKID></SpatialReference></EnvelopeN>";
		const char* xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.1'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:ProjectedCoordinateSystem'><WKT>PROJCS[&quot;WGS_1984_Web_Mercator_Auxiliary_Sphere&quot;,GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]],PROJECTION[&quot;Mercator_Auxiliary_Sphere&quot;],PARAMETER[&quot;False_Easting&quot;,0.0],PARAMETER[&quot;False_Northing&quot;,0.0],PARAMETER[&quot;Central_Meridian&quot;,0.0],PARAMETER[&quot;Standard_Parallel_1&quot;,0.0],PARAMETER[&quot;Auxiliary_Sphere_Type&quot;,0.0],UNIT[&quot;Meter&quot;,1.0],AUTHORITY[&quot;EPSG&quot;,3857]]</WKT><XOrigin>-22041196.5579357</XOrigin><YOrigin>-30241100</YOrigin><XYScale>144148177.11745703</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>0.001</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><WKID>102100</WKID><LatestWKID>3857</LatestWKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-180</X><Y>90</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>96</DPI><PreciseDPI>96</PreciseDPI><LODInfos xsi:type='typens:ArrayOfLODInfo'><LODInfo xsi:type='typens:LODInfo'><LevelID>0</LevelID><Scale>591657527.591555</Scale><Resolution>156543.03392800014</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>1</LevelID><Scale>295828763.79577702</Scale><Resolution>78271.516963999937</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>2</LevelID><Scale>147914381.89788899</Scale><Resolution>39135.758482000092</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>3</LevelID><Scale>73957190.948944002</Scale><Resolution>19567.879240999919</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>4</LevelID><Scale>36978595.474472001</Scale><Resolution>9783.9396204999593</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>5</LevelID><Scale>18489297.737236001</Scale><Resolution>4891.9698102499797</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>6</LevelID><Scale>9244648.8686180003</Scale><Resolution>2445.9849051249898</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>7</LevelID><Scale>4622324.4343090001</Scale><Resolution>1222.9924525624949</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>8</LevelID><Scale>2311162.2171550002</Scale><Resolution>611.49622628137968</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>9</LevelID><Scale>1155581.108577</Scale><Resolution>305.74811314055756</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>10</LevelID><Scale>577790.55428899999</Scale><Resolution>152.87405657041106</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>11</LevelID><Scale>288895.27714399999</Scale><Resolution>76.437028285073239</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>12</LevelID><Scale>144447.638572</Scale><Resolution>38.21851414253662</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>13</LevelID><Scale>72223.819285999998</Scale><Resolution>19.10925707126831</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>14</LevelID><Scale>36111.909642999999</Scale><Resolution>9.5546285356341549</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>15</LevelID><Scale>18055.954822</Scale><Resolution>4.7773142679493699</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>16</LevelID><Scale>9027.9774109999998</Scale><Resolution>2.3886571339746849</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>17</LevelID><Scale>4513.9887049999998</Scale><Resolution>1.1943285668550503</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>18</LevelID><Scale>2256.994353</Scale><Resolution>0.59716428355981721</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>19</LevelID><Scale>1128.4971760000001</Scale><Resolution>0.29858214164761665</Resolution></LODInfo></LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG32</CacheTileFormat><CompressionQuality>0</CompressionQuality><Antialiasing>false</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>";
		//const char* meta = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GeoBeans>  <TileStore>    <Name>esri</Name>    <Description>esri</Description>    <TilePath>esri.tar</TilePath>    <IndexPath>.\esri.idx\</IndexPath>    <Projection>srid=80004,PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",3857]]</Projection>    <TotalBound>-39135.7578125,6692214.5859375,.0,6731350.34375</TotalBound>    <Bound>-39135.7578125,6692214.5859375,.0,6731350.34375</Bound>    <TileSize>      <Width>256</Width>      <Height>256</Height>    </TileSize>    <TileName>google.png</TileName>    <StartLevel>11</StartLevel>    <EndLevel>13</EndLevel>    <Nodes>a,b,c,d</Nodes>  </TileStore></GeoBeans>";
		PutConfCDI(cdi);
		PutConfXML(xml);
		PutGeoMetaGoogle(start_level, end_level);

		return true;
	}

	bool RiakTileStore::PutGeoMetaGoogle(int start_level, int end_level)
	{
		std::stringstream ss;
		//ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GeoBeans>  <TileStore>    <Name>esri</Name>    <Description>esri</Description>    <TilePath>esri.tar</TilePath>    <IndexPath>.\esri.idx\</IndexPath>    <Projection>srid=80004,PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0],AUTHORITY[\"EPSG\",3857]]</Projection>    <TotalBound>-39135.7578125,6692214.5859375,.0,6731350.34375</TotalBound>    <Bound>-39135.7578125,6692214.5859375,.0,6731350.34375</Bound>    <TileSize>      <Width>256</Width>      <Height>256</Height>    </TileSize>    <TileName>google.png</TileName>    <StartLevel>";
		//ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GeoBeans>  <TileStore>    <Name>esri</Name>    <Description>esri</Description>    <TilePath>esri.tar</TilePath>    <IndexPath>.\esri.idx\</IndexPath>    <Projection>GOOGLE_CRS84_QUAD</Projection>    <TotalBound>-39135.7578125,6692214.5859375,.0,6731350.34375</TotalBound>    <Bound>-39135.7578125,6692214.5859375,.0,6731350.34375</Bound>    <TileSize>      <Width>256</Width>      <Height>256</Height>    </TileSize>    <TileName>png</TileName>    <StartLevel>";
		ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><GeoBeans>  <TileStore>    <Name>esri</Name>    <Description>esri</Description>    <TilePath>esri.tar</TilePath>    <IndexPath>.\esri.idx\</IndexPath>    <Projection>GOOGLE_CRS84_QUAD</Projection>    <TotalBound>-180,-180</TotalBound>    <Bound>180,180</Bound>    <TileSize>      <Width>256</Width>      <Height>256</Height>    </TileSize>    <TileName>png</TileName>    <StartLevel>";
		ss << start_level;
		ss << "</StartLevel> <EndLevel>";
		ss << end_level;
		ss << "</EndLevel> <Nodes>a, b, c, d</Nodes>  </TileStore></GeoBeans>";
		std::string meta = ss.str();
		PutGeoMeta(meta.c_str());
		
		return true;
	}

	bool RiakTileStore::PutStoreMetaPGIS(int start_level, int end_level, double xmin, double ymin, double xmax, double ymax)
	{
		//const char* cdi = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><EnvelopeN xsi:type='typens:EnvelopeN' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><XMin>-198.00000610351563</XMin><YMin>-99.000006103515631</YMin><XMax>198.00012817382813</XMax><YMax>99.000128173828131</YMax></EnvelopeN>";
		//const char* xml = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:GeographicCoordinateSystem'><WKT>GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]]</WKT><XOrigin>-399.99999999999989</XOrigin><YOrigin>-399.99999999999989</YOrigin><XYScale>11258999068426.24</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>8.9831528411952133e-009</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><LeftLongitude>-180</LeftLongitude><WKID>4326</WKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-256</X><Y>256</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>90</DPI><LODInfos xsi:type='typens:ArrayOfLODInfo'><LODInfo xsi:type='typens:LODInfo'><LevelID>0</LevelID><Scale>393998400.6595608</Scale><Resolution>1.0000040861108481</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>1</LevelID><Scale>196999200.3297804</Scale><Resolution>0.50000204305542406</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>2</LevelID><Scale>98499600.1648902</Scale><Resolution>0.25000102152771203</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>3</LevelID><Scale>49249800.0824451</Scale><Resolution>0.12500051076385602</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>4</LevelID><Scale>24624900.04122255</Scale><Resolution>0.062500255381928008</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>5</LevelID><Scale>12312450.020611275</Scale><Resolution>0.031250127690964004</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>6</LevelID><Scale>6156225.0103056375</Scale><Resolution>0.015625063845482002</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>7</LevelID><Scale>3078112.5051528187</Scale><Resolution>0.007812531922741001</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>8</LevelID><Scale>1539056.2525763882</Scale><Resolution>0.0039062659613704467</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>9</LevelID><Scale>769528.12628819409</Scale><Resolution>0.0019531329806852234</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>10</LevelID><Scale>384764.063144113</Scale><Resolution>0.00097656649034265201</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>11</LevelID><Scale>192382.0315720379</Scale><Resolution>0.00048828324517127884</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>12</LevelID><Scale>96191.015786038872</Scale><Resolution>0.00024414162258569</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>13</LevelID><Scale>48095.507893019436</Scale><Resolution>0.000122070811292845</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>14</LevelID><Scale>24047.753946509718</Scale><Resolution>6.10354056464225e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>15</LevelID><Scale>12023.876973235261</Scale><Resolution>3.0517702823161505e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>16</LevelID><Scale>6011.9384866176306</Scale><Resolution>1.5258851411580753e-005</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>17</LevelID><Scale>3005.9692433283726</Scale><Resolution>7.6294257058400141e-006</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>18</LevelID><Scale>1502.9846216446081</Scale><Resolution>3.8147128528703159e-006</Resolution></LODInfo><LODInfo xsi:type='typens:LODInfo'><LevelID>19</LevelID><Scale>751.49038700791209</Scale><Resolution>1.9073515436177414e-006</Resolution></LODInfo></LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG</CacheTileFormat><CompressionQuality>75</CompressionQuality><Antialiasing>false</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>";
		//const char* meta = "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?><GeoBeans>  <TileStore>  <Name>wgs84_vector_2to9_Layers</Name>    <Description>wgs84_vector_2to9_Layers</Description>    <TilePath>G:\\Data\\tar\\vector\\wgs84_vector_2to9_Layers.tar</TilePath>    <IndexPath>.\\wgs84_vector_2to9_Layers.idx\\</IndexPath>    <Projection>PGIS</Projection>    <Bound>-256.0,-128.0,256.0,128.0</Bound>    <TileSize>  <Width>256</Width>    <Height>256</Height>  </TileSize>    <TileName>png</TileName>    <StartLevel>12</StartLevel>    <EndLevel>14</EndLevel>    <Nodes>a,b,c,d</Nodes>  </TileStore>  </GeoBeans>";
		//PutConfCDI(cdi);
		PutConfCDIPGIS(xmin, ymin, xmax, ymax);
		//PutConfXML(xml);
		PutPutConfXMLPGIS(start_level, end_level);
		//PutGeoMeta(meta);
		PutGeoMeta(start_level, end_level);
	
		return true;
	}


	bool RiakTileStore::PutConfCDIPGIS(double xmin, double ymin, double xmax, double ymax)
	{
		std::stringstream ss;
		ss << "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?>";
		ss << "<EnvelopeN  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"  xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"  xmlns:typens=\"http://www.esri.com/schemas/ArcGIS/10.0\"  xsi:type=\"typens:EnvelopeN\">";
		ss << "    <XMin>" << xmin <<"</XMin>";
		ss << "    <YMin>" << ymin <<"</YMin>";
		ss << "    <XMax>" << xmax << "</XMax>";
		ss << "    <YMax>" << ymax << "</YMax>";
		ss << "</EnvelopeN>";
		std::string meta = ss.str();
		printf(meta.c_str());
		return PutTile(RADI_CONF_CDI_KEY, (unsigned char*)meta.c_str(), meta.length(), "text/plain");
	}

	bool RiakTileStore::UpdateConfCDI(double xmin, double ymin, double xmax, double ymax)
	{
		std::stringstream ss;
		ss << "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?>";
		ss << "<EnvelopeN  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"  xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"  xmlns:typens=\"http://www.esri.com/schemas/ArcGIS/10.0\"  xsi:type=\"typens:EnvelopeN\">";
		ss << "    <XMin>" << xmin << "</XMin>";
		ss << "    <YMin>" << ymin << "</YMin>";
		ss << "    <XMax>" << xmax << "</XMax>";
		ss << "    <YMax>" << ymax << "</YMax>";
		ss << "</EnvelopeN>";
		std::string meta = ss.str();
		return PutTile(RADI_CONF_CDI_KEY, (unsigned char*)meta.c_str(), meta.length(), "text/plain");
	}

	bool RiakTileStore::PutPutConfXMLPGIS(int start_level, int end_level)
	{
		char level[_MAX_PATH];
		std::string meta = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><CacheInfo xsi:type='typens:CacheInfo' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xs='http://www.w3.org/2001/XMLSchema' xmlns:typens='http://www.esri.com/schemas/ArcGIS/10.0'><TileCacheInfo xsi:type='typens:TileCacheInfo'><SpatialReference xsi:type='typens:GeographicCoordinateSystem'><WKT>GEOGCS[&quot;GCS_WGS_1984&quot;,DATUM[&quot;D_WGS_1984&quot;,SPHEROID[&quot;WGS_1984&quot;,6378137.0,298.257223563]],PRIMEM[&quot;Greenwich&quot;,0.0],UNIT[&quot;Degree&quot;,0.0174532925199433]]</WKT><XOrigin>-399.99999999999989</XOrigin><YOrigin>-399.99999999999989</YOrigin><XYScale>11258999068426.24</XYScale><ZOrigin>-100000</ZOrigin><ZScale>10000</ZScale><MOrigin>-100000</MOrigin><MScale>10000</MScale><XYTolerance>8.9831528411952133e-009</XYTolerance><ZTolerance>0.001</ZTolerance><MTolerance>0.001</MTolerance><HighPrecision>true</HighPrecision><LeftLongitude>-180</LeftLongitude><WKID>4326</WKID></SpatialReference><TileOrigin xsi:type='typens:PointN'><X>-256</X><Y>256</Y></TileOrigin><TileCols>256</TileCols><TileRows>256</TileRows><DPI>90</DPI><LODInfos xsi:type='typens:ArrayOfLODInfo'>";

		int counter = 0;
		for (int i = start_level; i <= end_level; i++, counter++)
		{
			sprintf(level, "%d", counter);
			meta.append("<LODInfo type = \"typens:LODInfo\">");

			meta.append("<LevelID>");
			meta.append(level);
			meta.append("</LevelID>");

			meta.append("<Scale>");
			meta.append(tile_scales[2 * i]);
			meta.append("</Scale>");

			meta.append("<Resolution>");
			meta.append(tile_scales[2 * i + 1]);
			meta.append("</Resolution>");

			meta.append("</LODInfo>");
		}

		meta.append("</LODInfos></TileCacheInfo><TileImageInfo xsi:type='typens:TileImageInfo'><CacheTileFormat>PNG</CacheTileFormat><CompressionQuality>75</CompressionQuality><Antialiasing>false</Antialiasing></TileImageInfo><CacheStorageInfo xsi:type='typens:CacheStorageInfo'><StorageFormat>esriMapCacheStorageModeExploded</StorageFormat><PacketSize>0</PacketSize></CacheStorageInfo></CacheInfo>");

		PutConfXML(meta.c_str());

		return true;
	}

	g_int64 RiakTileStore::GetVolume()
	{
		riack_client* client = m_riak_fs->GetConnection();
		if (!client)
		{
			return 0;
		}

		g_int64 volume = 0;
		riack_string_linked_list *list;
		riack_string_linked_list *rkey;
		riack_string bucket;
		bucket.value = (char*)m_key.c_str();
		bucket.len = m_key.length();

		int ret = riack_list_keys(client, &bucket, &list);

		char key[RADI_PATH_MAX];
		for (rkey = list; rkey != NULL; rkey = rkey->next)
		{
			memset(key, 0, RADI_PATH_MAX);
			memcpy(key, rkey->string.value, rkey->string.len);

			volume += GetVolume(key);
		}
		
		riack_free_string_linked_list_p(client, &list);
		return volume;
	}

	g_int64 RiakTileStore::GetVolume(const char* key)
	{
		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_key.c_str(), key);
		if (!robj->object.content_count)
		{
			return 0;
		}

		g_int64 volume = robj->object.content[0].data_len;
		riack_free_get_object_p(m_riak_fs->GetConnection(), &robj);
		return volume;
	}

	bool RiakTileStore::UpdateVolume()
	{
		riack_get_object* robj = m_riak_fs->GetRiakObjects(m_riak_fs->m_fs_name.c_str(), m_key.c_str());
		if (!robj->object.content_count)
		{
			return false;
		}

		g_int64 volume = GetVolume();
		//g_int64 volume = 100;

		radi_riack_set_string(robj->object.bucket, m_riak_fs->m_fs_name.c_str());
		radi_riack_set_string(robj->object.key, m_key.c_str());

		riack_content& r_content = robj->object.content[0];	
		riack_pair* r_pair = &(r_content.usermetas[0]);
		for (int i = 0; i < r_content.usermeta_count; i++, r_pair++)
		{
			if (!strncmp("SIZE", r_pair->key.value, r_pair->key.len))
			{
				char str[RADI_PATH_MAX] = { 0 };
				sprintf(str, "%lld", volume);
				r_pair->value_len = strlen(str);
				if (r_pair->value != NULL)
				{
					free(r_pair->value);
				}
				r_pair->value = (uint8_t*)strdup(str);
				r_pair->value_present = 1;
				break;
			}
		}

		riack_put(m_riak_fs->GetConnection(), &(robj->object), NULL, NULL);

		riack_free_get_object_p(m_riak_fs->GetConnection(), &robj);
		
		return true;
	}

	void RiakTileStore::GetInfo(TileStoreInfo& info)
	{
		riack_client* client = m_riak_fs->GetConnection();
		if (!client)
		{
			return;
		}

		info.count = 0;
		info.volume = 0;

		g_int64 volume = 0;
		riack_string_linked_list *list;
		riack_string_linked_list *rkey;
		riack_string bucket;
		bucket.value = (char*)m_key.c_str();
		bucket.len = m_key.length();

		int ret = riack_list_keys(client, &bucket, &list);

		char key[RADI_PATH_MAX];
		for (rkey = list; rkey != NULL; rkey = rkey->next)
		{
			memset(key, 0, RADI_PATH_MAX);
			memcpy(key, rkey->string.value, rkey->string.len);

			info.count++;
			info.volume += GetVolume(key);
		}

		riack_free_string_linked_list_p(client, &list);
	}

	RRect RiakTileStore::GetExtent()
	{
		RRect rect;
		memset(&rect, 0, sizeof(rect));

		const char* cdi = GetConfCDI();

		xmlParserCtxtPtr pxParseCtxt = NULL;
		xmlKeepBlanksDefault(0);
		xmlDoValidityCheckingDefaultValue = 0;
		pxParseCtxt = xmlCreateMemoryParserCtxt((const char*)cdi, strlen(cdi));
		if (pxParseCtxt == NULL)
		{
			return rect;
		}

		pxParseCtxt->linenumbers = 1;
		xmlParseDocument(pxParseCtxt);

		xmlDocPtr pxmlDoc = pxParseCtxt->myDoc;
		if (pxmlDoc == NULL)
		{
			xmlFreeParserCtxt(pxParseCtxt);
			return rect;
		}
		xmlNodePtr pxmlRoot = xmlDocGetRootElement(pxmlDoc);
		if (pxmlDoc == NULL)
		{
			xmlFreeDoc(pxmlDoc);
			xmlFreeParserCtxt(pxParseCtxt);
			return rect;
		}

		for (xmlNode* child = pxmlRoot->children; child; child = child->next)
		{
			const char* nodeName = (const char*)child->name;
			if (stricmp(nodeName, "XMin") == 0)
			{
				rect.xmin = atof((const char*)(child->children[0].content));
			}
			else if (stricmp(nodeName, "YMin") == 0)
			{
				rect.ymin = atof((const char*)(child->children[0].content));
			}
			else if (stricmp(nodeName, "XMax") == 0)
			{
				rect.xmax = atof((const char*)(child->children[0].content));
			}
			else if (stricmp(nodeName, "YMax") == 0)
			{
				rect.ymax = atof((const char*)(child->children[0].content));
			}
		}

		xmlFreeDoc(pxmlDoc);
		xmlFreeParserCtxt(pxParseCtxt);
		return rect;
	}

	bool RiakTileStore::SetExtent(RRect rect)
	{
		std::stringstream ss;
		ss << "<?xml  version=\"1.0\"  encoding=\"UTF-8\"?>";
		ss << "<EnvelopeN  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"  xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"  xmlns:typens=\"http://www.esri.com/schemas/ArcGIS/10.0\"  xsi:type=\"typens:EnvelopeN\">";
		ss << "    <XMin>" << rect.xmin << "</XMin>";
		ss << "    <YMin>" << rect.ymin << "</YMin>";
		ss << "    <XMax>" << rect.xmax << "</XMax>";
		ss << "    <YMax>" << rect.ymax << "</YMax>";
		ss << "</EnvelopeN>";
		std::string meta = ss.str();
		return PutTile(RADI_CONF_CDI_KEY, (unsigned char*)meta.c_str(), meta.length(), "text/plain");
	}
}