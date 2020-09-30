#ifndef DATADIRHELPER_H
#define DATADIRHELPER_H

#include <string>
#include <vector>

class DirectoryHelper
{
public:
    static DirectoryHelper &getInstance();

    /**
     * The application data can be located in different locations based on system.
     * This method makes a best attempt at finding the correct location by iterating through the possible directories.
     * @param fileName Name (or relative path) of the data file that's being searched.
     * @return The path to the data file if found, otherwise an empty string.
     */
    std::string locateData(const std::string &fileName) const;

    /**
     * The application config files can be located in different locations based on system.
     * This method makes a best attempt at finding the correct location by iterating through the possible directories.
     * @param fileName Name (or relative path) of the configuration file that's being searched.
     * @param suggestIfNotFound Whether to return a suggested path for the file if it doesn't exist yet.
     * @return The path to the configuration file if found, the suggested path for the file if not found
     * and suggestIfNotFound is true, otherwise an empty string.
     */
    std::string locateConfig(const std::string &fileName, bool suggestIfNotFound = false) const;

    // remove some functions for the singleton
    DirectoryHelper(DirectoryHelper const &) = delete;
    void operator=(DirectoryHelper const &) = delete;

private:
    DirectoryHelper();

    static DirectoryHelper instance;

    std::vector<std::string> dataPaths;
    std::vector<std::string> configPaths;
};

#endif