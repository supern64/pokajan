#ifndef SCENE_H
#define SCENE_H

typedef struct {
    void (*init)(void *self);
    void (*update)(void *self);
    void (*render)(void *self);
    void (*destroy)(void *self);
} SceneVTable;

typedef struct {
    const SceneVTable *vtable;
} Scene;

#endif