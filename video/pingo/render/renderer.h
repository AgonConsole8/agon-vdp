#pragma once

#include "texture.h"
#include "renderable.h"
#include "pixel.h"
#include "../math/vec4.h"

typedef struct tag_Scene Scene;
typedef struct tag_BackEnd BackEnd;

typedef struct tag_Renderer{
    Vec4i camera;
    Scene * scene;

    Texture frameBuffer;
    Pixel clearColor;
    int clear;

    Mat4 camera_projection;
    Mat4 camera_view;

    BackEnd * backEnd;

} Renderer;

extern int rendererRender(Renderer *);

extern int rendererInit(Renderer *, Vec2i size, struct tag_BackEnd * backEnd);

extern int rendererSetScene(Renderer *r, Scene *s);

extern int rendererSetCamera(Renderer *r, Vec4i camera);
