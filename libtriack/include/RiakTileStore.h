#ifndef __TRIAK_FILE_STORE_H__
#define __TRIAK_FILE_STORE_H__

#include "riack.h"
#include "triack.h"
#include <string>

namespace radi
{
	class RiakFS;
	class RiakTile;

	typedef struct
	{
		int		count;
		g_int64	volume;
	}TileStoreInfo;

	class TRIACK_API RiakTileStore
	{
	public:
		RiakTileStore();
		RiakTileStore(const char* name, const char* key, RiakFS* riak_fs);
		virtual ~RiakTileStore();
	public:
		const char*	GetKey();
		const char*	GetName();
		const char*	GetStatus();

		void		GetInfo(TileStoreInfo& info);

		RiakTile*	GetTile(const char* t_key);
		bool		PutTile(const char* t_key, const unsigned char* t_data, size_t size, const char* content_type);
		bool		PutTile(const char* t_key, const char* t_path);
		bool		PutTile(int level, int row, int col, const char* t_path);
		bool		PutTile(int level, int row, int col, const unsigned char* t_data, size_t size, const char* content_type);

		bool		DeleteTile(const char* t_key);

		g_int64		GetVolume();
		g_int64		GetVolume(const char* key);
		bool		UpdateVolume();

		const char* GetGeoMeta();
		const char* GetConfXML();
		const char* GetConfCDI();

		bool		PutGeoMeta(const char* data);
		bool		PutGeoMeta(int start_level, int end_level);
		bool		PutConfXML(const char* data);
		bool		PutConfCDIPGIS(double xmin, double ymin, double xmax, double ymax);
		bool		PutConfCDI(const char* data);
		bool		PutPutConfXMLPGIS(int start_level, int end_level);

		bool		UpdateConfCDI(double xmin, double ymin, double xmax, double ymax);

		bool		PutStoreMetaPGIS();
		bool		PutStoreMetaPGIS(int start_level, int end_level, double xmin, double ymin, double xmax, double ymax);
		
		void		Release();

		void		GetTiles();

	private:
		std::string	m_name;
		std::string	m_key;

		std::string m_geo_meta;
		std::string m_conf_xml;
		std::string m_conf_cdi;

		RiakFS 		*m_riak_fs;
	};
}

#endif //__TRIAK_FILE_STORE_H__
