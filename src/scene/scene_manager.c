#include "scene_manager.h"
#include <stddef.h>

#define MAX_SCENES 8

typedef enum {
    PENDING_NONE,
    PENDING_SWITCH,
    PENDING_PUSH,
    PENDING_POP,
} PendingOp;

static Scene *stack[MAX_SCENES];
static int stackSize = 0;

static PendingOp pendingOp = PENDING_NONE;
static Scene *pendingScene = NULL;   // used by SWITCH and PUSH

void SceneManagerInit(Scene *initialScene) {
    stack[0] = initialScene;
    stackSize = 1;
}

void SceneManagerSwitchTo(Scene *next) {
    pendingOp = PENDING_SWITCH;
    pendingScene = next;
}

void SceneManagerPush(Scene *next) {
    pendingOp = PENDING_PUSH;
    pendingScene = next;
}

void sceneManagerPop(void) {
    pendingOp = PENDING_POP;
}

static Scene *top(void) {
    return stackSize > 0 ? stack[stackSize - 1] : NULL;
}

static void applyPending(void) {
    switch (pendingOp) {
        case PENDING_SWITCH: {
            Scene *old = top();
            if (old) old->vtable->destroy(old);
            stack[stackSize - 1] = pendingScene;
            break;
        }
        case PENDING_PUSH: {
            if (stackSize < MAX_SCENES) {
                stack[stackSize++] = pendingScene;
            }
            break;
        }
        case PENDING_POP: {
            Scene *old = top();
            if (old && stackSize > 0) {
                old->vtable->destroy(old);
                stackSize--;
            }
            break;
        }
        case PENDING_NONE:
        default:
            break;
    }
    pendingOp = PENDING_NONE;
    pendingScene = NULL;
}

void SceneManagerUpdate(void) {
    Scene *t = top();
    if (t) t->vtable->update(t);

    // Safe now — update() has fully returned, nothing is running on 'old'
    applyPending();
}

void SceneManagerRender(void) {
    // Draw bottom-to-top so overlays (pause menu) show what's paused behind them
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