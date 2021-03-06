#include "battle_map.hpp"

#include "battle_map_ai.hpp"

entt::entity battle_map::create_battle(entt::registry& registry, random_state& rng, vec2i dim, level_info::types type)
{
    entt::entity res = registry.create();

    tilemap tmap;
    tmap.create(dim);

    //Create background tiles
    {
        for (int y = 0; y < dim.y(); y++)
        {
            for (int x = 0; x < dim.x(); x++)
            {
                sprite_handle handle = get_sprite_handle_of(rng, tiles::BASE);
                //handle.base_colour = clamp(rand_det_s(rng.rng, 0.7, 1.3) * handle.base_colour * 0.1, 0, 1);
                //handle.base_colour.w() = 1;

                tilemap_position tmap_pos;
                tmap_pos.pos = vec2i{ x, y };

                render_descriptor desc;
                desc.pos = camera::tile_to_world(vec2f{ tmap_pos.pos.x(), tmap_pos.pos.y() });
                desc.depress_on_hover = true;
                //desc.angle = rand_det_s(rng.rng, 0.f, 2 * M_PI);

                entt::entity base = registry.create();

                registry.assign<sprite_handle>(base, handle);
                registry.assign<tilemap_position>(base, tmap_pos);
                registry.assign<render_descriptor>(base, desc);
                registry.assign<battle_tag>(base, battle_tag());
                //registry.assign<mouse_interactable>(base, mouse_interactable());

                tmap.add(base, { x, y });
            }
        }
    }

    //Decor
    std::vector<tiles::type> decoration =
    {
        tiles::GRASS
    };
    //distribute_entities(registry, tmap, rng, dim, type, 20, decoration, 0);

    //Scenery
    std::vector<tiles::type> scenery =
    {
        tiles::TREE_1, tiles::TREE_2, tiles::TREE_ROUND, tiles::ROCKS, tiles::BRAMBLE
    };
    //distribute_entities(registry, tmap, rng, dim, type, 1, scenery, -1);

    registry.assign<tilemap>(res, tmap);
    registry.assign<battle_tag>(res, battle_tag());
    registry.assign<battle_map_state>(res, battle_map_state());

    return res;
}

void battle_map::distribute_entities(entt::registry& registry, tilemap& tmap, random_state& rng, vec2i dim, level_info::types type, int percentage, const std::vector<tiles::type>& scenery, float path_cost)
{
    for (int y = 0; y < dim.y(); y++)
    {
        for (int x = 0; x < dim.x(); x++)
        {
            if (!(rand_det_s(rng.rng, 0, 100) < percentage))
                continue;

            int random_element = rand_det_s(rng.rng, 0.f, scenery.size());

            random_element = clamp(random_element, 0, (int)scenery.size() - 1);

            tiles::type type = scenery[random_element];

            sprite_handle handle = get_sprite_handle_of(rng, type);
            handle.base_colour.w() = 1;

            tilemap_position trans;
            trans.pos = vec2i{ x, y };

            collidable coll;
            coll.cost = path_cost;

            auto base = create_scenery(registry, handle, trans, coll);

            tmap.add(base, { x, y });
        }
    }
}


entt::entity battle_map::create_battle_unit(entt::registry& registry, sprite_handle handle, tilemap_position transform, team t)
{
    entt::entity res = registry.create();

    render_descriptor desc;
    desc.pos = camera::tile_to_world(vec2f{ transform.pos.x(), transform.pos.y() });
    desc.depress_on_hover = true;

    registry.assign<sprite_handle>(res, handle);
    registry.assign<render_descriptor>(res, desc);
    registry.assign<tilemap_position>(res, transform);
    registry.assign<mouse_interactable>(res, mouse_interactable());

    damageable d;
    d.max_hp = 10;
    d.cur_hp = d.max_hp;
    registry.assign<damageable>(res, d);

    registry.assign<team>(res, t);

    battle_unit_info info;
    info.damage = 10;
    registry.assign<battle_unit_info>(res, info);

    registry.assign<battle_tag>(res, battle_tag());

    return res;
}

