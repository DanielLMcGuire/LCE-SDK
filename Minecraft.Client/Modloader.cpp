#include "stdafx.h"
#include "Modloader.h"
#include <memory>
#include <vector>
#include <string>
#include <iostream>

namespace {

class PluginFilter : public FileFilter
{
  public:
    bool accept(File *f) const override
    {
        if (!f || !f->isFile()) return false;
        const std::string name = f->getName();
        const size_t dot = name.find_last_of('.');
        if (dot == std::string::npos) return false;
        const std::string ext = name.substr(dot);
#if defined(_WIN32)
        return ext == ".dll" || ext == ".asi";
#elif defined(__APPLE__)
        return ext == ".dylib";
#else
        return ext == ".so";
#endif
    }
};

LibHandle makeHandle(void *raw)
{
#if defined(_WIN32)
    return LibHandle(
        static_cast<HMODULE>(raw),
        [](HMODULE h) { if (h) FreeLibrary(h); }
    );
#else
    return LibHandle(
        raw,
        [](void *h) { if (h) dlclose(h); }
    );
#endif
}

}  // namespace

bool Modloader::loadPlugin(const File &library)
{
    const std::string path = library.getPath();

#if defined(_WIN32)
    void *raw = static_cast<void *>(LoadLibraryA(path.c_str()));
#else
    void *raw = dlopen(path.c_str(), RTLD_NOW);
#endif

    if (!raw) return false;

    LibHandle handle = makeHandle(raw);

#if defined(_WIN32)
    ModEntry entry_point = reinterpret_cast<ModEntry>(
        GetProcAddress(static_cast<HMODULE>(handle.get()), ModEntryName.c_str()));
#else
    ModEntry entry_point = reinterpret_cast<ModEntry>(
        dlsym(handle.get(), ModEntryName.c_str()));
#endif

    if (!entry_point) return false;

    entry_point(m_instance);
    plugins_.emplace_back(std::move(handle), library, entry_point);
    return true;
}

bool Modloader::addPlugin(const File &pluginFile)
{
    return loadPlugin(pluginFile);
}

std::vector<File> Modloader::scanPlugins(const File &baseFolder)
{
    std::vector<File> result;
    if (!baseFolder.isDirectory()) return result;

    PluginFilter filter;

    std::unique_ptr<std::vector<std::unique_ptr<File>>> entries(
        baseFolder.listFiles(&filter)
    );

    if (!entries) return result;

    for (const auto &f : *entries) {
        if (f) result.push_back(*f);
    }

    return result;
}

Modloader::Modloader(Minecraft *minecraft) : m_instance(minecraft)
{
    for (const File &f : scanPlugins(m_instance->workingDirectory))
        loadPlugin(f);
}

Modloader::Modloader(const File &pluginFolder, Minecraft *minecraft)
    : m_instance(minecraft)
{
    for (const File &f : scanPlugins(pluginFolder))
        loadPlugin(f);
}