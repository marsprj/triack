#include "RiakTile.h"
#include <stdio.h>
#include <string.h>

namespace radi
{
	RiakTile::RiakTile():
	m_data(NULL),
	m_size(0)
	{

	}

	RiakTile::~RiakTile()
	{
		if (m_data != NULL)
		{
			delete[] m_data;
			m_data = NULL;
		}
	}

	void RiakTile::Release()
	{
		delete this;
	}

	bool RiakTile::Create(unsigned char* data, size_t size)
	{
		if (m_data != NULL)
		{
			delete[] m_data;
			m_data = NULL;
		}

		m_data = new unsigned char[size];
		memcpy(m_data, data, size);
		m_size = size;

		return true;
	}

	void RiakTile::Save(const char* path)
	{
		if (path == NULL)
		{
			return;
		}

		FILE* fp = fopen(path, "wb");

		fwrite(m_data, sizeof(unsigned char), m_size, fp);

		fclose(fp);

	}
}