#include "stdafx.h"
#include "Modloader.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#if defined(_WIN32)
#include <direct.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#if defined(_WIN32)
#define getcwd _getcwd
#endif

static std::string joinPath(const std::string &base, const std::string &sub)
{
#if defined(_WIN32)
    return base + "\\" + sub;
#else
    return base + "/" + sub;
#endif
}

bool Modloader::loadPlugin(const std::string &library, Minecraft *minecraft)
{
    using PluginEntryFunc = void (*)(const PhasorAPI *, PhasorVM *);

#if defined(_WIN32)
    HMODULE lib = LoadLibraryA(library.c_str());
    if (!lib)
    {
        return false;
    }
    auto entry_point = (PluginEntryFunc)GetProcAddress(lib, "minecraft_mod");
#else
    void *lib = dlopen(library.c_str(), RTLD_NOW);
    if (!lib)
    {
        return false;
    }
    auto entry_point = (PluginEntryFunc)dlsym(lib, "minecraft_mod");
#endif

    if (!entry_point)
    {
#if defined(_WIN32)
        FreeLibrary(lib);
#else
        dlclose(lib);
#endif
        return false;
    }

    plugins_.push_back(Plugin{lib, library, nullptr});
    return true;
}

bool Modloader::addPlugin(const std::string &pluginPath)
{
    return loadPlugin(pluginPath, m_instance);
}

std::vector<std::string> Modloader::scanPlugins(const std::string &folder)
{
    if (folder.empty())
        return {};

    std::vector<std::string> plugins;
    std::string              exeDir;
    std::vector<std::string> foldersToScan;

#if defined(_WIN32)
    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    exeDir = std::string(path);
    size_t pos = exeDir.find_last_of("\\/");
    if (pos != std::string::npos)
        exeDir = exeDir.substr(0, pos);
#elif defined(__APPLE__)
    char     path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
    {
        std::string p(path);
        size_t pos = p.find_last_of('/');
        exeDir = (pos != std::string::npos) ? p.substr(0, pos) : p;
    }
    else
    {
        char cwdBuf[4096];
        if (getcwd(cwdBuf, sizeof(cwdBuf)))
            exeDir = cwdBuf;
    }
#elif defined(__linux__)
    char    path[1024];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
    if (count != -1)
    {
        std::string p(path, count);
        size_t pos = p.find_last_of('/');
        exeDir = (pos != std::string::npos) ? p.substr(0, pos) : p;
    }
    else
    {
        char cwdBuf[4096];
        if (getcwd(cwdBuf, sizeof(cwdBuf)))
            exeDir = cwdBuf;
    }
#else
    char cwdBuf[4096];
    if (getcwd(cwdBuf, sizeof(cwdBuf)))
        exeDir = cwdBuf;
#endif

    std::string cwdStr;
    {
        char cwdBuf[4096];
        if (getcwd(cwdBuf, sizeof(cwdBuf)))
            cwdStr = cwdBuf;
    }

    foldersToScan.push_back(joinPath(exeDir, folder));
    if (exeDir != cwdStr)
        foldersToScan.push_back(joinPath(cwdStr, folder));

    for (auto &folderPath : foldersToScan)
    {
#if defined(_WIN32)
        WIN32_FIND_DATAA fd;
        HANDLE hFind = FindFirstFileA((folderPath + "\\*").c_str(), &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            continue;
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;
            std::string name(fd.cFileName);
            std::string ext;
            size_t dot = name.find_last_of('.');
            if (dot != std::string::npos)
                ext = name.substr(dot);
            if (ext == ".dll" || ext == ".asi")
                plugins.push_back(folderPath + "\\" + name);
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
#else
        DIR *dir = opendir(folderPath.c_str());
        if (!dir)
            continue;
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            if (entry->d_type != DT_REG)
                continue;
            std::string name(entry->d_name);
            std::string ext;
            size_t dot = name.find_last_of('.');
            if (dot != std::string::npos)
                ext = name.substr(dot);
#if defined(__APPLE__)
            if (ext == ".dylib")
#else
            if (ext == ".so")
#endif
                plugins.push_back(folderPath + "/" + name);
        }
        closedir(dir);
#endif
    }

    return plugins;
}

void Modloader::unloadAll()
{
    for (auto &plugin : plugins_)
    {
#if defined(_WIN32)
        FreeLibrary(plugin.handle);
#else
        dlclose(plugin.handle);
#endif
    }
    plugins_.clear();
}

Modloader::Modloader(const std::string &pluginFolder, Minecraft *minecraft) : pluginFolder_(pluginFolder), m_instance(minecraft)
{
    auto plugins = scanPlugins(pluginFolder_);
    for (const auto &pluginPath : plugins)
    {
        loadPlugin(pluginPath, minecraft);
    }
}

Modloader::~Modloader()
{
    unloadAll();
}