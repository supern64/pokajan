#include <raylib.h>
#include "scene/scene.h"
#include "scene/scene_manager.h"
#include "scene/scene_title.h"
#include "utils/text.h"
#include "sound/sound.h"

int main(void) {
	InitWindow(1920, 1080, "Pokajan!");
	InitAudioDevice();
	SoundLoadBGM();

	LoadFonts();
	SceneManagerInit(TitleCreate());

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		SceneManagerUpdate();

		BeginDrawing();
		SceneManagerRender();
		EndDrawing();
	}

	SceneManagerShutdown();
	UnloadFonts();

	CloseAudioDevice();
	CloseWindow();
	return 0;
}