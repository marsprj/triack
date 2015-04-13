#include "RiakFS.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"
#include "RiakTileStoreSet.h"
#include "stdlib.h"

#include <time.h>

#ifdef WIN32
#	include <rpc.h>
#endif

#ifdef WIN32
#	include "process.h"
#else
#	include <sys/types.h>
#	include <unistd.h>
#	include <time.h>
#	include <uuid/uuid.h>
#endif

namespace radi
{
	void *radi_riack_alloc(void *optional_data, size_t size)
	{
		return malloc(size);
	}

	void radi_riack_free(void *optional_data, void *pointer)
	{
		if (pointer)
		{
			free(pointer);
		}
	}

	void radi_raick_set_pair(riack_pair& rpair, const char* key, const char* data)
	{
		rpair.value_present = 1;
		rpair.key.value = strdup(key);
		rpair.key.len = strlen(key);
		rpair.value_len = strlen(data);
		rpair.value = (uint8_t*)malloc(sizeof(uint8_t)*rpair.value_len);
		memcpy(rpair.value, data, rpair.value_len);
	}

	void radi_riack_set_link(riack_link& r_link, const char* bucket, const char* key, const char* tag)
	{
		r_link.bucket.value = strdup(bucket);
		r_link.bucket.len = strlen(bucket);
		r_link.key.value = strdup(key);
		r_link.key.len = strlen(key);
		r_link.tag.value = strdup(tag);
		r_link.tag.len = strlen(tag);
	}

	void radi_riack_set_string(riack_string& rstring, const char* value)
	{
		rstring.value = strdup(value);
		rstring.len = strlen(value);
	}


