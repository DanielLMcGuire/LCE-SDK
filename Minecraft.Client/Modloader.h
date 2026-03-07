#pragma once
#include "Minecraft.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

class File;

class FileFilter;

typedef void (*ModEntry)(Minecraft *minecraft);
const std::string ModEntryName = "mcmain";

#if defined(_WIN32)
    using LibHandle = std::unique_ptr<void, std::function<void(HMODULE)>>;
#else
    using LibHandle = std::unique_ptr<void, std::function<void(void*)>>;
#endif

struct Plugin
{
    LibHandle handle;
    File      path;
    ModEntry  init;

    Plugin(LibHandle h, const File &f, ModEntry e)
        : handle(std::move(h)), path(f), init(e) {}

    Plugin(Plugin&&) = default;
    Plugin& operator=(Plugin&&) = default;

    Plugin(const Plugin&) = delete;
    Plugin& operator=(const Plugin&) = delete;
};

class Modloader
{
  public:
    explicit Modloader(Minecraft *minecraft);
    explicit Modloader(const File &pluginFolder, Minecraft *minecraft);

    ~Modloader() = default;

    bool addPlugin(const File &pluginFile);

  private:
    bool loadPlugin(const File &library);

    std::vector<File> scanPlugins(const File &folder);

    std::vector<Plugin> plugins_;
    Minecraft          *m_instance;
};