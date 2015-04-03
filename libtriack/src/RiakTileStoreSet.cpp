#include "RiakTileStore.h"
#include "RiakTileStoreSet.h"

namespace radi
{
	RiakTileStoreSet::RiakTileStoreSet()
	{
		m_iter = m_stores.begin();
	}

	RiakTileStoreSet::~RiakTileStoreSet()
	{
		Cleanup();
	}

	RiakTileStore* RiakTileStoreSet::Next()
	{
		if (m_iter == m_stores.end())
		{
			return NULL;
		}
		return *(m_iter++);
	}

	void RiakTileStoreSet::Reset()
	{
		m_iter = m_stores.begin();
	}

	void RiakTileStoreSet::Release()
	{
		delete this;
	}

	void RiakTileStoreSet::Add(RiakTileStore* store)
	{
		if (store)
		{
			m_stores.push_back(store);
		}
	}

	void RiakTileStoreSet::Cleanup()
	{
		std::vector<RiakTileStore*>::iterator iter;
		for (iter = m_stores.begin(); iter != m_stores.end(); iter++)
		{
			(*iter)->Release();
		}
		m_stores.clear();
	}
}