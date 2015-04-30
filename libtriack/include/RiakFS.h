#ifndef __TRIAK_FILE_SYSTEM_H__
#define __TRIAK_FILE_SYSTEM_H__

#include "riack.h"
#include "triack.h"
#include "RiakLogger.h"
#include <string>

namespace radi
{
	class RiakFile;
	class RiakFileSet;
	class RiakTileStore;
	class RiakTileStorePGIS;
	class RiakTileStoreSet;

	typedef enum 
	{
		radiTileStorePIGS	= 0
	}TileStoreType;

	class TRIACK_API RiakFS
	{
		friend class RiakFile;
		friend class RiakTileStore;
		friend class RiakTileStorePGIS;
	public:
		RiakFS();
		RiakFS(const char* server, int port);
		virtual ~RiakFS();

	public:
		//void			SetServer(const char* server);
		//void			SetPort(int port);

		bool			Connect();
		void			Close();

		RiakFile*		GetRoot();
		RiakFile*		GetRiakFile(const char* bucket, const char* key);
		RiakFile*		GetRiakFileByName(const char* bucket, const char* name);
		RiakFileSet*	ListFiles(const char* dir_key);
		bool			HasFile(const char* parent_key, const char* name);

		bool			CreateRiakFolder(const char* parent_key, const char* f_name);
		bool			CreateRiakFile(const char* parent_key, const char* f_name, const char* type = "PGIS");

		RiakTileStore*	CreateTileStore(const char* name, TileStoreType type);
		RiakTileStore*	GetTileStore(const char* name);
		RiakTileStoreSet* GetTileStores();
		
		void			Release();

	public:
		riack_client*	GetConnection();
		bool			GetRiakFolderKey(const char* bucket, const char* parent_key, const char* file_name, char* file_key);
		bool			GetFileNameByKey(char* fname, size_t fsize, const char* key);

		riack_get_object*	GetRiakObjects(const char* bucket, const char* key);

	private:
		bool			CreateRiakFolderObj(const char* f_name, const char* f_key, const char* p_key);
		bool			CreateRiakFileObj(const char* f_name, const char* f_key, const char* p_key, const char* data_type = "PGIS");
		RiakTileStore*	CreateTileStorePGS(const char* name);
		bool			HasBucket(const char* name);


	private:
		std::string		m_riak_server;
		int				m_riak_port;

		riack_client	*m_riak;
		riack_allocator	m_riak_allocator;

		std::string		m_fs_name;
		std::string		m_fs_root;

		RiakLogger		m_logger;

	};

	extern "C"
	{
		void radi_raick_set_pair(riack_pair& rpair, const char* key, const char* data);
		void radi_riack_set_link(riack_link& r_link, const char* bucket, const char* key, const char* tag);
		void radi_riack_set_string(riack_string& rstring, const char* value);
	}
}

#endif //__TRIAK_FILE_SYSTEM_H__