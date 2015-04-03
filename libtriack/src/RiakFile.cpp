#include "RiakFS.h"
#include "RiakFile.h"
#include "RiakFileSet.h"
#include "RiakTileStore.h"

namespace radi
{
	RiakFile::RiakFile() :
		m_isFolder(false),
		m_riak_fs(NULL),
		//m_cxn(NULL),
		//m_cfg(NULL),
		m_tile_store(NULL),
		m_status(RIAK_FILE_STATUS_COMPLETED)
	{

	}

	RiakFile::~RiakFile()
	{
		if (m_tile_store != NULL)
		{
			m_tile_store->Release();
			m_tile_store = NULL;
		}
	}

	void RiakFile::Release()
	{
		delete this;
	}


	bool RiakFile::IsFolder()
	{
		return m_isFolder;
	}

	bool RiakFile::IsRoot()
	{
		return !(strcmp(m_key.c_str(), "root"));
	}

	void RiakFile::SetIsFolder(const char* val)
	{
		m_isFolder = (val[0] == 't');
	}

	const char* RiakFile::GetName()
	{
		return m_name.c_str();
	}

	void RiakFile::SetName(const char* name)
	{
		if (name)
		{
			m_name = name;
		}
	}

	const char* RiakFile::GetKey()
	{
		return m_key.c_str();
	}

	const char* RiakFile::GetStatus()
	{
		return m_status.c_str();
	}

	void RiakFile::SetStatus(const char* status)
	{
		m_status = status;
	}

	bool RiakFile::IsDeleted()
	{
		return !(strcmp(m_status.c_str(), RIAK_FILE_STATUS_DELETED));
	}

	void RiakFile::SetDataType(const char* val, size_t len)
	{
		m_data_type.assign(val, len);
	}

	const char* RiakFile::GetDataType() const
	{
		return m_data_type.c_str();
	}

	void RiakFile::SetDataStore(const char* val, size_t len)
	{
		m_data_store.assign(val, len);
	}

	const char* RiakFile::GetDataStore() const
	{
		return m_data_store.c_str();
	}

	bool RiakFile::Create(RiakFS* rfs, const char* key, riack_content* robj)
	{
		m_riak_fs = rfs;
		m_key = key;

		char* name = (char*)robj->data;
		size_t size = robj->data_len;

		m_name = std::string(name, size);

		int n_meta = robj->usermeta_count;
		for (int i = 0; i < n_meta; i++)
		{
			riack_pair* meta = &(robj->usermetas[i]);
			if (strncmp(meta->key.value, "IS_FOLDER", meta->key.len) == 0)
			{
				SetIsFolder((char*)meta->value);
			}
			else if (strncmp(meta->key.value, "DATA_TYPE", meta->key.len) == 0)
			{
				SetDataType((char*)meta->value, meta->value_len);
			}
			else if (strncmp(meta->key.value, "DATA_STORAGE_TYPE", meta->key.len) == 0)
			{
			}
			else if (strncmp(meta->key.value, "CREATE_TIME", meta->key.len) == 0)
			{
			}
		}
		return true;
	}

	RiakTileStore* RiakFile::GetTileStore()
	{
		if (m_isFolder)
		{
			return NULL;
		}
		if (m_tile_store == NULL)
		{
			m_tile_store = new RiakTileStore(m_name.c_str(), m_key.c_str(), m_riak_fs);
		}
		return m_tile_store;
	}

	RiakFileSet* RiakFile::GetFiles()
	{
		return m_riak_fs->ListFiles(m_key.c_str());
	}

	RiakFile* RiakFile::GetRiakFile(const char* name)
	{
		return m_riak_fs->GetRiakFileByName(m_key.c_str(), name);
	}

	RiakFile* RiakFile::CreateRiakFolder(const char* name)
	{
		if (name == NULL)
		{
			return NULL;
		}

		m_riak_fs->CreateRiakFile(m_key.c_str(), name, true);

		return NULL;
	}

	bool RiakFile::AddLink(const char* key)
	{
		if (key == NULL)
		{
			return false;
		}

		riack_client* client = m_riak_fs->GetConnection();
		riack_get_object* robj = NULL;

		robj = m_riak_fs->GetRiakObjects("rfs", m_key.c_str());
		if (!robj->object.content_count)
		{
			riack_free_get_object_p(client, &robj);
			return false;
		}


		riack_free_get_object_p(client, &robj);

		return true;
	}
}
