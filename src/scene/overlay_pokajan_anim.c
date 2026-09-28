#include "overlay_pokajan_anim.h"
#include <raylib.h>
#include "scene_manager.h"

typedef struct {
	Scene base;
} PokajanAnimOverlay;

static void PokajanAnimStart(void *self) {

}

static void PokajanAnimUpdate(void *self) {

}

static void PokajanAnimRender(void *self) {

}

static void PokajanAnimDestroy(void *self) {
    free(self);
}

static const SceneVTable pokajanAnimVTable = {
    .start = PokajanAnimStart,
    .update = PokajanAnimUpdate,
    .render = PokajanAnimRender,
    .destroy = PokajanAnimDestroy
};

Scene *PokajanAnimCreate(void) {
    PokajanAnimOverlay *s = malloc(sizeof(PokajanAnimOverlay));
    s->base.vtable = &pokajanAnimVTable;
    return (Scene*)s;
}