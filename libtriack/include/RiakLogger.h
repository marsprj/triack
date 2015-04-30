#ifndef __RADI_LOGGER_IMPL_H__
#define __RADI_LOGGER_IMPL_H__

#include "triack.h"

namespace radi
{
	class TRIACK_API RiakLogger
	{
	public:
		RiakLogger();
		~RiakLogger();
	public:
		void	Trace(const char* info, const char* fname=NULL, int line=0);
		void	Debug(const char* info, const char* fname=NULL, int line=0);
		void	Info (const char* info, const char* fname=NULL, int line=0);
		void	Warn (const char* info, const char* fname=NULL, int line=0);
		void	Error(const char* info, const char* fname=NULL, int line=0);
		void	Fatal(const char* info, const char* fname=NULL, int line=0);

		bool	Initialize(const char* config_path=NULL);
		bool	IsInitialized();
	private:
		bool	m_IsInitialized;
	};
}

#endif //__RADI_LOGGER_IMPL_H__
