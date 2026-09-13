#include "scene_setup.h"
#include "scene_manager.h"
#include "../network/bridge.h"

typedef struct {
    Scene base;
    PokajanTable* table;
} SetupScene;

static void SetupInit(void *self) {

}

static void SetupStart(void *self) {

}

static void SetupUpdate(void *self) {
    
}

static void SetupRender(void *self) {

}

static void SetupDestroy(void *self) {
    free(self);
}

static const SceneVTable setupVTable = {
	.start = SetupStart,
	.update = SetupUpdate,
	.render = SetupRender,
	.destroy = SetupDestroy
};

Scene *SetupCreate(PokajanTable* table) {
    SetupScene *s = malloc(sizeof(SetupScene));
    s->base.vtable = &setupVTable;
    s->table = table;
    SetupInit(s);
    return (Scene *)s;
}