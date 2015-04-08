#ifndef __TRIACK_H__
#define __TRIACK_H__

#ifdef WIN32
#	ifdef TRIACK_EXPORTS
#		define TRIACK_API __declspec(dllexport)
#	else
#		define TRIACK_API __declspec(dllimport)
#	endif
#else
#	define TRIACK_API
#endif

#ifdef WIN32
#	define radi_stricmp		_stricmp
#	define radi_strnicmp	_strnicmp
#	define radi_sprintf		sprintf
#	define radi_snprintf	_snprintf
#	define radi_strncpy		strncpy
#else
#	define radi_stricmp		strcasecmp
#	define radi_strnicmp	strncasecmp
#	define radi_sprintf		sprintf
#	define radi_snprintf	snprintf
#	define radi_strncpy		strncpy
#endif

#ifdef WIN32
typedef int64_t		g_int64;
#else
typedef long long	g_int64;
#endif

#define RADI_PATH_MAX		256

namespace radi
{
	extern "C"
	{
		TRIACK_API void radi_uuid_generate(char* uuid, size_t size);
	}
}
//==============================================================================
//------------------------------------------------------------------------------
// 连接所需动态库
//------------------------------------------------------------------------------
#ifdef WIN32
#	ifndef TRIACK_EXPORTS
#		define _TRIACK_LIB_NAME "triack"
#		if defined(_DEBUG)
#			if defined(_UNICODE)
//#				define _TRIACK_FILTER_CONFIGURATION "UD"
#				define _TRIACK_FILTER_CONFIGURATION "D"
#			else
#				define _TRIACK_FILTER_CONFIGURATION "D"
#			endif // _UNICODE
#		else
#			if defined(_UNICODE)
#				define _TRIACK_FILTER_CONFIGURATION "U"
#			else
#				define _TRIACK_FILTER_CONFIGURATION ""
#			endif // _UNICODE
#		endif // _DEBUG
#		pragma comment(lib, _TRIACK_LIB_NAME _TRIACK_FILTER_CONFIGURATION ".lib")
#		pragma message("Automatically linking with " _TRIACK_LIB_NAME _TRIACK_FILTER_CONFIGURATION ".dll")
#		undef _TRIACK_FILTER_CONFIGURATION
#		undef _TRIACK_LIB_NAME
#	endif // TRIACK_FILTER_EXPORTS 
#endif
//==============================================================================

#endif //__TRIACK_H__