	long long get_current_time_millis()
	{
#ifdef WIN32
		time_t now;
		time(&now);
		return (long long)(now * 1000);
		//return 0;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

	void radi_uuid_generate(char* uuid, size_t size)
	{
#ifdef WIN32
		uuid_t uu;
		UuidCreate(&uu);
		unsigned char* buffer = NULL;
		UuidToString((uuid_t*)&uu, &buffer);
		int len = strlen((char*)buffer);
		memset(uuid, 0, size);
		memcpy(uuid, buffer, len);
#else
		uuid_t uu;
		uuid_generate(uu);
		uuid_unparse(uu, uuid);
#endif
	}

	RiakFS::RiakFS() :
		//m_riak_server("192.168.111.104"),
		//m_riak_server("192.168.111.151"),
		//m_riak_server("192.168.111.86"),
		m_riak_server("123.57.207.198"),
		m_riak_port(8087),
		m_riak(NULL),
		m_fs_name("rfs"),
		m_fs_root("root")
	{
		riack_init();

		m_riak_allocator.alloc = radi_riack_alloc;
		m_riak_allocator.free = radi_riack_free;
	}

	RiakFS::RiakFS(const char* server, int port) :
		m_riak_server(server),
		m_riak_port(port),
		m_riak(NULL),
		m_fs_name("rfs"),
		m_fs_root("root")
	{
		riack_init();

		m_riak_allocator.alloc = radi_riack_alloc;
		m_riak_allocator.free = radi_riack_free;
	}

	RiakFS::~RiakFS()
	{
		if (m_riak != NULL)
		{
			riack_free(m_riak);
			m_riak = NULL;
		}
		riack_cleanup();
	}

	bool RiakFS::Connect()
	{
		if (m_riak != NULL)
		{
			return true;
		}
		m_riak = riack_new_client(&m_riak_allocator);
		//m_riak = riack_new_client(NULL);
		int ret = riack_connect(m_riak, m_riak_server.c_str(), m_riak_port, NULL);
		if (ret != RIACK_SUCCESS)
		{
			riack_free(m_riak);
			m_riak = NULL;
			return false;
		}

		return true;
	}

	void RiakFS::Close()
	{
		if (m_riak != NULL)
		{
			riack_disconnect(m_riak);
			riack_free(m_riak);
			m_riak = NULL;
		}
	}

	riack_client* RiakFS::GetConnection()
	{
		return m_riak;
	}

	//void RiakFS::SetServer(const char* server)
	//{
	//	if (server == NULL)
	//	{
	//		m_riak_server.clear();
	//	}
	//	else
	//	{
	//		m_riak_server = server;
	//	}
	//}

	//void RiakFS::SetPort(int port)
	////void RiakFS::SetPort(int port)
	//{
	//	m_riak_port = port;
	//}

	void RiakFS::Release()
	{
		delete this;
	}

	RiakFile* RiakFS::GetRoot()
	{
		riack_client* client = GetConnection();
		if (client == NULL)
		{
			return NULL;
		}

		riack_string rbucket, rkey;
		riack_get_object *obj;

		rbucket.len = m_fs_name.length();
		rbucket.value = (char*)m_fs_name.c_str();

		rkey.len = m_fs_root.length();
		rkey.value = (char*)m_fs_root.c_str();

		int ret = riack_get(client, &rbucket, &rkey, NULL, &obj);
		if (ret != RIACK_SUCCESS)
		{
			return NULL;
		}

		if (!obj->object.content_count)
		{
			return NULL;
		}


		RiakFile* rf = new RiakFile();
		if (!rf->Create(this, m_fs_root.c_str(), &(obj->object.content[0])))
		{
			riack_free_get_object_p(client, &obj);
			return NULL;
		}

		riack_free_get_object_p(client, &obj);
		return rf;
	}


	RiakFileSet* RiakFS::ListFiles(const char* dir_key)
	{
		riack_get_object* robj = NULL;
		robj = GetRiakObjects(m_fs_name.c_str(), dir_key);
		if (robj == NULL)
		{
			return NULL;
		}

		int n_content = robj->object.content_count;
		if (!n_content)
		{
			riack_free_get_object_p(m_riak, &robj);
			return NULL;
		}

		riack_content& r_content = robj->object.content[0];

		RiakFileSet* files = new RiakFileSet();

		char key[RADI_PATH_MAX];
		int n_links = r_content.link_count;
		for (int i = 0; i < n_links; i++)
		{
			riack_link& r_link = r_content.links[i];
			if (!strncmp(r_link.tag.value, "parent", r_link.tag.len))
			{
				RiakFile* rf = NULL;
				memset(key, 0, RADI_PATH_MAX);
				memcpy(key, r_link.key.value, r_link.key.len);
				rf = GetRiakFile(m_fs_name.c_str(), key);
				if (rf != NULL)
				{
					files->Add(rf);
				}
			}
		}

		riack_free_get_object_p(m_riak, &robj);
		return files;
	}

	RiakFile* RiakFS::GetRiakFileByName(const char* bucket, const char* name)
	{
		riack_get_object* robj = NULL;
		robj = GetRiakObjects(m_fs_name.c_str(), bucket);
		if (robj == NULL)
		{
			return NULL;
		}

		int n_content = robj->object.content_count;
		if (!n_content)
		{
			riack_free_get_object_p(m_riak, &robj);
			return NULL;
		}

		RiakFile* rf = NULL;
		riack_content& r_content = robj->object.content[0];

		char key[RADI_PATH_MAX];
		int n_links = r_content.link_count;
		for (int i = 0; i < n_links; i++)
		{
			riack_link& r_link = r_content.links[i];
			if (!strncmp(r_link.tag.value, "parent", r_link.tag.len))
			{
				memset(key, 0, RADI_PATH_MAX);
				memcpy(key, r_link.key.value, r_link.key.len);
				rf = GetRiakFile(m_fs_name.c_str(), key);
				if (rf != NULL)
				{
					//printf("%s\n", rf->GetName());
					if (!strcmp(rf->GetName(), name))
					{
						break;
					}
					else
					{
						rf->Release();
						rf = NULL;
					}
				}
			}
		}

		riack_free_get_object_p(m_riak, &robj);
		return rf;
	}

	riack_get_object* RiakFS::GetRiakObjects(const char* bucket, const char* key)
	{
		riack_string rbucket, rkey;
		riack_get_object *obj;

		rbucket.len = strlen(bucket);
		rbucket.value = (char*)bucket;

		rkey.len = strlen(key);
		rkey.value = (char*)key;

		int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &obj);
		if (ret != RIACK_SUCCESS)
		{
			return NULL;
		}

		return obj;
	}

