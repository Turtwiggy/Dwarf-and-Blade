#include "overworld_map.hpp"
#include "entity_common.hpp"

entt::entity create_overworld_unit(entt::registry& registry, sprite_handle handle, world_transform transform)
{
    entt::entity res = registry.create();

    render_descriptor desc;
    desc.pos = transform.position;

    registry.assign<sprite_handle>(res, handle);
    registry.assign<world_transform>(res, transform);
    registry.assign<overworld_tag>(res, overworld_tag());
    registry.assign<render_descriptor>(res, desc);

    return res;
}

std::vector<float> generate_noise(random_state& rng, vec2i dim)
{
    std::vector<float> ret;
    ret.resize(dim.x() * dim.y());

    for(auto& i : ret)
    {
        i = rand_det_s(rng.rng, 0, 1);
    }

    return ret;
}

float positive_fmod(float x, float y)
{
    float result = std::remainder(std::fabs(x), (y = std::fabs(y)));

    if (std::signbit(result))
    {
        result += y;
    }

    return result;
}

float simple_sample(const std::vector<float>& data, vec2f pos, vec2i dim)
{
    vec2f tl = floor(pos);
    vec2f br = ceil(pos);

    float xfrac = (pos.x() - tl.x());
    float yfrac = (pos.y() - tl.y());

    tl.x() = positive_fmod(tl.x(), dim.x() - 1);
    tl.y() = positive_fmod(tl.y(), dim.y() - 1);

    br.x() = positive_fmod(br.x(), dim.x() - 1);
    br.y() = positive_fmod(br.y(), dim.y() - 1);

    float tl_val = data[((int)tl.y()) * dim.x() + (int)tl.x()];
    float tr_val = data[((int)tl.y()) * dim.x() + (int)br.x()];
    float bl_val = data[((int)br.y()) * dim.x() + (int)tl.x()];
    float br_val = data[((int)br.y()) * dim.x() + (int)br.x()];

    float y1 = mix(tl_val, tr_val, xfrac);
    float y2 = mix(bl_val, br_val, xfrac);

    return mix(y1, y2, yfrac);
}

struct noise_data
{
    std::vector<float> noise_1;
    std::vector<float> noise_2;
    std::vector<float> noise_3;
    std::vector<float> noise_4;
    vec2i dim;

    noise_data(random_state& rng, vec2i _dim) : dim(_dim)
    {
        noise_1 = generate_noise(rng, dim);
        noise_2 = generate_noise(rng, dim);
        noise_3 = generate_noise(rng, dim);
        noise_4 = generate_noise(rng, dim);
    }

    float sample(vec2f pos)
    {
        float sample_freq = 0.005;

        vec2f warp = {simple_sample(noise_2, pos * sample_freq, dim), simple_sample(noise_3, pos * sample_freq, dim)};
        vec2f warp2 = vec2f{simple_sample(noise_2, pos * sample_freq * 10, dim), simple_sample(noise_3, pos * sample_freq * 10, dim)};

        vec2f wpos = pos + warp * 40 + warp2 * 20;

        float density = 0;

        density += simple_sample(noise_1, wpos, dim);
        density += simple_sample(noise_1, wpos / 2.f, dim) * 2;
        density += simple_sample(noise_1, wpos / 4.f, dim) * 4;
        density += simple_sample(noise_1, wpos / 8.f, dim) * 8;

        float width = dim.x();

        float water_width = width * 0.8;
        float land_width = width - water_width;

        vec2f centre = vec2f{dim.x(), dim.y()}/2.f;

        float distance_from_centre = (pos - centre).length();

        distance_from_centre = clamp(distance_from_centre, 0.f, width/2);

        float final_density = density / (8 + 4 + 2 + 1);

        if(distance_from_centre >= land_width/2)
        {
            float water_frac = (distance_from_centre - (land_width/2)) / (water_width/2);

            float subtractive_density = simple_sample(noise_4, wpos/32.f, dim);

            float low_val = final_density - subtractive_density;

            low_val = clamp(low_val, 0.f, 1.f);

            //return low_val;

            return mix(final_density, low_val, water_frac);

            //return mix(final_density, 0.f, (distance_from_centre - land_width/2) / (water_width/2));
        }

        return final_density;
    }
};

