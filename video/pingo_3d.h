#ifndef PINGO_3D_H
#define PINGO_3D_H

#include <stdint.h>
#include <string.h>
#include <agon.h>
#include <map>
#include "esp_heap_caps.h"
#include "sprites.h"

namespace p3d {

    extern "C" {

        #include "pingo/assets/teapot.h"

        #include "pingo/render/mesh.h"
        #include "pingo/render/object.h"
        #include "pingo/render/pixel.h"
        #include "pingo/render/renderer.h"
        #include "pingo/render/scene.h"
        #include "pingo/render/backend.h"
        #include "pingo/render/depth.h"

    } // extern "C"

} // namespace p3d

#define PINGO_3D_CONTROL_TAG    0x43443350 // "P3DC"

class VDUStreamProcessor;

typedef struct tag_Transformable {
    p3d::Vec3f      m_scale;
    p3d::Vec3f      m_rotation;
    p3d::Vec3f      m_translation;
    p3d::Mat4       m_transform;
    bool            m_modified;

    void initialize() {
        memset(this, 0, sizeof(struct tag_Transformable));
        m_scale.x = 1.0f;
        m_scale.y = 1.0f;
        m_scale.z = 1.0f;
        m_modified = true;
    }

    void compute_transformation_matrix() {
        m_transform = p3d::mat4Scale(m_scale);
        if (m_rotation.x) {
            auto t = p3d::mat4RotateX(m_rotation.x);
            m_transform = mat4MultiplyM(&m_transform, &t);
        }
        if (m_rotation.y) {
            auto t = p3d::mat4RotateY(m_rotation.y);
            m_transform = mat4MultiplyM(&m_transform, &t);
        }
        if (m_rotation.z) {
            auto t = p3d::mat4RotateY(m_rotation.z);
            m_transform = mat4MultiplyM(&m_transform, &t);
        }
        if (m_translation.x || m_translation.y || m_translation.z) {
            auto t = p3d::mat4Translate(m_translation);
            m_transform = mat4MultiplyM(&m_transform, &t);
        }
        m_modified = false;
    }

    void dump() {
        for (int i = 0; i < 16; i++) {
            debug_log("        [%i] %f\n", i, m_transform.elements[i]);
        }
        debug_log("Scale: %f %f %f\n", m_scale.x, m_scale.y, m_scale.z);
        debug_log("Rotation: %f %f %f\n", m_rotation.x, m_rotation.y, m_rotation.z);
        debug_log("Translation: %f %f %f\n", m_translation.x, m_translation.y, m_translation.z);
    }
} Transformable;

typedef struct tag_TexObject : public Transformable {
    p3d::Object     m_object;
    p3d::Texture    m_texture;
    p3d::Material   m_material;
    uint16_t        m_oid;

    void update_transformation_matrix() {
        m_object.material = &m_material;
        m_material.texture = &m_texture;
        compute_transformation_matrix();
        m_object.transform = m_transform;
    }

    void dump() {
        Transformable::dump();
        debug_log("TObject: %p %u\n", this, m_oid);
        debug_log("Object: %p %p %p %p\n", &m_object, m_object.material, m_object.mesh,
                    m_object.transform.elements);
        debug_log("Texture: %p %u %u %p\n", &m_texture, m_texture.size.x, m_texture.size.y, m_texture.frameBuffer);
        debug_log("Material: %p %p %u %u %p\n", &m_material, m_material.texture, m_material.texture->size.x,
                    m_material.texture->size.y, m_material.texture->frameBuffer);
    }
} TexObject;

struct tag_Pingo3dControl;

extern "C" {

    void static_init(p3d::Renderer* ren, p3d::BackEnd* backEnd, p3d::Vec4i _rect);

    void static_before_render(p3d::Renderer* ren, p3d::BackEnd* backEnd);

    void static_after_render(p3d::Renderer* ren, p3d::BackEnd* backEnd);

    p3d::Pixel* static_get_frame_buffer(p3d::Renderer* ren, p3d::BackEnd* backEnd);

    p3d::PingoDepth* static_get_zeta_buffer(p3d::Renderer* ren, p3d::BackEnd* backEnd);

} // extern "C"