entt::entity create_battle_unit_at(entt::registry& registry, random_state& rng, vec2i pos, int team_id)
{
    tilemap_position transform;
    transform.pos = pos;

    team base_team;
    base_team.type = team::NUMERIC;
    base_team.t = team_id;

    sprite_handle handle = get_sprite_handle_of(rng, tiles::SOLDIER_SPEAR);
    handle.base_colour *= team::colours.at(team_id);

    entt::entity unit = battle_map::create_battle_unit(registry, handle, transform, base_team);

    return unit;
}


entt::entity battle_map::create_obstacle(entt::registry& registry, sprite_handle handle, tilemap_position transform, int path_cost)
{
    entt::entity res = registry.create();

    render_descriptor desc;
    desc.pos = camera::tile_to_world(vec2f{ transform.pos.x(), transform.pos.y() });
    desc.depress_on_hover = true;

    registry.assign<sprite_handle>(res, handle);
    registry.assign<render_descriptor>(res, desc);
    registry.assign<mouse_interactable>(res, mouse_interactable());

    collidable c;
    c.cost = path_cost;
    registry.assign<collidable>(res, c);
    // registry.assign<battle_unit>(res, battle_unit());

    return res;
}

entt::entity create_obstacle_at(entt::registry& registry, random_state& rng, vec2i pos, tilemap& map, sprite_handle handle, int path_cost)
{
    tilemap_position transform;
    transform.pos = pos;

    entt::entity obstacle = battle_map::create_obstacle(registry, handle, transform, path_cost);

    map.add(obstacle, pos);
}


void battle_map::battle_map_state::update_ai(entt::registry& registry, entt::entity& map, float delta_time, random_state& rng)
{
    auto view = registry.view<battle_tag, tilemap_position, render_descriptor, sprite_handle, wandering_ai> ();

    tilemap& tmap = registry.get<tilemap>(map);

    for (auto ent : view)
    {
        auto& ai = view.get<wandering_ai>(ent);
        auto& desc = view.get<render_descriptor>(ent);

        ai.tick_ai(registry, delta_time, tmap, ent, rng);
        ai.tick_animation(delta_time, desc);
    }
}

