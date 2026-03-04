#pragma once
#include "Minecraft.h"
#include <functional>
#include <filesystem>
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <dlfcn.h>
#endif

typedef void (*ModEntry)(const Minecraft *minecraft); 
const std::string ModEntryName = "minecraft_mod";

struct Plugin
{
#if defined(_WIN32)
	HMODULE handle;
#else
	void *handle;
#endif
	std::string        path;
	ModEntry           init;
};

class Modloader
{
  public:
	explicit Modloader(const std::string &pluginFolder, Minecraft *minecraft);

	~Modloader();

	bool addPlugin(const std::string &pluginPath);

  private:
	bool loadPlugin(const std::string &library, Minecraft *minecraft);

	std::vector<std::string> scanPlugins(const std::string &folder);

	void unloadAll();

	std::vector<Plugin>   plugins_;
	std::string pluginFolder_;
	Minecraft *m_instance;
};
