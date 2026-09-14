#include <raylib.h>
#include "scene/scene_manager.h"
#include "scene/scene_setup.h"
#include "scene/scene_game.h"
#include "utils/text.h"
#include "utils/misc.h"
#include "sound/sound.h"
#include "network/bridge.h"
#include "network/network.h"

int main(void) {
	PokajanTable* table = BridgeGetTable();
	BridgeInitTable(table);

	if (!NetworkInit(table)) return -1;

	InitWindow(1920, 1080, "Pokajan!");
	InitAudioDevice();
	SoundLoadBGM();

	LoadFonts();
	#ifdef F_SKIP_TO_GAME
		SceneManagerInit(GameCreate(table));
	#else
		SceneManagerInit(SetupCreate(table));
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
	UnloadFonts();
	SoundUnloadBGM();

	CloseAudioDevice();
	CloseWindow();

	NetworkShutdown();
	return 0;
}