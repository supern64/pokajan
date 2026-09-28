#include "scene_result.h"
#include <raylib.h>
#include "scene_manager.h"

typedef struct {
	Scene base;
} ResultScene;

static void ResultStart(void *self) {

}

static void ResultUpdate(void *self) {

}

static void ResultRender(void *self) {

}

static void ResultDestroy(void *self) {
    free(self);
}

static const SceneVTable resultVTable = {
    .start = ResultStart,
    .update = ResultUpdate,
    .render = ResultRender,
    .destroy = ResultDestroy
};

Scene *ResultCreate(void) {
    ResultScene *s = malloc(sizeof(ResultScene));
    s->base.vtable = &resultVTable;
    return (Scene*)s;
}