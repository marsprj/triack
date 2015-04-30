#include "RiakLogger.h"

#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>

#ifdef WIN32
#	include "windows.h"
#endif


using namespace log4cplus;
using namespace log4cplus::helpers;

namespace radi
{
	void radi_get_cwd(char* szpath, size_t length);
	bool radi_get_parent_dir(char* dir, char* pdir, int size);
	void radi_make_path(char* path, const char* drv, const char* dir, const char* file, const char* ext);
	void radi_split_path(const char *path, char *drive, char *dir, char *fname, char *ext);
	int	 radi_find_last_char(const char *str, char ch);

	RiakLogger* augeGetLoggerInstance()
	{
		static RiakLogger g_logger;
		return &g_logger;
	}
	
	RiakLogger::RiakLogger()
	{

	}

	RiakLogger::~RiakLogger()
	{

	}
	
	void RiakLogger::Debug(const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(DEBUG_LOG_LEVEL,info, filename, line);
	}

	void RiakLogger::Trace(const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(TRACE_LOG_LEVEL,info, filename, line);
	}

	void RiakLogger::Info (const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(INFO_LOG_LEVEL,info, filename, line);
	}

	void RiakLogger::Warn (const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(WARN_LOG_LEVEL,info, filename, line);
	}

	void RiakLogger::Error(const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(ERROR_LOG_LEVEL,info, filename, line);
	}

	void RiakLogger::Fatal(const char* info, const char* filename, int line)
	{
		if(!m_IsInitialized)
		{
			Initialize();
		}
		Logger::getRoot().log(FATAL_LOG_LEVEL,info, filename, line);
	}

	//////////////////////////////////////////////////////////////////////////

	bool RiakLogger::Initialize(const char* config_path/*=NULL*/)
	{
		char log_path[RADI_PATH_MAX] = {0};
	
		try
		{			
			char cdir[RADI_PATH_MAX] = {0};
			radi_get_cwd(cdir, RADI_PATH_MAX);

#ifdef WIN32
			radi_make_path(log_path, NULL, cdir, "conf\\glogger", "properties");
#else
			char pdir[RADI_PATH_MAX] = {0};
			radi_get_parent_dir(cdir, pdir, RADI_PATH_MAX);
			radi_make_path(log_path, NULL, pdir, "conf/glogger", "properties");
#endif

			PropertyConfigurator::doConfigure(log_path);
		}
		catch (...)
		{
			printf("注意：您的配置文件有问题，将使用默认配置!\n");
			std::string logfile = "auge.log";
			SharedAppenderPtr _append(new FileAppender(logfile,std::ios::app));
			_append->setName("file_aglog");
		
			std::string strPattern = "%D{%Y-%m-%d %H:%M:%S} - %p - %m [%l]%n";
			std::auto_ptr<Layout> _layout(new PatternLayout(strPattern));
		
			_append->setLayout(_layout);
			Logger::getRoot().addAppender(_append);
			Logger::getRoot().setLogLevel(INFO_LOG_LEVEL);
		}
	
		m_IsInitialized = true;
		return true;
	}

	bool RiakLogger::IsInitialized()
	{
		return m_IsInitialized;
	}

}

namespace radi
{
	void radi_get_cwd(char* szpath, size_t length)
	{
#ifdef WIN32
		char module_path[_MAX_PATH];
		GetModuleFileName(NULL, module_path, _MAX_PATH);
		char drv[_MAX_DRIVE];
		char dir[_MAX_DIR];
		_splitpath(module_path, drv, dir, NULL, NULL);
		_makepath(szpath, drv, dir, NULL, NULL);
#else
		getcwd(szpath, length);
#endif
	}

	void radi_make_path(char* path, const char* drv, const char* dir, const char* file, const char* ext)
	{
		_makepath(path, drv, dir, file, ext);
	}

	void radi_split_path(const char *path, char *drive, char *dir, char *fname, char *ext)
	{
		_splitpath(path, drive, dir, fname, ext);
	}

	bool radi_get_parent_dir(char* dir, char* pdir, int size)
	{
		if (dir == NULL)
		{
			return false;
		}

		int pos = radi_find_last_char(dir, '\\');
		if (pos > 0)
		{
			memset(pdir, 0, size);
			strncpy(pdir, dir, pos);
			return true;
		}
		else if (pos == 0)
		{
			return false;
		}
		else
		{
			pos = radi_find_last_char(dir, '/');
			if (pos > 0)
			{
				memset(pdir, 0, size);
				strncpy(pdir, dir, pos);
				return true;
			}
			else if (pos == 0)
			{
				return false;
			}
		}

		return false;
	}

	int radi_find_last_char(const char *str, char ch)
	{
		if (str == NULL)
			return -1;

		int pos = strlen(str) - 1;
		for (char* ptr = (char*)str + pos; pos >= 0; pos--, ptr--)
		{
			if ((*ptr) == ch)
			{
				break;
			}
		}
		return pos;
	}

}