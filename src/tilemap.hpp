#ifndef TILEMAP_HPP_INCLUDED
#define TILEMAP_HPP_INCLUDED

#include <vector>
#include <vec/vec.hpp>
#include <map>
#include <optional>
#include "sprite_renderer.hpp"
#include "random.hpp"
#include <networking/serialisable_fwd.hpp>

namespace ai_info
{
    enum type
    {
        ACTIVE,
        NONE,
    };
}

namespace tiles
{
    enum type
    {
        BASE,
        WATER,
        DIRT,
        GRASS,
        TREE_1,
        TREE_2,
        TREE_DENSE,
        TREE_ROUND,
        CACTUS,
        DENSE_CACTUS,
        VINE,
        SHRUB,
        ROCKS,
        BRAMBLE,
        CIVILIAN,
        SOLDIER,

        SOLDIER_BASIC,
        SOLDIER_SPEAR,
        SOLDIER_BASIC_SHIELD,
        SOLDIER_ADVANCED,
        SOLDIER_ADVANCED_SPEAR,
        SOLDIER_TOUGH,
        SOLDIER_BEST,

        GROUND_BUG,
        FLYING_BUG,
        ARMOURED_BUG,
        SCORPION,
        SMALL_PINCHY,
        LAND_ANIMAL,
        SEA_ANIMAL,
        CROCODILE,
        FACE_MALE,
        FACE_WOMAN,
        THIN_DOOR_CLOSED,
        THIN_DOOR_OPEN,
        DOOR_CLOSED,
        DOOR_OPEN,
        GRAVE,
        WOOD_FENCE_FULL,
        WOOD_FENCE_HALF,
        TILING_WALL,
        CULTIVATION,

        //effects
        EFFECT_1,       //swipe
        EFFECT_2,       //curved swipe
        EFFECT_3,       //slash claws
        EFFECT_4,
        EFFECT_5,       //fire
        EFFECT_6,       //fireball
        EFFECT_7,       //
        EFFECT_8,       //stationary fire
        EFFECT_9,
        EFFECT_10,      //snow?
        EFFECT_11,      //snow?
        EFFECT_12,      //snow?
        EFFECT_13,      //snow?

        //medieval houses in increasing height
        HOUSE_1,
        HOUSE_2,
        HOUSE_3,
        HOUSE_4,

        TENT,
        FANCY_TENT,
        CAPITAL_TENT, //not sure, big fancy thing

        TOWER_THIN,
        TOWER_MEDIUM,
        TOWER_THICK,

        //variations on the same style
        CASTLE_1,
        CASTLE_2,

        PYRAMID,
        CHURCH,
    };
}

namespace level_info
{
    enum types
    {
        BARREN, ///dirt, some grass
        DESERT,
        GRASS
    };
}

std::map<tiles::type, std::vector<vec2i>>& get_locations();
sprite_handle get_sprite_handle_of(random_state& rng, tiles::type type);
vec4f get_colour_of(tiles::type type, level_info::types level_type);

struct tilemap : serialisable, free_function
{
    std::optional<entt::entity> selected;

    vec2i dim;
    // x * y, back to front rendering
    std::vector<std::vector<entt::entity>> all_entities;

    void create(vec2i dim);
    void add(entt::entity en, vec2i pos);
    void remove(entt::entity en, vec2i pos);
    void move(entt::entity en, vec2i from, vec2i to);
    void render(entt::registry& reg, render_window& win, camera& cam, sprite_renderer& renderer, vec2f mpos);

    int entities_at_position(vec2i pos);
};

#endif
