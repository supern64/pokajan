#include "scene_manager.h"
#include <stddef.h>

#define MAX_SCENES 8
#define MAX_PENDING 4

typedef enum {
    PENDING_SWITCH,
    PENDING_PUSH,
    PENDING_POP,
} PendingOpType;

typedef struct {
    PendingOpType type;
    Scene *scene;
} PendingAction;

static Scene *stack[MAX_SCENES];
static int stackSize = 0;

static PendingAction pendingQueue[MAX_PENDING];
static int pendingCount = 0;

static Scene *Top(void) {
    return stackSize > 0 ? stack[stackSize - 1] : NULL;
}

void SceneManagerSwitchTo(Scene *next) {
    if (pendingCount < MAX_PENDING)
        pendingQueue[pendingCount++] = (PendingAction){ PENDING_SWITCH, next };
}

void SceneManagerPush(Scene *next) {
    if (pendingCount < MAX_PENDING)
        pendingQueue[pendingCount++] = (PendingAction){ PENDING_PUSH, next };
}

void SceneManagerPop(void) {
    if (pendingCount < MAX_PENDING)
        pendingQueue[pendingCount++] = (PendingAction){ PENDING_POP, NULL };
}

static void ApplyPending(void) {
    for (int i = 0; i < pendingCount; i++) {
        PendingAction *a = &pendingQueue[i];
        switch (a->type) {
            case PENDING_SWITCH: {
                Scene *old = Top();
                if (old) old->vtable->destroy(old);
                stack[stackSize - 1] = a->scene;
                a->scene->vtable->start(a->scene);
                break;
            }
            case PENDING_PUSH:
                if (stackSize < MAX_SCENES) {
                    stack[stackSize++] = a->scene;
                    a->scene->vtable->start(a->scene);
                }
                break;
            case PENDING_POP: {
                Scene *old = Top();
                if (old && stackSize > 0) {
                    old->vtable->destroy(old);
                    stackSize--;
                }
                break;
            }
        }
    }
    pendingCount = 0;
}

void SceneManagerInit(Scene *initialScene) {
    stack[0] = initialScene;
    stackSize = 1;
    initialScene->vtable->start(initialScene);   // <-- was init
}

void SceneManagerUpdate(void) {
    Scene *t = Top();
    if (t) t->vtable->update(t);
    ApplyPending();
}

void SceneManagerRender(void) {
    for (int i = 0; i < stackSize; i++) {
        stack[i]->vtable->render(stack[i]);
    }
}

void SceneManagerShutdown(void) {
    for (int i = 0; i < stackSize; i++) {
        stack[i]->vtable->destroy(stack[i]);
    }
    stackSize = 0;
}