entt::entity create_tile_from_density(entt::registry& registry, random_state& rng, noise_data& noise, vec2i pos, vec2i dim)
{
    vec2f fpos = {pos.x(), pos.y()};
    fpos = fpos / vec2f{dim.x(), dim.y()};

    float fraction = noise.sample(fpos * 100);

    sprite_handle han;
    collidable coll;

    vec4f beach = srgb_to_lin_approx(vec4f{255, 218, 180, 255}/255.f);
    vec4f grass_col = srgb_to_lin_approx(vec4f{56, 217, 115, 255} / 255.f);

    float water_level = 0.2;
    float beach_to_water = 0.25;
    float grass_to_beach = 0.3;

    if(fraction < water_level)
    {
        han = get_sprite_handle_of(rng, tiles::WATER);

        float mfrac = fraction / water_level;

        mfrac = (mfrac + 1) / 2;

        han.base_colour.w() *= mfrac;

        coll.cost = -1;
    }
    /*else
    {
        han = get_sprite_handle_of(rng, tiles::BASE);

        float mfrac = (fraction - 0.2) / 0.8;

        float mov = 0.5;

        mfrac = (mfrac + mov) / (1 + mov);

        han.base_colour.w() *= mfrac;
    }*/

    else if(fraction < beach_to_water)
    {
        vec4f water = srgb_to_lin_approx(vec4f{60, 172, 215, 255}/255.f);

        float ffrac = (fraction - water_level) / (beach_to_water - water_level);

        han = get_sprite_handle_of(rng, tiles::BASE);
        han.base_colour = beach;

        han.base_colour = mix(water, beach, ffrac);

        coll.cost = 2;

        //han.base_colour.w() *= ffrac;
    }
    else if(fraction < grass_to_beach)
    {
        float ffrac = (fraction - beach_to_water) / (grass_to_beach - beach_to_water);

        han = get_sprite_handle_of(rng, tiles::BASE);

        vec4f gcol = grass_col;
        gcol.w() *= fraction;

        han.base_colour = mix(beach, gcol, ffrac);

        coll.cost = 2;
    }
    else
    {
        han = get_sprite_handle_of(rng, tiles::BASE);

        float ffrac = (fraction - grass_to_beach) / (1 - grass_to_beach);
        float mov = 0.25;
        ffrac = (ffrac + mov) / (1 + mov);

        han.base_colour.w() *= (fraction + mov) / (mov + 1);

        coll.cost = 1;
    }

    /*else
    {
        han = get_sprite_handle_of(rng, tiles::BASE);

        han.base_colour = srgb_to_lin_approx(vec4f{122, 68, 74, 255} / 255.f);
    }*/

    render_descriptor desc;
    desc.pos = vec2f{pos.x(), pos.y()} * TILE_PIX + vec2f{TILE_PIX / 2, TILE_PIX / 2};

    entt::entity base = registry.create();

    registry.assign<sprite_handle>(base, han);
    registry.assign<render_descriptor>(base, desc);
    registry.assign<overworld_tag>(base, overworld_tag());

    return base;
}

entt::entity create_overworld(entt::registry& registry, random_state& rng, vec2i dim)
{
    entt::entity res = registry.create();

    tilemap tmap;
    tmap.create(dim);

    noise_data noise(rng, {100, 100});

    int factions = 5;

    for (int y = 0; y < dim.y(); y++)
    {
        for (int x = 0; x < dim.x(); x++)
        {
            //sprite_handle handle = get_sprite_handle_of(rng, tiles::BASE);

            //sprite_handle handle = get_tile_from_density(rng, );

            //handle.base_colour = clamp(rand_det_s(rng.rng, 0.5, 1.5) * handle.base_colour * 0.2, 0, 1);
            //handle.base_colour *= noise.sample({x, y});

            //handle.base_colour.w() = 1;

            auto base = create_tile_from_density(registry, rng, noise, {x, y}, dim);

            tmap.add(base, {x, y});
        }
    }

    registry.assign<tilemap>(res, tmap);
    registry.assign<overworld_tag>(res, overworld_tag());

    return res;
}

entt::entity create_dummy_army_at(entt::registry& registry, random_state& rng, vec2i pos, int team_id)
{
    world_transform transform;
    transform.position = vec2f{pos.x(), pos.y()} * TILE_PIX + vec2f{TILE_PIX/2, TILE_PIX/2};

    team base_team;
    base_team.type = team::NUMERIC;
    base_team.t = team_id;

    sprite_handle handle = get_sprite_handle_of(rng, tiles::SOLDIER_SPEAR);

    handle.base_colour *= team::colours.at(team_id);

    entt::entity army = create_unit_group(registry, base_team, handle, transform);

    int unit_count = 10;

    for(int i=0; i < unit_count; i++)
    {
        world_transform trans;
        trans.position = vec2f{pos.x(), pos.y()} * TILE_PIX + vec2f{TILE_PIX/2, TILE_PIX/2};

        damageable damage;

        entt::entity en = create_basic_unit(registry, base_team, get_sprite_handle_of(rng, tiles::SOLDIER_SPEAR), trans, damage);

        unit_group& ugroup = registry.get<unit_group>(army);

        ugroup.entities.push_back(en);
    }

    return army;
}

void debug_overworld(entt::registry& registry, entt::entity en, random_state& rng)
{
    tilemap& tmap = registry.get<tilemap>(en);

    vec2i half = tmap.dim/2;

    entt::entity army1 = create_dummy_army_at(registry, rng, half, 0);
    entt::entity army2 = create_dummy_army_at(registry, rng, {half.x()+1, half.y()}, 1);

    tmap.add(army1, half);
    tmap.add(army2, {half.x()+1, half.y()});
}
