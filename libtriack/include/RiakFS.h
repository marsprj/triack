#ifndef __TRIAK_FILE_SYSTEM_H__
#define __TRIAK_FILE_SYSTEM_H__

#include "riack.h"
#include "triack.h"
#include <string>

namespace radi
{
	class RiakFile;
	class RiakFileSet;

	class TRIACK_API RiakFS
	{
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

		bool			CreateRiakFile(const char* parent_key, const char* f_name, bool is_folder, const char* data_type = "PGIS");
		
		void			Release();

	public:
		riack_client*	GetConnection();
		bool			GetRiakFolderKey(const char* bucket, const char* parent_key, const char* file_name, char* file_key);

		riack_get_object*	GetRiakObjects(const char* bucket, const char* key);

	private:
		bool			CreateRiakFileObj(const char* f_name, const char* f_key, const char* p_key, const char* data_type = "PGIS");


	private:
		std::string		m_riak_server;
		int				m_riak_port;

		riack_client	*m_riak;
		riack_allocator	m_riak_allocator;

		std::string		m_fs_name;
		std::string		m_fs_root;

	};
}

#endif //__TRIAK_FILE_SYSTEM_H__