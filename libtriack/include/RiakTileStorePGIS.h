#ifndef __TRIAK_FILE_STORE_PGIS_H__
#define __TRIAK_FILE_STORE_PGIS_H__

#include "RiakTileStore.h"

namespace radi
{
	class RiakTileStorePGIS : public RiakTileStore
	{
	public:
		RiakTileStorePGIS();
		RiakTileStorePGIS(const char* name, const char* key, RiakFS* riak_fs);
		virtual ~RiakTileStorePGIS();
	public:
		const char*	GetKey();
		const char*	GetName();
		const char*	GetStatus();

		void		GetInfo(TileStoreInfo& info);
		RRect		GetExtent();
		bool		SetExtent(RRect rect);

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

		bool		PutStoreMeta();
		bool		PutStoreMetaPGIS();
		bool		PutStoreMetaPGIS(int start_level, int end_level, double xmin, double ymin, double xmax, double ymax);

		void		Release();

		void		GetTiles();

	};
}

#endif //__TRIAK_FILE_STORE_PGIS_H__
