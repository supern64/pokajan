#include <raylib.h>
#include <time.h>
#include "component/component_char_mini_icon.h"
#include "component/component_hud.h"
#include "scene/scene_manager.h"
#include "scene/scene_title.h"
#include "scene/scene_game.h"
#include "scene/scene_setup.h"
#include "pokajan_core/cards.h"
#include "utils/text.h"
#include "utils/misc.h"
#include "sound/sound.h"
#include "network/bridge.h"
#include "network/network.h"

int main(void) {
	PokajanBuildLinearOrders();
	BridgeLoadCardTable();
	SetRandomSeed(time(NULL));
	PokajanTable* table = BridgeGetTable();
	BridgeInitTable(table);

	if (!NetworkInit(table)) return -1;

	InitWindow(1920, 1080, "Pokajan!");
	InitAudioDevice();

	// load specific assets that are used throughout the whole game
	SoundLoadBGM();
	SoundLoadSFX();
	LoadFonts();
	CharMiniIconLoad();
	HUDLoad();

	#ifdef F_SKIP_TO_GAME
		SceneManagerInit(GameCreate(table));
	#else
		SceneManagerInit(TitleCreate(table));
	#endif

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		NetworkLoop();
		SceneManagerUpdate();

		BeginDrawing();
			SceneManagerRender();
		EndDrawing();
	}

	SceneManagerShutdown();

	HUDUnload();
	CharMiniIconUnload();
	UnloadFonts();
	SoundUnloadSFX();
	SoundUnloadBGM();

	CloseAudioDevice();
	CloseWindow();

	NetworkShutdown();
	return 0;
}