#ifndef __TRIAK_FILE_H__
#define __TRIAK_FILE_H__

#include "riack.h"
#include "triack.h"
#include <string>

#define RIAK_FILE_STATUS_COMPLETED	"COMPLETED"
#define RIAK_FILE_STATUS_DELETED	"DELETED"

namespace radi
{
	class RiakFS;
	class RiakFileSet;
	class RiakTileStore;

	class TRIACK_API RiakFile
	{
	public:
		RiakFile();
		virtual ~RiakFile();

	public:
		bool			IsFolder();
		bool			IsRoot();

		const char*		GetName();
		void			SetName(const char* name);
		const char*		GetKey();

		const char*		GetStatus();
		void			SetStatus(const char* status);
		bool			IsDeleted();

		const char*		GetDataType() const;
		const char*		GetDataStore() const;

		RiakTileStore*	GetTileStore();

		RiakFileSet*	GetFiles();
		RiakFile*		GetRiakFile(const char* name);

		RiakFile*		CreateFolder(const char* name);
		RiakFile*		CreateRiakFile(const char* name, const char* type="PGIS");
		RiakFile*		CreateRiakFile(const char* name, int start_level, int end_level, const char* type = "PGIS");

		bool			AddLink(const char* link_key);

		void			Release();

	public:
		bool			Create(RiakFS* rfs, const char* key, riack_content* robj);

	private:
		void			SetIsFolder(const char* val);
		void			SetDataType(const char* val, size_t len);
		void			SetDataStore(const char* val, size_t len);

	private:
		bool		m_isFolder;
		std::string	m_name;
		std::string	m_key;
		std::string	m_data_type;
		std::string	m_data_store;
		std::string	m_status;

		RiakFS		*m_riak_fs;
		RiakTileStore	*m_tile_store;

		//riak_connection *m_cxn;
		//riak_config 	*m_cfg;
	};
}

#endif //__TRIAK_FILE_H__
