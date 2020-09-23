#ifndef DATADIRHELPER_H
#define DATADIRHELPER_H

#include <string>
#include <vector>

class DataDirHelper
{
public:
    static DataDirHelper &getInstance();

    /**
     * The application data can be located in different locations based on system.
     * This method makes a best attempt at finding the correct location by iterating through the possible directories.
     * @param fileName Name (or relative path) of the data file that's being searched.
     * @return The path to the data file if found, otherwise an empty string.
     */
    std::string locate(const std::string &fileName) const;

    // remove some functions for the singleton
    DataDirHelper(DataDirHelper const &) = delete;
    void operator=(DataDirHelper const &) = delete;

private:
    DataDirHelper();

    static DataDirHelper instance;

    std::vector<std::string> basePaths;
};

#endif