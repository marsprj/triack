#include "RiakFS.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"
#include "stdlib.h"

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

	long get_current_time_millis()
	{
#ifdef WIN32
		return 0;
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}

	void griak_uuid_generate(char* uuid, size_t size)
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
					printf("%s\n", rf->GetName());
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

	bool RiakFS::CreateRiakFile(const char* parent_key, const char* f_name, bool is_folder, const char* data_type/*="PGIS"*/)
	{
		if (parent_key == NULL || f_name == NULL)
		{
			return false;
		}

		RiakFile* pf = GetRiakFile(m_fs_name.c_str(), parent_key);
		if (pf == NULL)
		{
			return false;
		}

		bool ret = false;
		char f_key[RADI_PATH_MAX];
		griak_uuid_generate(f_key, RADI_PATH_MAX);
		if (is_folder)
		{
			ret = CreateRiakFileObj(f_name, f_key, parent_key, data_type);
		}
		else
		{

		}
		
		if (!ret)
		{
			pf->Release();
			
			return NULL;
		}

		// add link
		pf->AddLink(f_key);

		pf->Release();
		return true;
	}

	bool RiakFS::CreateRiakFileObj(const char* f_name, const char* f_key, const char* p_key, const char* data_type/*="PGIS"*/)
	{
		const char* type = "text/plain";
		riack_client* client = GetConnection();
		riack_object* robj = riack_object_alloc(m_riak);
		
		// file property
		robj->bucket.value = (char*)m_fs_name.c_str();
		robj->bucket.len = m_fs_name.length();
		robj->key.value = (char*)f_key;
		robj->key.len = strlen(f_key);
		robj->content_count = 1;
		robj->content = (riack_content*)malloc(sizeof(riack_content));
		memset(robj->content, 0, sizeof(riack_content));
		// content type
		robj->content[0].content_type.value = (char*)type;
		robj->content[0].content_type.len = strlen(type);
		// file name
		robj->content[0].data = (unsigned char*)f_name;
		robj->content[0].data_len = strlen(f_name);

		// link
		riack_link *r_link = (riack_link *)malloc(sizeof(riack_link));
		memset(r_link, 0, sizeof(riack_link));
		robj->content->link_count = 1;
		robj->content->links = r_link;
		r_link->bucket.value = (char*)m_fs_name.c_str();
		r_link->bucket.len = m_fs_name.length();
		r_link->key.value = (char*)p_key;
		r_link->key.len = strlen(p_key);

		// user meta
		riack_pair* r_meta = NULL;
		robj->content->usermeta_count = 6;
		r_meta = (riack_pair *)malloc(sizeof(riack_pair)*robj->content->usermeta_count);
		memset(r_meta, 0, sizeof(riack_pair)*robj->content->usermeta_count);
		robj->content->usermetas = r_meta;

		// meta isfolder
		const char* meta_key = "IS_FOLDER";
		const char* meta_val = "true";
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		// meta storage type
		//if (data_type != NULL)
		//{
		//	//meta_key = "IS_FOLDER";
		//	//meta_val = "true" : "false";
		//	//r_meta->key.value = (char*)meta_key;
		//	//r_meta->key.len = strlen(meta_key);
		//	//r_meta->value = (unsigned char*)meta_val;
		//	//r_meta->value_len = strlen(meta_val);
		//	//r_meta++;
		//}

		// meta storage type
		meta_key = "DATA_STORAGE_TYPE";
		meta_val = "VALUE";
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		// meta FILE_NAME
		meta_key = "FILE_NAME";
		meta_val = f_name;
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		// meta DESCRIBE
		meta_key = "DESCRIBE";
		meta_val = f_name;
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		// meta CREATE_TIME
		meta_key = "CREATE_TIME";
		meta_val = f_name;
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		// meta MODIFY_TIME
		meta_key = "MODIFY_TIME";
		meta_val = f_name;
		r_meta->key.value = (char*)meta_key;
		r_meta->key.len = strlen(meta_key);
		r_meta->value = (unsigned char*)meta_val;
		r_meta->value_len = strlen(meta_val);
		r_meta++;

		int result = riack_put(client, robj, NULL, NULL);
		//int result = RIACK_SUCCESS;

		riack_free_object_p(m_riak, &robj);

		return (result == RIACK_SUCCESS);
	}
}