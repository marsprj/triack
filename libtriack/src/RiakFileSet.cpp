#include "RiakFile.h"
#include "RiakFileSet.h"

namespace radi
{
	RiakFileSet::RiakFileSet()
	{

	}

	RiakFileSet::~RiakFileSet()
	{
		RiakFile* pf = NULL;
		std::vector<RiakFile*>::iterator iter;
		for (iter = m_files.begin(); iter != m_files.end(); iter++)
		{
			pf = *iter;
			pf->Release();
		}
		m_files.clear();
	}

	int RiakFileSet::GetCount()
	{
		return m_files.size();
	}

	RiakFile* RiakFileSet::GetRiakFile(int i)
	{
		if (i >= m_files.size())
		{
			return NULL;
		}
		return m_files[i];
	}

	void RiakFileSet::Add(RiakFile* pFile)
	{
		m_files.push_back(pFile);
	}

	void RiakFileSet::Release()
	{
		delete this;
	}
}