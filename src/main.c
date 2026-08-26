#include <raylib.h>
#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "scene/scene_title.h"
#include "scene/scene_game.h"
#include "utils/text.h"
#include "utils/misc.h"
#include "sound/sound.h"

int main(void) {
	InitWindow(1920, 1080, "Pokajan!");
	InitAudioDevice();
	SoundLoadBGM();

	LoadFonts();
	#ifdef F_SKIP_TO_GAME
		SceneManagerInit(GameCreate());
	#else
		SceneManagerInit(TitleCreate());
	#endif

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
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
	return 0;
}