typedef struct tag_Pingo3dControl {
    uint32_t            m_tag;              // Used to verify the existence of this structure
    uint32_t            m_size;             // Used to verify the existence of this structure
    VDUStreamProcessor* m_proc;             // Used by subcommands to obtain more data
    p3d::BackEnd        m_backend;          // Used by the renderer
    p3d::Pixel*         m_frame;            // Frame buffer for rendered pixels
    p3d::PingoDepth*    m_zeta;             // Zeta buffer for depth information
    uint16_t            m_width;            // Width of final render in pixels
    uint16_t            m_height;           // Height of final render in pixels
    Transformable       m_camera;           // Camera settings
    std::map<uint16_t, p3d::Mesh>* m_meshes;    // Map of meshes for use by objects
    std::map<uint16_t, TexObject>* m_objects;   // Map of textured objects that use meshes and have transforms

    void show_free_ram() {
        debug_log("Free PSRAM: %u\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }

    // VDU 23, 0, &A0, sid; &48, 0, 1 :  Initialize Control Structure
    void initialize(VDUStreamProcessor& processor, uint16_t width, uint16_t height) {
        debug_log("initialize: pingo creating control structure for %ux%u scene\n", width, height);
        memset(this, 0, sizeof(tag_Pingo3dControl));
        m_tag = PINGO_3D_CONTROL_TAG;
        m_size = sizeof(tag_Pingo3dControl);
        m_width = width;
        m_height = height;
        m_camera.initialize();

        auto frame_size = (uint32_t) width * (uint32_t) height;

        auto size = sizeof(p3d::Pixel) * frame_size;
        m_frame = (p3d::Pixel*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (!m_frame) {
            debug_log("initialize: failed to allocate %u bytes for frame\n", size);
            show_free_ram();
        }

        size = sizeof(p3d::PingoDepth) * frame_size;
        m_zeta = (p3d::PingoDepth*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (!m_zeta) {
            debug_log("initialize: failed to allocate %u bytes for zeta\n", size);
            show_free_ram();
        }

        m_backend.init = &static_init;
        m_backend.beforeRender = &static_before_render;
        m_backend.afterRender = &static_after_render;
        m_backend.getFrameBuffer = &static_get_frame_buffer;
        m_backend.getZetaBuffer = &static_get_zeta_buffer;
        m_backend.drawPixel = NULL;
        m_backend.clientCustomData = (void*) this;

        m_meshes = new std::map<uint16_t, p3d::Mesh>;
        m_objects = new std::map<uint16_t, TexObject>;
    }

    // VDU 23, 0, &A0, sid; &48, 0, 0 :  Deinitialize Control Structure
    void deinitialize(VDUStreamProcessor& processor) {
    }

    bool validate() {
        return (m_tag == PINGO_3D_CONTROL_TAG &&
                m_size == sizeof(tag_Pingo3dControl));
    }

    void handle_subcommand(VDUStreamProcessor& processor, uint8_t subcmd) {
        debug_log("P3D: handle_subcommand(%hu)\n", subcmd);
        m_proc = &processor;
        switch (subcmd) {
            case 1: define_mesh_vertices(); break;
            case 2: set_mesh_vertex_indexes(); break;
            case 3: define_texture_coordinates(); break;
            case 4: set_texture_coordinate_indexes(); break;
            case 5: create_object(); break;
            case 6: set_object_x_scale_factor(); break;
            case 7: set_object_y_scale_factor(); break;
            case 8: set_object_z_scale_factor(); break;
            case 9: set_object_xyz_scale_factors(); break;
            case 10: set_object_x_rotation_angle(); break;
            case 11: set_object_y_rotation_angle(); break;
            case 12: set_object_z_rotation_angle(); break;
            case 13: set_object_xyz_rotation_angles(); break;
            case 14: set_object_x_translation_distance(); break;
            case 15: set_object_y_translation_distance(); break;
            case 16: set_object_z_translation_distance(); break;
            case 17: set_object_xyz_translation_distances(); break;

            case 18: set_camera_x_scale_factor(); break;
            case 19: set_camera_y_scale_factor(); break;
            case 20: set_camera_z_scale_factor(); break;
            case 21: set_camera_xyz_scale_factors(); break;
            case 22: set_camera_x_rotation_angle(); break;
            case 23: set_camera_y_rotation_angle(); break;
            case 24: set_camera_z_rotation_angle(); break;
            case 25: set_camera_xyz_rotation_angles(); break;
            case 26: set_camera_x_translation_distance(); break;
            case 27: set_camera_y_translation_distance(); break;
            case 28: set_camera_z_translation_distance(); break;
            case 29: set_camera_xyz_translation_distances(); break;
            case 30: render_to_bitmap(); break;
        }
    }

    p3d::Mesh* establish_mesh(uint16_t mid) {
        auto mesh_iter = m_meshes->find(mid);
        if (mesh_iter == m_meshes->end()) {
            p3d::Mesh mesh;
            memset(&mesh, 0, sizeof(mesh));
            (*m_meshes).insert(std::pair<uint16_t, p3d::Mesh>(mid, mesh));
            return &m_meshes->find(mid)->second;
        } else {
            return &mesh_iter->second;
        }
    }

    p3d::Mesh* get_mesh() {
        auto mid = m_proc->readWord_t();
        if (mid >= 0) {
            return establish_mesh(mid);
        }
        return NULL;
    }

    TexObject* establish_object(uint16_t oid) {
        auto object_iter = m_objects->find(oid);
        if (object_iter == m_objects->end()) {
            TexObject object;
            memset(&object, 0, sizeof(object));
            object.m_oid = oid;
            object.m_object.material = &object.m_material;
            object.m_material.texture = &object.m_texture;
            object.initialize();
            (*m_objects).insert(std::pair<uint16_t, TexObject>(oid, object));
            return &m_objects->find(oid)->second;
        } else {
            return &object_iter->second;
        }
    }

    TexObject* get_object() {
        auto oid = m_proc->readWord_t();
        if (oid >= 0) {
            return establish_object(oid);
        }
        return NULL;
    }

    // VDU 23, 0, &A0, sid; &48, 1, mid; n; x0; y0; z0; ... :  Define Mesh Vertices
    void define_mesh_vertices() {
        auto mesh = get_mesh();
        if (mesh->positions) {
            heap_caps_free(mesh->positions);
            mesh->positions = NULL;
        }
        auto n = (uint32_t) m_proc->readWord_t();
        if (n > 0) {
            auto size = n*sizeof(p3d::Vec3f);
            mesh->positions = (p3d::Vec3f*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
            auto pos = mesh->positions;
            if (!pos) {
                debug_log("define_mesh_vertices: failed to allocate %u bytes\n", size);
                show_free_ram();
            }
            debug_log("Reading %u vertices\n", n);
            for (uint32_t i = 0; i < n; i++) {
                uint16_t x = m_proc->readWord_t();
                uint16_t y = m_proc->readWord_t();
                uint16_t z = m_proc->readWord_t();
                if (pos) {
                    pos->x = convert_position_value(x);
                    pos->y = convert_position_value(y);
                    pos->z = convert_position_value(z);
                    if (!(i & 0x1F)) debug_log("%u %f %f %f\n", i, pos->x, pos->y, pos->z);
                    pos++;
                }
            }
            debug_log("\n");
        }
    }

    // VDU 23, 0, &A0, sid; &48, 2, mid; n; i0; ... :  Set Mesh Vertex Indexes
    void set_mesh_vertex_indexes() {
        auto mesh = get_mesh();
        if (mesh->pos_indices) {
            heap_caps_free(mesh->pos_indices);
            mesh->pos_indices = NULL;
            mesh->indexes_count = 0;
        }
        auto n = (uint32_t) m_proc->readWord_t();
        if (n > 0) {
            mesh->indexes_count = n;
            auto size = n*sizeof(uint16_t);
            mesh->pos_indices = (uint16_t*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
            auto idx = mesh->pos_indices;
            if (!idx) {
                debug_log("set_mesh_vertex_indexes: failed to allocate %u bytes\n", size);
                show_free_ram();
            }
            debug_log("Reading %u vertex indexes\n", n);
            for (uint32_t i = 0; i < n; i++) {
                uint16_t index = m_proc->readWord_t();
                if (idx) {
                    *idx++ = index;
                }
                if (!(i & 0x1F)) debug_log("%u %hu\n", i, index);
            }
            debug_log("\n");
        }
    }

    // VDU 23, 0, &A0, sid; &48, 3, mid; n; u0; v0; ... :  Define Texture Coordinates
    void define_texture_coordinates() {
        auto mesh = get_mesh();
        if (mesh->textCoord) {
            heap_caps_free(mesh->textCoord);
            mesh->textCoord = NULL;
        }
        auto n = (uint32_t) m_proc->readWord_t();
        if (n > 0) {
            auto size = n*sizeof(p3d::Vec2f);
            mesh->textCoord = (p3d::Vec2f*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
            auto coord = mesh->textCoord;
            if (!coord) {
                debug_log("set_mesh_vertex_indexes: failed to allocate %u bytes\n", size);
                show_free_ram();
            }
            debug_log("Reading %u texture coordinates\n", n);
            for (uint32_t i = 0; i < n; i++) {
                uint16_t u = m_proc->readWord_t();
                uint16_t v = m_proc->readWord_t();
                if (coord) {
                    coord->x = convert_texture_coordinate_value(u);
                    coord->y = convert_texture_coordinate_value(v);
                    coord++;
                }
            }
        }
    }

    // VDU 23, 0, &A0, sid; &48, 4, mid; n; i0; ... :  Set Texture Coordinate Indexes
    void set_texture_coordinate_indexes() {
        auto mesh = get_mesh();
        if (mesh->tex_indices) {
            heap_caps_free(mesh->tex_indices);
            mesh->tex_indices = NULL;
        }
        auto n = (uint32_t) m_proc->readWord_t();
        if (n > 0) {
            auto size = n*sizeof(uint16_t);
            mesh->tex_indices = (uint16_t*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
            auto idx = mesh->tex_indices;
            if (!idx) {
                debug_log("set_texture_coordinate_indexes: failed to allocate %u bytes\n", size);
                show_free_ram();
            }
            debug_log("Reading %u texture coordinate indexes\n", n);
            for (uint32_t i = 0; i < n; i++) {
                uint16_t index = m_proc->readWord_t();
                if (idx && (i < mesh->indexes_count)) {
                    *idx++ = index;
                }
                if (!(i & 0x1F)) debug_log("%u %hu\n", i, index);
            }
        }
    }

    // VDU 23, 0, &A0, sid; &48, 5, oid; mid; bmid; :  Create Object
    void create_object() {
        auto object = get_object();
        auto mesh = get_mesh();
        auto bmid = m_proc->readWord_t();
        if (object && mesh && bmid) {
            debug_log("Creating 3D object %u with bitmap %u\n", object->m_oid, bmid);
            auto stored_bitmap = getBitmap(bmid);
            if (stored_bitmap) {
                auto bitmap = stored_bitmap.get();
                if (bitmap) {
                    auto size = p3d::Vec2i{(p3d::I_TYPE)bitmap->width, (p3d::I_TYPE)bitmap->height};
                    auto pix = (p3d::Pixel*) bitmap->data;
                    texture_init(&object->m_texture, size, pix);
                    object->m_object.mesh = mesh;
                    debug_log("Texture data:  %02hX %02hX %02hX %02hX\n", pix->r, pix->g, pix->b, pix->a);
                }
            }
        }
    }

    p3d::F_TYPE convert_scale_value(int32_t value) {
        static const p3d::F_TYPE factor = 1.0f / 256.0f;
        return ((p3d::F_TYPE) value) * factor;
    }

    p3d::F_TYPE convert_rotation_value(int32_t value) {
        if (value & 0x8000) {
            value = (int32_t)(int16_t)(uint16_t) value;
        }
        static const p3d::F_TYPE factor = (2.0f * 3.1415926f) / 32767.0f;
        return ((p3d::F_TYPE) value) * factor;
    }

    p3d::F_TYPE convert_translation_value(int32_t value) {
        if (value & 0x8000) {
            value = (int32_t)(int16_t)(uint16_t) value;
        }
        static const p3d::F_TYPE factor = 1.0f / 256.0f;
        return ((p3d::F_TYPE) value) * factor;
    }

    p3d::F_TYPE convert_position_value(int32_t value) {
        if (value & 0x8000) {
            value = (int32_t)(int16_t)(uint16_t) value;
        }
        static const p3d::F_TYPE factor = 1.0f / 32767.0f;
        return ((p3d::F_TYPE) value) * factor;
    }

    p3d::F_TYPE convert_texture_coordinate_value(int32_t value) {
        static const p3d::F_TYPE factor = 1.0f / 65535.0f;
        return ((p3d::F_TYPE) value) * factor;
    }

    // VDU 23, 0, &A0, sid; &48, 6, oid; scalex; :  Set Object X Scale Factor
    void set_object_x_scale_factor() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_scale.x = convert_scale_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 7, oid; scaley; :  Set Object Y Scale Factor
    void set_object_y_scale_factor() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_scale.y = convert_scale_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 8, oid; scalez; :  Set Object Z Scale Factor
    void set_object_z_scale_factor() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_scale.y = convert_scale_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 9, oid; scalex; scaley; scalez :  Set Object XYZ Scale Factors
    void set_object_xyz_scale_factors() {
        auto object = get_object();
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if (object && (valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            object->m_scale.x = convert_scale_value(valuex);
            object->m_scale.y = convert_scale_value(valuey);
            object->m_scale.z = convert_scale_value(valuez);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 10, oid; anglex; :  Set Object X Rotation Angle
    void set_object_x_rotation_angle() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_rotation.x = convert_rotation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 11, oid; angley; :  Set Object Y Rotation Angle
    void set_object_y_rotation_angle() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_rotation.y = convert_rotation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 12, oid; anglez; :  Set Object Z Rotation Angle
    void set_object_z_rotation_angle() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_rotation.z = convert_rotation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 13, oid; anglex; angley; anglez; :  Set Object XYZ Rotation Angles
    void set_object_xyz_rotation_angles() {
        auto object = get_object();
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if (object && (valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            object->m_rotation.x = convert_rotation_value(valuex);
            object->m_rotation.y = convert_rotation_value(valuey);
            object->m_rotation.z = convert_rotation_value(valuez);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 14, oid; distx; :  Set Object X Translation Distance
    void set_object_x_translation_distance() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_translation.x = convert_translation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 15, oid; disty; :  Set Object Y Translation Distance
    void set_object_y_translation_distance() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_translation.y = convert_translation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 16, oid; distz; :  Set Object Z Translation Distance
    void set_object_z_translation_distance() {
        auto object = get_object();
        auto value = m_proc->readWord_t();
        if (object && (value >= 0)) {
            object->m_translation.z = convert_translation_value(value);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 17, oid; distx; disty; distz :  Set Object XYZ Translation Distances
    void set_object_xyz_translation_distances() {        auto object = get_object();
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if (object && (valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            object->m_translation.x = convert_translation_value(valuex);
            object->m_translation.y = convert_translation_value(valuey);
            object->m_translation.z = convert_translation_value(valuez);
            object->m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 18, oid; scalex; :  Set Camera X Scale Factor
    void set_camera_x_scale_factor() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_scale.x = convert_scale_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 19, oid; scaley; :  Set Camera Y Scale Factor
    void set_camera_y_scale_factor() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_scale.y = convert_scale_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 20, oid; scalez; :  Set Camera Z Scale Factor
    void set_camera_z_scale_factor() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_scale.y = convert_scale_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 21, oid; scalex; scaley; scalez :  Set Camera XYZ Scale Factors
    void set_camera_xyz_scale_factors() {
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if ((valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            m_camera.m_scale.x = convert_scale_value(valuex);
            m_camera.m_scale.y = convert_scale_value(valuey);
            m_camera.m_scale.z = convert_scale_value(valuez);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 22, oid; anglex; :  Set Camera X Rotation Angle
    void set_camera_x_rotation_angle() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_rotation.x = convert_rotation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 23, oid; angley; :  Set Camera Y Rotation Angle
    void set_camera_y_rotation_angle() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_rotation.y = convert_rotation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 24, oid; anglez; :  Set Camera Z Rotation Angle
    void set_camera_z_rotation_angle() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_rotation.z = convert_rotation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 25, oid; anglex; angley; anglez; :  Set Camera XYZ Rotation Angles
    void set_camera_xyz_rotation_angles() {
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if ((valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            m_camera.m_rotation.x = convert_rotation_value(valuex);
            m_camera.m_rotation.y = convert_rotation_value(valuey);
            m_camera.m_rotation.z = convert_rotation_value(valuez);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 26, oid; distx; :  Set Camera X Translation Distance
    void set_camera_x_translation_distance() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_translation.x = convert_translation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 27, oid; disty; :  Set Camera Y Translation Distance
    void set_camera_y_translation_distance() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_translation.y = convert_translation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 28, oid; distz; :  Set Camera Z Translation Distance
    void set_camera_z_translation_distance() {
        auto value = m_proc->readWord_t();
        if (value >= 0) {
            m_camera.m_translation.z = convert_translation_value(value);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 29, oid; distx; disty; distz :  Set Camera XYZ Translation Distances
    void set_camera_xyz_translation_distances() {        auto object = get_object();
        auto valuex = m_proc->readWord_t();
        auto valuey = m_proc->readWord_t();
        auto valuez = m_proc->readWord_t();
        if ((valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
            m_camera.m_translation.x = convert_translation_value(valuex);
            m_camera.m_translation.y = convert_translation_value(valuey);
            m_camera.m_translation.z = convert_translation_value(valuez);
            m_camera.m_modified = true;
        }
    }

    // VDU 23, 0, &A0, sid; &48, 30, bmid; :  Render To Bitmap
    void render_to_bitmap() {
        auto bmid = m_proc->readWord_t();
        if (bmid < 0) {
            return;
        }

        p3d::Pixel* dst_pix = NULL;
        auto old_bitmap = getBitmap(bmid);
        if (old_bitmap) {
            auto bitmap = old_bitmap.get();
            if (bitmap && bitmap->width == m_width && bitmap->height == m_height) {
                dst_pix = (p3d::Pixel*) bitmap->data;
            }
        }

        if (!dst_pix) {
            debug_log("render_to_bitmap: output bitmap %u not found or invalid\n", bmid);
        }

        auto start = millis();
        auto size = p3d::Vec2i{(p3d::I_TYPE)m_width, (p3d::I_TYPE)m_height};
        p3d::Renderer renderer;
        rendererInit(&renderer, size, &m_backend );
        rendererSetCamera(&renderer,(p3d::Vec4i){0,0,size.x,size.y});

        p3d::Scene scene;
        sceneInit(&scene);
        p3d::rendererSetScene(&renderer, &scene);

        for (auto object = m_objects->begin(); object != m_objects->end(); object++) {
            if (object->second.m_modified) {
                object->second.update_transformation_matrix();
                //object->second.dump();
            }
            sceneAddRenderable(&scene, p3d::object_as_renderable(&object->second.m_object));
        }

        p3d::F_TYPE phi = 0;

        // Set the projection matrix
        renderer.camera_projection =
            p3d::mat4Perspective( 1, 2500.0, (p3d::F_TYPE)size.x / (p3d::F_TYPE)size.y, 0.6);

        if (m_camera.m_modified) {
            m_camera.compute_transformation_matrix();
        }
        renderer.camera_view = m_camera.m_transform;

        // Set the scene transformation matrix
        scene.transform = p3d::mat4RotateY(phi);
        phi += 0.01;

        debug_log("Frame data:  %02hX %02hX %02hX %02hX\n", m_frame->r, m_frame->g, m_frame->b, m_frame->a);
        debug_log("Destination: %02hX %02hX %02hX %02hX\n", dst_pix->r, dst_pix->g, dst_pix->b, dst_pix->a);

        rendererRender(&renderer);

        memcpy(dst_pix, m_frame, sizeof(p3d::Pixel) * m_width * m_height);

        auto stop = millis();
        auto diff = stop - start;
        debug_log("Render to %ux%u took %u ms\n", m_width, m_height, diff);
        debug_log("Frame data:  %02hX %02hX %02hX %02hX\n", m_frame->r, m_frame->g, m_frame->b, m_frame->a);
        debug_log("Final data:  %02hX %02hX %02hX %02hX\n", dst_pix->r, dst_pix->g, dst_pix->b, dst_pix->a);
    }

} Pingo3dControl;

extern "C" {

    void static_init(p3d::Renderer* ren, p3d::BackEnd* backEnd, p3d::Vec4i _rect) {
        //rect = _rect;
    }

    void static_before_render(p3d::Renderer* ren, p3d::BackEnd* backEnd) {
    }

    void static_after_render(p3d::Renderer* ren, p3d::BackEnd* backEnd) {
    }

    p3d::Pixel* static_get_frame_buffer(p3d::Renderer* ren, p3d::BackEnd* backEnd) {
        auto p_this = (struct tag_Pingo3dControl*) backEnd->clientCustomData;
        return p_this->m_frame;
    }

    p3d::PingoDepth* static_get_zeta_buffer(p3d::Renderer* ren, p3d::BackEnd* backEnd) {
        auto p_this = (struct tag_Pingo3dControl*) backEnd->clientCustomData;
        return p_this->m_zeta;
    }

#if DEBUG
    void show_pixel(float x, float y, uint8_t a, uint8_t b, uint8_t g, uint8_t r) {
        debug_log("%f %f %02hX %02hX %02hX %02hX\n", x, y, a, b, g, r);
    }
#endif

} // extern "C"

#endif // PINGO_3D_H
