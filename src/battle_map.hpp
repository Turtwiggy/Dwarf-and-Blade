#pragma once

#include <stdint.h>

#include "sprite_renderer.hpp"
#include "entity_common.hpp"
#include "tilemap.hpp"
#include "random.hpp"

#include <entt/entt.hpp>
#include <imgui/imgui.h>

namespace battle_map {

    struct battle_unit_info
    {
        float damage = 1;

        int kills = 0;
    };

    struct battle_map_state
    {

        enum combobox_items
        {
            ENEMY_UNITS,
            PLAYER_UNITS,
            OBSTACLES
        };
        combobox_items convert_string_to_item(std::string str)
        {
            if (str == items[0]) return combobox_items::ENEMY_UNITS;
            if (str == items[1]) return combobox_items::PLAYER_UNITS;
            if (str == items[2]) return combobox_items::OBSTACLES;
            return combobox_items::ENEMY_UNITS;
        }
        std::vector<std::string> items = {
            "ENEMY_UNITS", "PLAYER_UNITS", "OBSTACLES"
        };
        std::string current_item_str = "ENEMY_UNITS";
        combobox_items current_item = combobox_items::ENEMY_UNITS;

        void update_ai(entt::registry& registry, entt::entity& map, float delta_time, random_state& rng);
        void debug_combat(entt::registry& registry, entt::entity& map, random_state& rng, render_window& win, camera& cam, vec2f mpos);
    
        //void reset_tilemap_colours(tilemap& tmap, entt::registry& registry);    
    }; 
    
    entt::entity create_battle(entt::registry& registry, random_state& rng, vec2i dim, level_info::types type);
    void distribute_entities(entt::registry& registry, tilemap& tmap, random_state& rng, vec2i dim, level_info::types type, int percentage, const std::vector<tiles::type>& scenery, float path_cost);
    entt::entity create_battle_unit( entt::registry& registry, sprite_handle handle, tilemap_position transform, team t);
    entt::entity create_obstacle(entt::registry& registry, sprite_handle handle, tilemap_position transform, int path_cost);

}