	RiakFile* RiakFS::GetRiakFile(const char* bucket, const char* key)
	{
		riack_client* client = GetConnection();
		if (client == NULL)
		{
			return NULL;
		}

		riack_string rbucket, rkey;
		riack_get_object *robj = NULL;

		rbucket.len = strlen(bucket);
		rbucket.value = (char*)bucket;

		rkey.len = strlen(key);
		rkey.value = (char*)key;

		int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &robj);
		if (ret != RIACK_SUCCESS)
		{
			return NULL;
		}

		int count = robj->object.content_count;
		if (!count)
		{
			riack_free_get_object_p(m_riak, &robj);
			return NULL;
		}

		RiakFile* rf = NULL;
		riack_content& r_content = robj->object.content[0];

		rf = new RiakFile();
		rf->Create(this, key, &r_content);

		riack_free_get_object_p(m_riak, &robj);
		return rf;
	}

	bool RiakFS::CreateRiakFolder(const char* parent_key, const char* f_name)
	{
		if (parent_key == NULL || f_name == NULL)
		{
			return false;
		}

		RiakFile* pf = GetRiakFile(m_fs_name.c_str(), parent_key);
		if (pf==NULL)
		{
			pf->Release();
			return false;
		}

		bool ret = false;
		char f_key[RADI_PATH_MAX];
		radi_uuid_generate(f_key, RADI_PATH_MAX);
		if (!CreateRiakFolderObj(f_name, f_key, parent_key))
		{
			pf->Release();
			return NULL;
		}

		// add link
		pf->AddLink(f_key);

		pf->Release();
		return true;
	}

	bool RiakFS::CreateRiakFile(const char* parent_key, const char* f_name, const char* data_type/*="PGIS"*/)
	{
		if (parent_key == NULL || f_name == NULL)
		{
			return false;
		}

		RiakFile* pf = GetRiakFile(m_fs_name.c_str(), parent_key);
		if (pf==NULL)
		{
			return false;
		}
		
		bool ret = false;
		char f_key[RADI_PATH_MAX];
		radi_uuid_generate(f_key, RADI_PATH_MAX);
		if (!CreateRiakFileObj(f_name, f_key, parent_key, data_type))
		{
			pf->Release();
			return NULL;
		}

		// add link
		pf->AddLink(f_key);

		pf->Release();
		return true;
	}

	bool RiakFS::CreateRiakFolderObj(const char* f_name, const char* f_key, const char* p_key)
	{
		const char* type = "text/plain";
		riack_client* client = GetConnection();
		riack_object* robj = riack_object_alloc(m_riak);

		// file property
		radi_riack_set_string(robj->bucket, m_fs_name.c_str());
		radi_riack_set_string(robj->key, f_key);

		robj->content_count = 1;
		robj->content = (riack_content*)malloc(sizeof(riack_content));
		memset(robj->content, 0, sizeof(riack_content));
		// content type
		radi_riack_set_string(robj->content[0].content_type, type);		
		// file name
		robj->content[0].data = (uint8_t*)strdup(f_name);
		robj->content[0].data_len = strlen(f_name);

		// user meta
		riack_pair* r_meta = NULL;
		robj->content->usermeta_count = 6;
		r_meta = (riack_pair *)malloc(sizeof(riack_pair)*robj->content->usermeta_count);
		memset(r_meta, 0, sizeof(riack_pair)*robj->content->usermeta_count);
		robj->content->usermetas = r_meta;

		// meta isfolder
		radi_raick_set_pair(*r_meta,"IS_FOLDER","true");
		r_meta++;

		// meta storage type
		radi_raick_set_pair(*r_meta,"DATA_STORAGE_TYPE", "VALUE");
		r_meta++;

		// meta FILE_NAME
		radi_raick_set_pair(*r_meta,"FILE_NAME", f_name);
		r_meta++;

		// meta DESCRIBE
		radi_raick_set_pair(*r_meta,"DESCRIBE", f_name);
		r_meta++;

		// meta CREATE_TIME
		//get_current_time_millis
		char s_time[RADI_PATH_MAX];
		long long lt = get_current_time_millis();
		sprintf(s_time, "%lld", lt);
		//radi_raick_set_pair(*r_meta,"CREATE_TIME", "1427961943715");
		radi_raick_set_pair(*r_meta, "CREATE_TIME", s_time);
		r_meta++;

		// meta MODIFY_TIME
		//radi_raick_set_pair(*r_meta,"MODIFY_TIME", "1427961943715");
		radi_raick_set_pair(*r_meta, "MODIFY_TIME", s_time);
		r_meta++;

		int result = riack_put(client, robj, NULL, NULL);
		//int result = RIACK_SUCCESS;

		riack_free_object_p(m_riak, &robj);

		return (result == RIACK_SUCCESS);
	}

	bool RiakFS::CreateRiakFileObj(const char* f_name, const char* f_key, const char* p_key, const char* data_type/*="PGIS"*/)
	{
		const char* type = "text/plain";
		riack_client* client = GetConnection();
		riack_object* robj = riack_object_alloc(m_riak);

		// file property
		radi_riack_set_string(robj->bucket, m_fs_name.c_str());
		radi_riack_set_string(robj->key, f_key);

		robj->content_count = 1;
		robj->content = (riack_content*)malloc(sizeof(riack_content));
		memset(robj->content, 0, sizeof(riack_content));
		// content type
		radi_riack_set_string(robj->content[0].content_type, type);
		// file name
		robj->content[0].data = (uint8_t*)strdup(f_name);
		robj->content[0].data_len = strlen(f_name);

		// user meta
		riack_pair* r_meta = NULL;
		robj->content->usermeta_count = 8;
		r_meta = (riack_pair *)malloc(sizeof(riack_pair)*robj->content->usermeta_count);
		memset(r_meta, 0, sizeof(riack_pair)*robj->content->usermeta_count);
		robj->content->usermetas = r_meta;

		// meta IS_FOLDER
		radi_raick_set_pair(*r_meta, "IS_FOLDER", "false");
		r_meta++;

		// meta DATA_TYPE
		radi_raick_set_pair(*r_meta, "DATA_TYPE", data_type);
		r_meta++;

		// meta DATA_STORAGE_TYPE
		radi_raick_set_pair(*r_meta, "DATA_STORAGE_TYPE", "BUCKET");
		r_meta++;

		// meta FILE_NAME
		radi_raick_set_pair(*r_meta, "FILE_NAME", f_name);
		r_meta++;

		// meta CREATE_TIME
		radi_raick_set_pair(*r_meta, "CREATE_TIME", "1427961943715");
		r_meta++;

		// meta MODIFY_TIME
		radi_raick_set_pair(*r_meta, "MODIFY_TIME", "1427961943715");
		r_meta++;

		// meta STATUS
		radi_raick_set_pair(*r_meta, "STATUS", "COMPLETED");
		r_meta++;

		// meta MODIFY_TIME
		radi_raick_set_pair(*r_meta, "SIZE", "0");
		r_meta++;

		int result = riack_put(client, robj, NULL, NULL);

		riack_free_object_p(m_riak, &robj);

		return (result == RIACK_SUCCESS);
	}

	RiakTileStore* RiakFS::CreateTileStore(const char* name, TileStoreType type)
	{
		RiakTileStore* store = NULL;
		switch (type)
		{
		case radiTileStorePIGS:
			store = CreateTileStorePGS(name);
			break;
		default:
			break;
		}
		return store;
	}

	RiakTileStore* RiakFS::CreateTileStorePGS(const char* name)
	{
		RiakTileStore* store = NULL;
		if (HasBucket(name))
		{
			return NULL;
		}

		return store;
	}

	bool RiakFS::HasBucket(const char* name)
	{
		bool has = false;

		riack_string_list* bucket_list;
		riack_list_buckets(m_riak, &bucket_list);

		char temp[_MAX_PATH];
		for (int i = 0; i < bucket_list->string_count; i++)
		{
			riack_string& str = bucket_list->strings[i];
			if (!strncmp(name, str.value, str.len))
			{
				has = true;
				break;
			}
		}

		riack_free_string_list_p(m_riak, &bucket_list);

		return has;
	}

	RiakTileStore* RiakFS::GetTileStore(const char* name)
	{
		if (name == NULL)
		{
			return NULL;
		}

		if (!HasBucket(name))
		{
			return NULL;
		}

		RiakTileStore* store = new RiakTileStore("", name, this);
		return store;
	}

	RiakTileStoreSet* RiakFS::GetTileStores()
	{
		RiakTileStoreSet* pset = new RiakTileStoreSet();
		
		riack_string_list* bucket_list;
		riack_list_buckets(m_riak, &bucket_list);


		char name[_MAX_PATH];
		for (int i = 0; i < bucket_list->string_count; i++)
		{
			riack_string& str = bucket_list->strings[i];
			memset(name, 0, _MAX_PATH);
			memcpy(name, str.value, str.len);
			if (strcmp(m_fs_name.c_str(), name))
			{
				RiakTileStore* store = new RiakTileStore("", name, this);
				pset->Add(store);
			}
		}

		riack_free_string_list_p(m_riak, &bucket_list);

		return pset;
	}

	bool RiakFS::HasFile(const char* parent_key, const char* name)
	{
		riack_get_object* robj = GetRiakObjects(m_fs_name.c_str(), parent_key);

		if (!robj->object.content_count)
		{
			riack_free_get_object_p(m_riak, &robj);
			return false;
		}
		
		char key[RADI_PATH_MAX];
		char fname[RADI_PATH_MAX]; 
		riack_content* r_content = &(robj->object.content[0]);
		riack_link* r_link = &(r_content->links[0]);
		for (int i = 0; i < r_content->link_count; i++, r_link++)
		{
			memset(key, 0, RADI_PATH_MAX);
			strncpy(key, r_link->key.value, r_link->key.len);
			if (GetFileNameByKey(fname, RADI_PATH_MAX, key))
			{
				if (!strcmp(fname, name))
				{
					riack_free_get_object_p(m_riak, &robj);
					return true;
				}
			}
			
		}

		riack_free_get_object_p(m_riak, &robj);

		return false;
	}

	bool RiakFS::GetFileNameByKey(char* fname, size_t fsize, const char* key)
	{
		riack_get_object* robj = GetRiakObjects(m_fs_name.c_str(), key);

		if (!robj->object.content_count)
		{
			riack_free_get_object_p(m_riak, &robj);
			return false;
		}

		riack_content* r_content = &(robj->object.content[0]);
		riack_pair* r_pair = &(r_content->usermetas[0]);
		for (int i = 0; i < r_content->usermeta_count; i++, r_pair++)
		{
			if (!strncmp("FILE_NAME", r_pair->key.value, r_pair->key.len))
			{
				memset(fname, 0, fsize);
				memcpy(fname, r_pair->value, r_pair->value_len);
				riack_free_get_object_p(m_riak, &robj);
				return true;
			}
		}

		//char key[RADI_PATH_MAX];
		//char fname[RADI_PATH_MAX];
		//riack_content* r_content = &(robj->object.content[0]);
		//riack_link* r_link = &(r_content->links[0]);
		//for (int i = 0; i < r_content->link_count; i++, r_link++)
		//{
		//	strncpy(key, r_link->key.value, r_link->key.len);
		//	GetFileNameByKey(fname, key);
		//}

		riack_free_get_object_p(m_riak, &robj);

		return false;
	}
}