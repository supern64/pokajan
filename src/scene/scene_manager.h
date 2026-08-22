#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scene.h"

void SceneManagerInit(Scene *initialScene);
void SceneManagerUpdate(void);
void SceneManagerRender(void);
void SceneManagerShutdown(void);

// Replace the top scene entirely (old one destroyed)
void SceneManagerSwitchTo(Scene *next);

// Push a new scene on top (old one kept underneath, NOT destroyed)
void SceneManagerPush(Scene *next);

// Remove and destroy the top scene, resuming the one below
void SceneManagerPop(void);

#endif