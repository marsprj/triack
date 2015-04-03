#ifndef __TRIAK_FILE_SET_H__
#define __TRIAK_FILE_SET_H__

#include "riack.h"
#include "triack.h"
#include <string>
#include <vector>

namespace radi
{
	class RiakFile;

	class TRIACK_API RiakFileSet
	{
	public:
		RiakFileSet();
		virtual ~RiakFileSet();
	public:
		int			GetCount();
		RiakFile*	GetRiakFile(int i);
		void		Add(RiakFile* pFile);
		void		Release();

	private:
		std::vector<RiakFile*> m_files;
	};
}

#endif //__TRIAK_FILE_SET_H__