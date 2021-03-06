#include "sprite_renderer.hpp"
#include <toolkit/render_window.hpp>
#include <toolkit/vertex.hpp>
#include <SFML/Graphics.hpp>
#include <toolkit/fs_helpers.hpp>
#include "camera.hpp"
#include <math.h>

sprite_renderer::sprite_renderer()
{
    std::string spritesheet_name = "res/monochrome_transparent.png";
    std::string spritesheet_data = file::read(spritesheet_name, file::mode::BINARY);

    assert(spritesheet_data.size() > 0);

    sf::Image img;
    img.loadFromMemory(spritesheet_data.c_str(), spritesheet_data.size());

    for(int y=0; y < img.getSize().y; y++)
    {
        for(int x=0; x < img.getSize().x; x++)
        {
            sf::Color col = img.getPixel(x, y);

            if(col.a == 0)
            {
                col = sf::Color(255, 255, 255, 0);

                img.setPixel(x, y, col);
            }
        }
    }

    texture_settings tex_sett;
    tex_sett.width = img.getSize().x;
    tex_sett.height = img.getSize().y;
    tex_sett.is_srgb = true;
    tex_sett.magnify_linear = false;

    sprite_sheet.load_from_memory(tex_sett, img.getPixelsPtr());
}

void sprite_renderer::add(const sprite_handle& handle, const render_descriptor& descriptor)
{
    next_renderables.push_back({ handle, descriptor });
}

void sprite_renderer::render(render_window& window, const camera& cam)
{
    std::vector<vertex> vertices;
    vertices.reserve(next_renderables.size() * 6);

    vec2i screen_dimensions = window.get_window_size();

    vec2f tl_visible = cam.screen_to_world(window, { 0,0 }) - vec2f{ TILE_PIX, TILE_PIX };
    vec2f br_visible = cam.screen_to_world(window, { screen_dimensions.x(), screen_dimensions.y() }) + vec2f{ TILE_PIX, TILE_PIX };

    vec2f uv_scale = { 1.f / sprite_sheet.dim.x(), 1.f / sprite_sheet.dim.y() };

    float camera_scale = cam.calculate_scale();

    for (auto [handle, desc] : next_renderables)
    {
        if(desc.pos.x() < tl_visible.x() || desc.pos.y() < tl_visible.y() || desc.pos.x() > br_visible.x() || desc.pos.y() > br_visible.y())
            continue;

        vec2f real_pos = cam.world_to_screen(window, desc.pos);
        vec2f real_dim = vec2f{TILE_PIX, TILE_PIX} * camera_scale;

        vec2f origin = real_dim / 2.f;

        vec2f tl_local = -origin;
        vec2f tr_local = -origin + vec2f{ real_dim.x(), 0 };
        vec2f br_local = -origin + vec2f{ real_dim.x(), real_dim.y() };
        vec2f bl_local = -origin + vec2f{ 0, real_dim.y() };

        tl_local *= desc.scale;
        tr_local *= desc.scale;
        br_local *= desc.scale;
        bl_local *= desc.scale;

        if(desc.angle != 0)
        {
            tl_local = tl_local.rot(desc.angle);
            tr_local = tr_local.rot(desc.angle);
            br_local = br_local.rot(desc.angle);
            bl_local = bl_local.rot(desc.angle);
        }

        vertex tl, tr, br, bl;
        tl.position = tl_local + real_pos;
        tr.position = tr_local + real_pos;
        br.position = br_local + real_pos;
        bl.position = bl_local + real_pos;

        tl.position = round(tl.position);
        tr.position = round(tr.position);
        br.position = round(br.position);
        bl.position = round(bl.position);

        //printf("Diff %f\n", br.position.x() - tl.position.x());

        //printf("TLP %f %f\n", tl.position.x(), tl.position.y());
        //printf("BRP %f %f\n", br.position.x(), br.position.y());

        vec2i texture_coordinate = handle.offset * (TILE_PIX + TILE_SEP);

        vec2f tltx = { texture_coordinate.x(), texture_coordinate.y() };
        vec2f trtx = { texture_coordinate.x() + TILE_PIX, texture_coordinate.y()};
        vec2f brtx = { texture_coordinate.x() + TILE_PIX, texture_coordinate.y() + TILE_PIX };
        vec2f bltx = { texture_coordinate.x(), texture_coordinate.y() + TILE_PIX };

        /*tltx += vec2f{0.5f, 0.5f};
        trtx += vec2f{0.5f, 0.5f};
        brtx += vec2f{0.5f, 0.5f};
        bltx += vec2f{0.5f, 0.5f};*/

        tltx = tltx * uv_scale;
        trtx = trtx * uv_scale;
        brtx = brtx * uv_scale;
        bltx = bltx * uv_scale;

        tl.uv = tltx;
        tr.uv = trtx;
        br.uv = brtx;
        bl.uv = bltx;

        float shade = 0.05;

        vec4f base_colour = handle.base_colour * desc.colour;

        vec4f tl_col = clamp(base_colour * (1 + shade), 0, 1);
        vec4f tr_col = clamp(base_colour, 0, 1);
        vec4f br_col = clamp(base_colour * (1 - shade), 0, 1);
        vec4f bl_col = clamp(base_colour, 0, 1);

        tl.colour = tl_col;
        tr.colour = tr_col;
        br.colour = br_col;
        bl.colour = bl_col;

        vertices.push_back(tl);
        vertices.push_back(bl);
        vertices.push_back(tr);

        vertices.push_back(tr);
        vertices.push_back(bl);
        vertices.push_back(br);
    }

    window.render(vertices, &sprite_sheet);

    next_renderables.clear();
}
