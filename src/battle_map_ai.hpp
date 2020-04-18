#pragma once

#include "sprite_renderer.hpp"
#include "entity_common.hpp"
#include "tilemap.hpp"
#include "random.hpp"
#include "pathfinding.hpp"

struct ai_destination_tag
{
    vec2i destination;
};

struct wandering_ai
{
    //pathfinding
    vec2i current_xy;
    vec2i destination_xy;
    float time_between_move_tiles = 1.;
    float time_left_before_move_tiles = time_between_move_tiles;

    //animation
    float time_between_animation_updates = 0.3f;
    float time_left_before_animation_update = time_between_animation_updates;
    bool on_max_scale = true;
    vec2f max_scale = { 1., 1. };
    vec2f min_scale = { 0.95f, 0.9f };

    void tick_ai
    (
        entt::registry&     registry, 
        float               delta_time, 
        render_descriptor&  desc, 
        sprite_handle&      handle, 
        random_state&       rng,
        tilemap&            tmap, 
        entt::entity        en 
    );

    void move_ai
    (
        entt::registry&     registry,
        render_descriptor&  desc,
        sprite_handle&      handle,
        random_state&       rng,
        tilemap&            tmap,
        entt::entity        en
    );

    void tick_animation
    (
        entt::registry&     registry,
        float               delta_time,
        render_descriptor&  desc,
        sprite_handle&      handle,
        random_state&       rng,
        tilemap&            tmap,
        entt::entity        en
    );

    void wandering_ai::update_animation
    (
        entt::registry&     registry,
        float               delta_time,
        render_descriptor&  desc,
        sprite_handle&      handle,
        random_state&       rng,
        tilemap&            tmap,
        entt::entity        e
    );
   
    void reset_tilemap_colours(tilemap& tmap, entt::registry& registry);
    void show_path_colours_on_tilemap(tilemap& tmap, entt::registry& registry, std::vector<vec2i> points, vec2i destination);
};