#include "DataDirHelper.h"

//#include <filesystem> needs C++17, currently blocked on MXE due to old glibmm
#include <boost/filesystem.hpp>
#include <glibmm-2.4/glibmm/miscutils.h>

#include "config.h"

DataDirHelper::DataDirHelper()
{
    std::vector<std::string> systemDataDirs = Glib::get_system_data_dirs();

    // create paths based on glib enumerated paths
    for (std::string systemDataDir : systemDataDirs)
    {
        basePaths.push_back(systemDataDir + "/" PROJECT_NAME "/");
    }

    // add datadir relative to binary if present (share/project_name)
    if (boost::filesystem::exists(DATADIR))
    {
        basePaths.push_back(DATADIR);
    }

    // add datadir relative to parent folder of binary if present (../share/project_name)
    if (boost::filesystem::exists("../" DATADIR))
    {
        basePaths.push_back("../" DATADIR);
    }
}

DataDirHelper &DataDirHelper::getInstance()
{
    static DataDirHelper instance;
    return instance;
}

std::string DataDirHelper::locate(const std::string &fileName) const
{
    for (std::string basePath : basePaths)
    {
        std::string filePath = basePath + fileName;
        if (boost::filesystem::exists(filePath))
        {
            return filePath;
        }
    }

    return "";
}