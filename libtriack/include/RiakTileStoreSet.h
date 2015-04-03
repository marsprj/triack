#ifndef __TRIAK_TILE_STORE_SET_H__
#define __TRIAK_TILE_STORE_SET_H__

#include "riack.h"
#include "triack.h"
#include <string>
#include <vector>

namespace radi
{
	class RiakTileStore;

	class TRIACK_API RiakTileStoreSet
	{
	public:
		RiakTileStoreSet();
		virtual ~RiakTileStoreSet();
	public:
		RiakTileStore*	Next();
		void			Reset();
		void			Release();

	public:
		void			Add(RiakTileStore* store);

	private:
		void			Cleanup();

	private:
		std::vector<RiakTileStore*> m_stores;
		std::vector<RiakTileStore*>::iterator m_iter;
	};
}

#endif //__TRIAK_TILE_STORE_SET_H__