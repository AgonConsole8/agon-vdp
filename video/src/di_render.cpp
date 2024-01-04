// di_render.cpp - Function definitions for drawing bitmaps via 3D rendering
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// This code reliese on 'pingo' for its rendering functions. Refer to the
// README.md and LICENSE in the pingo directory for more information.
//

#include "di_render.h"
#include "esp_heap_caps.h"

extern "C" {

#include "pingo/assets/teapot.h"

#include "pingo/render/mesh.h"
#include "pingo/render/object.h"
#include "pingo/render/pixel.h"
#include "pingo/render/renderer.h"
#include "pingo/render/scene.h"
#include "pingo/render/backend.h"
#include "pingo/render/depth.h"
#include <cstring>

typedef struct Pixel Pixel;

typedef  struct {
    BackEnd backend;
    Vec2i size;
} DiRenderBackEnd;

void di_render_init(DiRenderBackEnd * backend, Vec2i size);

DiRenderBackEnd my_backend;
Vec4i rect;
Vec2i totalSize;
PingoDepth * zetaBuffer;
Pixel * frameBuffer;
PingoDepth * my_zetaBuffer;


void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    rect = _rect;

}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    //DiRenderBackEnd * backend = (DiRenderBackEnd *) backEnd;
}

void afterRender( Renderer * ren,  BackEnd * backEnd) {
}

Pixel * getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

PingoDepth * getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return my_zetaBuffer;
}

void di_render_init( DiRenderBackEnd * backend, Vec2i size) {
  frameBuffer = (Pixel*) heap_caps_malloc(sizeof(Pixel) * 160 * 120, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
  my_zetaBuffer = (PingoDepth*) heap_caps_malloc(sizeof(PingoDepth) * 160 * 120, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

  totalSize = size;
  backend->backend.init = &init;
  backend->backend.beforeRender = &beforeRender;
  backend->backend.afterRender = &afterRender;
  backend->backend.getFrameBuffer = &getFrameBuffer;
  backend->backend.getZetaBuffer = &getZetaBuffer;
  backend->backend.drawPixel = 0;
}

} // extern "C"

//-----------------------------------------

DiRender::DiRender(uint32_t width, uint32_t height, uint16_t flags, bool use_psram) :
  DiBitmap(width, height, flags, use_psram) {
}

DiRender::~DiRender() {
}

extern "C" {

float angleZ = 3.142128;

void do_render(int width, int height) {
  Vec2i size = {width, height};

  di_render_init(&my_backend, size);
  Renderer renderer;
  rendererInit(&renderer, size,(BackEnd*) &my_backend );
  rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});

  Scene s;
  sceneInit(&s);
  rendererSetScene(&renderer, &s);

  Object object;
  //object.material = NULL;
  object.mesh = &mesh_teapot;

/*
  Material m;
  Texture tex;
  tex.size = Vec2i{2,2};
  Pixel solid[4];
  solid[0] = Pixel { 0xFE, 0xFD, 0x00, 0xFF };
  solid[1] = Pixel { 0xEE, 0x00, 0xEC, 0xFF };
  solid[2] = Pixel { 0x00, 0xDD, 0xDC, 0xFF };
  solid[3] = Pixel { 0xCE, 0x00, 0x00, 0xFF };
  tex.frameBuffer = solid;
  m.texture = &tex;
  object.material = &m;
*/
  object.material = NULL;

  sceneAddRenderable(&s, object_as_renderable(&object));

  float phi = 0;
  Mat4 t;

  // PROJECTION MATRIX - Defines the type of projection used
  renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

  //VIEW MATRIX - Defines position and orientation of the "camera"
  Mat4 v = mat4Translate((Vec3f) { 0,2,-35});

  Mat4 rotateDown = mat4RotateX(-0.40); //Rotate around origin/orbit
  renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

  object.transform = mat4Scale(Vec3f { 6.0f, 6.0f, 6.0f });
  
  //TRANSFORM - Defines position and orientation of the object
  t = mat4RotateZ(angleZ);
  angleZ += 3.142128f/4.0f;

  object.transform = mat4MultiplyM(&object.transform, &t );  

  t = mat4RotateZ(0);
  object.transform = mat4MultiplyM(&object.transform, &t );

  //SCENE
  s.transform = mat4RotateY(phi);
  phi += 0.01;

  rendererRender(&renderer);
}

} // extern "C"

void DiRender::render() {
  do_render(m_width, m_height);

  Pixel* p_render_pixels = frameBuffer;
	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {
			uint8_t c = ((p_render_pixels->b >> 6) << 4) |
                  ((p_render_pixels->g >> 6) << 2) |
                  ((p_render_pixels->r >> 6));
			set_transparent_pixel(x, y, c|PIXEL_ALPHA_100_MASK);
      p_render_pixels++;
		}
	}
}
