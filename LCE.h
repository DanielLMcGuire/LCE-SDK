#pragma once
class Timer;
class MultiPlayerLevel;
class LevelRenderer;
class MultiplayerLocalPlayer;
class Player;
class Mob;
class ParticleEngine;
class User;
class Canvas;
class Textures;
class Font;
class Screen;
class ProgressRenderer;
class GameRenderer;
class BackgroundDownloader;
class HumanoidModel;
class HitResult;
class Options;
class SoundEngine;
class MinecraftApplet;
class MouseHandler;
class TexturePackRepository;
class File;
class LevelStorageSource;
class StatsCounter;
class Component;
class Entity;
class AchievementPopup;
class WaterTexture;
class LavaTexture;
class Gui;
class ClientConnection;
class ConsoleSaveFile;
class ItemInHandRenderer;
class LevelSettings;
class ColourTable;
class MultiPlayerGameMode;


// Abstract Definitions to replace first
class Level;
class MultiPlayerLevelArray;

const int XUSER_MAX_COUNT = 4;

#include <vector>
#include <memory>

class Minecraft
{
public:
	static const std::wstring VERSION_STRING;
	Minecraft(Component *mouseComponent, Canvas *parent, MinecraftApplet *minecraftApplet, int width, int height, bool fullscreen);
	void init();

public:
	MultiPlayerGameMode *gameMode;

public:
	int width, height;
	int width_phys, height_phys;

public:
	Level *oldLevel;
public:

	MultiPlayerLevel *level;
	LevelRenderer *levelRenderer;
	std::shared_ptr<MultiplayerLocalPlayer> player;

	// MultiPlayerLevelArray levels;

	MultiPlayerGameMode *localgameModes[XUSER_MAX_COUNT];
	int localPlayerIdx;
	ItemInHandRenderer *localitemInHandRenderers[XUSER_MAX_COUNT];
	unsigned int	uiDebugOptionsA[XUSER_MAX_COUNT];

	bool m_connectionFailed[XUSER_MAX_COUNT];
	ClientConnection *m_pendingLocalConnections[XUSER_MAX_COUNT];

	bool addLocalPlayer(int idx);
	void addPendingLocalConnection(int idx, ClientConnection *connection);
	// void connectionDisconnected(int idx, DisconnectPacket::eDisconnectReason reason) { m_connectionFailed[idx] = true; m_connectionFailedReason[idx] = reason; }

	std::shared_ptr<MultiplayerLocalPlayer> createExtraLocalPlayer(int idx, const std::wstring& name, int pad, int iDimension, ClientConnection *clientConnection = NULL,MultiPlayerLevel *levelpassedin=NULL);
	void createPrimaryLocalPlayer(int iPad);
	bool setLocalPlayerIdx(int idx);
	int getLocalPlayerIdx();
	void removeLocalPlayerIdx(int idx);
	void storeExtraLocalPlayer(int idx);
	void updatePlayerViewportAssignments();
	int unoccupiedQuadrant;

	std::shared_ptr<Mob> cameraTargetPlayer;
	ParticleEngine *particleEngine;
	User *user;
	std::wstring serverDomain;
	Canvas *parent;
	bool appletMode;

	volatile bool pause;

	Textures *textures;
	Font *font, *altFont;
	Screen *screen;
	ProgressRenderer *progressRenderer;
	GameRenderer *gameRenderer;

public:
	AchievementPopup *achievementPopup;
public:
	Gui *gui;

	bool noRender;

	HumanoidModel *humanoidModel;
	HitResult *hitResult;
	Options *options;
public:
	SoundEngine *soundEngine;
	MouseHandler *mouseHandler;
public:
	TexturePackRepository *skins;
	// File workingDirectory;
public:
	static const int frameTimes_length = 512;
	static __int64 frameTimes[frameTimes_length];
	static const int tickTimes_length = 512;
	static __int64 tickTimes[tickTimes_length];
	static int frameTimePos;
	static __int64 warezTime;
public:
	StatsCounter* stats[4];

public:
	void clearConnectionFailed();
	void connectTo(const std::wstring& server, int port);

public:
	void blit(int x, int y, int sx, int sy, int w, int h);

public:
	static File getWorkingDirectory();
	static File getWorkingDirectory(const std::wstring& applicationName);
public:
	LevelStorageSource *getLevelSource();
	void setScreen(Screen *screen);

public:
	void destroy();
	volatile bool running;
	std::wstring fpsString;
	void run();
	static Minecraft *GetInstance();
	void run_middle();
	void run_end();
	void emergencySave();

public:
	void stop();

	void pauseGame();

public:
	void tick(bool bFirst, bool bUpdateTextures);
public:
	bool isClientSide();
	void selectLevel(ConsoleSaveFile *saveFile, const std::wstring& levelId, const std::wstring& levelName, LevelSettings *levelSettings);
	bool saveSlot(int slot, const std::wstring& name);
	bool loadSlot(const std::wstring& userName, int slot);
	void releaseLevel(int message);
	void setLevel(MultiPlayerLevel *level, int message = -1, std::shared_ptr<Player> forceInsertPlayer = nullptr, bool doForceStatsSave = true,bool bPrimaryPlayerSignedOut=false);
	void forceaddLevel(MultiPlayerLevel *level);
	void prepareLevel(int title);
	void fileDownloaded(const std::wstring& name, File *file);

	std::wstring gatherStats1();
	std::wstring gatherStats2();
	std::wstring gatherStats3();
	std::wstring gatherStats4();

	void respawnPlayer(int iPad,int dimension,int newEntityId);
	static void start(const std::wstring& name, const std::wstring& sid);
	static void startAndConnectTo(const std::wstring& name, const std::wstring& sid, const std::wstring& url);
	ClientConnection *getConnection(int iPad);
	static void main();
	static bool renderNames();
	static bool useFancyGraphics();
	static bool useAmbientOcclusion();
	static bool renderDebug();
	bool handleClientSideCommand(const std::wstring& chatMessage);

	static int maxSupportedTextureSize();
	void delayTextureReload();
	static __int64 currentTimeMillis();

	static int InGame_SignInReturned(void *pParam,bool bContinue, int iPad);
	Screen * getScreen();

	void forceStatsSave(int idx);

	// ???
	// CRITICAL_SECTION	m_setLevelCS;

public:
	bool isTutorial();
	void playerStartedTutorial(int iPad);
	void playerLeftTutorial(int iPad);

	MultiPlayerLevel *getLevel(int dimension);

	void tickAllConnections();

	Level *animateTickLevel;

	std::vector<std::wstring> m_pendingTextureRequests;
	std::vector<std::wstring> m_pendingGeometryRequests;

	bool addPendingClientTextureRequest(const std::wstring &textureName);
	void handleClientTextureReceived(const std::wstring &textureName);
	void clearPendingClientTextureRequests() { m_pendingTextureRequests.clear(); }
	bool addPendingClientGeometryRequest(const std::wstring &textureName);
	void handleClientGeometryReceived(const std::wstring &textureName);
	void clearPendingClientGeometryRequests() { m_pendingGeometryRequests.clear(); }

	unsigned int getCurrentTexturePackId();
	ColourTable *getColourTable();
};
