/*================================================================
    * Copyright: 2020 John Jackson
    * gfxt

    The purpose of this example is to demonstrate how to use the gs_gfxt
    util.


    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GFXT_IMPL
#include <gs/util/gs_gfxt.h>

#include "data.c"

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new();
    app->gsi = gs_immediate_draw_new(gs_platform_main_window());
    app->asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
    char TMP[256] = {0};

    // Load pipeline from resource file
    gs_snprintf(TMP, sizeof(TMP), "%s/%s", app->asset_dir, "pipelines/simple.sf");
    app->pip = gs_gfxt_pipeline_load_from_file(TMP);

    // Create material using this pipeline
    app->mat = gs_gfxt_material_create(&(gs_gfxt_material_desc_t){
        .pip_func.hndl = &app->pip 
    });

    // Create mesh that uses the layout from the pipeline's requested mesh layout 
    gs_snprintf(TMP, sizeof(TMP), "%s/%s", app->asset_dir, "meshes/slave.gltf");
    app->mesh = gs_gfxt_mesh_load_from_file(TMP, &(gs_gfxt_mesh_import_options_t){
        .layout = app->pip.mesh_layout,
        .size = gs_dyn_array_size(app->pip.mesh_layout) * sizeof(gs_gfxt_mesh_layout_t),
        .index_buffer_element_size = app->pip.desc.raster.index_buffer_element_size
    }); 

    gs_snprintf(TMP, sizeof(TMP), "%s/%s", app->asset_dir, "textures/slave_albedo.png");
    app->texture = gs_gfxt_texture_load_from_file(TMP, NULL, false, false);
} 

void app_update()
{
    // Cache data for frame
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_t* cb = &app->cb;
    gs_immediate_draw_t* gsi = &app->gsi; 
    gs_gfxt_material_t* mat = &app->mat;
    gs_gfxt_mesh_t* mesh = &app->mesh;
    gs_gfxt_texture_t* tex = &app->texture;

    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const float _t = gs_platform_elapsed_time() * 0.001f;
    const float dt = gs_platform_delta_time(); 

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) 
    {
        gs_quit();
    } 

    // Camera for scene
    gs_camera_t cam = gs_camera_perspective();
    cam.transform.position.z = 20.f;
    gs_mat4 model = gs_mat4_rotatev(_t, GS_YAXIS); 
    gs_mat4 vp = gs_camera_get_view_projection(&cam, fbs.x, fbs.y); 
    gs_mat4 mvp = gs_mat4_mul(vp, model);

    // Apply material uniforms
    gs_gfxt_material_set_uniform(mat, "u_mvp", &mvp);
    gs_gfxt_material_set_uniform(mat, "u_tex", tex);

    // Rendering
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05, 0.05, 1.f}}};
    gs_graphics_begin_render_pass(cb, (gs_handle(gs_graphics_render_pass_t)){0});
    {
        // Set view port
        gs_graphics_set_viewport(cb,0,0,(int)fbs.x,(int)fbs.y);

        // Clear screen
        gs_graphics_clear(cb, &clear); 

        // Bind material
        gs_gfxt_material_bind(cb, mat);

        // Bind material uniforms
        gs_gfxt_material_bind_uniforms(cb, mat);

        // Render mesh
        gs_gfxt_mesh_draw(cb, mesh);
    }
    gs_graphics_end_render_pass(cb);
    
    //Submits to cb
    gs_graphics_submit_command_buffer(cb);
}

void app_shutdown()
{
    // free
    app_t* app = gs_user_data(app_t);
    gs_immediate_draw_free(&app->gsi);
    gs_command_buffer_free(&app->cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window_width = 900,
        .window_height = 580
    };
}


