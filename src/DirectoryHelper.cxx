#include "DirectoryHelper.h"

#include <iostream>
//#include <filesystem> needs C++17, currently blocked on MXE due to old glibmm
#include <boost/filesystem.hpp>
#include <glibmm-2.4/glibmm/miscutils.h>

#include "config.h"

DirectoryHelper::DirectoryHelper()
{
    // add data dirs based on glib enumerated paths
    std::vector<std::string> systemDataDirs = Glib::get_system_data_dirs();
    for (const std::string &systemDataDir : systemDataDirs)
    {
        dataPaths.push_back(systemDataDir + "/" PROJECT_NAME "/");
    }

    std::string userDataDir = Glib::get_user_data_dir();
    if (!userDataDir.empty())
    {
        dataPaths.push_back(userDataDir + "/" PROJECT_NAME "/");
    }

    // add data dir relative to binary if present (share/project_name)
    if (boost::filesystem::exists(DATADIR))
    {
        dataPaths.push_back(DATADIR);
    }

    // add data dir relative to parent folder of binary if present (../share/project_name)
    if (boost::filesystem::exists("../" DATADIR))
    {
        dataPaths.push_back("../" DATADIR);
    }

    // add config dirs based on glib enumerated paths (prefer user dir here)
    std::string userConfigDir = Glib::get_user_config_dir();
    if (!userConfigDir.empty())
    {
        configPaths.push_back(userConfigDir + "/" PROJECT_NAME "/");
    }

    std::vector<std::string> systemConfigDirs = Glib::get_system_config_dirs();
    for (const std::string &systemConfigDir : systemConfigDirs)
    {
        configPaths.push_back(systemConfigDir + "/" PROJECT_NAME "/");
    }
}

DirectoryHelper &DirectoryHelper::getInstance()
{
    static DirectoryHelper instance;
    return instance;
}

std::string DirectoryHelper::locateData(const std::string &fileName) const
{
    for (const std::string &dataPath : dataPaths)
    {
        std::string filePath = dataPath + fileName;
        if (boost::filesystem::exists(filePath))
        {
            return filePath;
        }
    }

    return "";
}

std::string DirectoryHelper::locateConfig(
    const std::string &fileName, bool suggestIfNotFound) const
{
    for (const std::string &configPath : configPaths)
    {
        std::string filePath = configPath + fileName;
        if (boost::filesystem::exists(filePath))
        {
            return filePath;
        }
    }

    if (suggestIfNotFound)
    {
        // suggest the first path in the vector
        // create directories if they don't exist already
        if (!boost::filesystem::exists(configPaths[0]))
        {
            try
            {
                boost::filesystem::create_directories(configPaths[0]);
            }
            catch (boost::filesystem::filesystem_error &e)
            {
                std::cerr << "Failed creating directory " << e.path1()
                          << ", no suggestion will be returned\n"
                          << "\terror code: " << e.code().value()
                          << " '" << e.code().message() << "'" << std::endl;
                return "";
            }
        }

        return configPaths[0] + fileName;
    }

    return "";
}