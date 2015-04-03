#ifndef __RIAK_TILE_H__
#define __RIAK_TILE_H__

#include "riack.h"
#include "triack.h"
#include <string>


namespace radi
{
	class TRIACK_API RiakTile
	{
	public:
		RiakTile();
		virtual ~RiakTile();
	public:
		void	Save(const char* path);
		void	Release();

	public:
		bool	Create(unsigned char* data, size_t size);

	private:
		unsigned char*	m_data;
		size_t			m_size;
	};
}

#endif //__RIAK_TILE_H__
