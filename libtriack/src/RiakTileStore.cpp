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

}