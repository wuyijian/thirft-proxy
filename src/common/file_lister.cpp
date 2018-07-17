#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "./file_lister.h"

FileLister::FileLister(const std::string& path)	: m_dirpath(path), m_dir(NULL)
{
}

FileLister::~FileLister()
{
	Close();
}

void FileLister::Close()
{
    if(m_dir)
    {
        closedir(m_dir);
        m_dir = NULL;
    }
}

void FileLister::Open(const std::string& path)
{
	m_dirpath = path;
    this->Open();
}

void FileLister::Open()
{
    Close();

	m_dir = opendir(m_dirpath.c_str());
}

void FileLister::Start()
{
    if(m_dir)
        rewinddir(m_dir);
}

bool FileLister::Next(string &filename)
{
    if(NULL == m_dir)
        return false;

	dirent* entry = readdir(m_dir);
	while (entry) 
    {
		if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN)
        {
            filename = entry->d_name;
			return true;
		}

		entry = readdir(m_dir);
	}

	return false;
}