void battle_map::battle_map_state::battle_editor(entt::registry& registry, entt::entity& map, random_state& rng, render_window& win, camera& cam, vec2f mpos)
{
    battle_map_state& state = registry.get<battle_map::battle_map_state>(map);
    tilemap& tmap = registry.get<tilemap>(map);

    bool mouse_clicked = ImGui::IsMouseClicked(0) && !ImGui::IsAnyWindowHovered() && !ImGui::GetIO().WantCaptureMouse;
    bool mouse_hovering = !ImGui::IsAnyWindowHovered();

    if (mouse_clicked)
    {
        vec2f clamped_tile = clamp(
            cam.screen_to_tile(win, mpos),
            vec2f{ 0, 0 },
            vec2f{ tmap.dim.x() - 1, tmap.dim.y() - 1 });
        vec2i clamped_i_tile = vec2i{ (int)clamped_tile.x(), (int)clamped_tile.y() };

        printf(" clicked tile: %d %d \n", clamped_i_tile.x(), clamped_i_tile.y());

        if (state.current_item == combobox_items::OBSTACLES)
        {
            sprite_handle handle = get_sprite_handle_of(rng, tiles::type::CACTUS);
            create_obstacle_at(registry, rng, clamped_i_tile, tmap, handle, -1);
        }

        if (state.current_item == combobox_items::ENEMY_UNITS)
        {
            vec2i half = tmap.dim / 2;

            //add enemy
            vec2i start_pos = clamped_i_tile;
            vec2i dest_pos = tmap.dim - 1;

            entt::entity enemy_unit = create_battle_unit_at(registry, rng, start_pos, 1);

            wandering_ai ai;
            ai.destination_xy = dest_pos;
            registry.assign<wandering_ai>(enemy_unit, ai);

            collidable coll;
            coll.cost = 150;
            registry.assign<collidable>(enemy_unit, coll);

            tmap.add(enemy_unit, start_pos);
        }

        if (state.current_item == combobox_items::PLAYER_UNITS)
        {
            vec2i half = tmap.dim / 2;

            //add enemy
            vec2i start_pos = clamped_i_tile;
            vec2i dest_pos = tmap.dim - 1;

            entt::entity player_unit = create_battle_unit_at(registry, rng, start_pos, 0);

            wandering_ai ai;
            ai.destination_xy = dest_pos;
            registry.assign<wandering_ai>(player_unit, ai);

            collidable coll;
            coll.cost = 150;
            registry.assign<collidable>(player_unit, coll);

            tmap.add(player_unit, start_pos);
        }
    }

    std::string battle_editor_label = "Battle Editor ##" + std::to_string((int)map);
    ImGui::Begin(battle_editor_label.c_str());

    if (ImGui::BeginCombo("##combo", state.current_item_str.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < state.items.size(); n++)
        {
            bool is_selected = (state.current_item_str == state.items[n]); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(state.items[n].c_str(), is_selected))
            {
                printf("Selected! \n");
                state.current_item_str = state.items[n];
                state.current_item = combobox_str_to_item(state.current_item_str);
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::EndCombo();
    }
    ImGui::End();
}

void battle_map::battle_map_state::unit_editor(entt::registry& registry, entt::entity& map, random_state& rng, render_window& win, camera& cam, vec2f mpos)
{
    battle_map_state& state = registry.get<battle_map::battle_map_state>(map);
    tilemap& tmap = registry.get<tilemap>(map);

    //iterate over everything tilemap
    //check if entities have required components

    std::string unit_editor_label = "Unit Editor ##" + std::to_string((int)map);
    ImGui::Begin(unit_editor_label.c_str());

    auto view = registry.view<battle_tag, battle_unit_info, render_descriptor, sprite_handle, damageable, wandering_ai, tilemap_position>();

    int id = 0;
    for (auto ent : view)
    {
        auto& ai = view.get<wandering_ai>(ent);
        auto& health = view.get<damageable>(ent);
        auto& tmap_pos = view.get<tilemap_position>(ent);
        auto& info = view.get<battle_unit_info>(ent);

        if (health.cur_hp <= 0)
            continue;

        std::string name = "Unit name: STEVE";
        ImGui::Text(name.c_str());

        std::string hp = "Unit hp: " + std::to_string(health.cur_hp);
        ImGui::Text(hp.c_str());

        std::string attack = "Unit attack: " + std::to_string(info.damage);
        ImGui::Text(attack.c_str());

        std::string position = "Unit position (x): "
            + std::to_string(tmap_pos.pos.x())
            + " (y): " + std::to_string(tmap_pos.pos.y());
        ImGui::Text(position.c_str());

        std::string kills = "Unit kills: " + std::to_string(info.kills);
        ImGui::Text(kills.c_str());

        std::string button_label = "Destroy unit!##" + std::to_string(id);
        if (ImGui::Button(button_label.c_str()))
        {
            tmap.remove(ent, tmap_pos.pos);
            health.damage_amount(health.max_hp);
        };
        id += 1;
    }

    ImGui::End();


}

void battle_map::battle_map_state::debug_combat(entt::registry& registry, entt::entity& map, random_state& rng, render_window& win, camera& cam, vec2f mpos)
{
    battle_map_state& state = registry.get<battle_map::battle_map_state>(map);
    tilemap& tmap = registry.get<tilemap>(map);

    std::string gamemode_editor_label = "Gamemode Editor ##" + std::to_string((int)map);
    ImGui::Begin(gamemode_editor_label.c_str());

    if (ImGui::BeginCombo("##combo", state.current_gamemode_str.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < state.gamemodes.size(); n++)
        {
            bool is_selected = (state.current_gamemode_str == state.gamemodes[n]); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(state.gamemodes[n].c_str(), is_selected))
            {
                printf("Selected! \n");
                state.current_gamemode_str = state.gamemodes[n];
                state.current_gamemode = combobox_str_to_gamemode(state.current_gamemode_str);
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::EndCombo();
    }

    ImGui::End();
}


