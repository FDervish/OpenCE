/* MCC skull status for MCC maps. Only the host runs a map's scripts, so its
 * skull status globals are the truth; a client knows the skulls the campaign
 * snapshot carries (the replicated ones) and treats the rest as off, since
 * their effects are the host's to apply. */
#include "cseries.h"
#include "game/game.h"
#include "game/game_allegiance.h"
#include "game/players.h"
#include "items/weapons.h"
#include "items/weapon_definitions.h"
#include "objects/objects.h"
#include "units/units.h"
#include "mcc_cache.h"
#include "mcc_skulls.h"

/* The status globals' MCC names, by enum mcc_skull. */
static char const *const mcc_skull_names[NUMBER_OF_MCC_SKULLS] = {
    "debug_ice_cream_flavor_status_fog",
    "debug_ice_cream_flavor_status_blind",
    "debug_ice_cream_flavor_status_eye_patch",
    "debug_ice_cream_flavor_status_recession",
    "debug_ice_cream_flavor_status_boom",
    "debug_ice_cream_flavor_status_iron",
    "debug_ice_cream_flavor_status_famine",
    "debug_ice_cream_flavor_status_mythic",
    "debug_ice_cream_flavor_status_assassins",
    "debug_ice_cream_flavor_status_black_eye",
    "debug_ice_cream_flavor_status_catch",
    "debug_ice_cream_flavor_status_anger",
    "debug_ice_cream_flavor_status_thunderstorm",
    "debug_ice_cream_flavor_status_thats_just_wrong",
    "debug_ice_cream_flavor_status_ghost",
    "debug_ice_cream_flavor_status_sputnik",
    "debug_ice_cream_flavor_status_foreign",
    "debug_ice_cream_flavor_status_malfunction",
    "debug_ice_cream_flavor_status_pinata",
    "debug_ice_cream_flavor_status_grunt_funeral",
    "debug_ice_cream_flavor_status_grunt_birthday_party",
    "debug_ice_cream_flavor_status_i_would_have_been_your_daddy",
    "debug_ice_cream_flavor_status_streaking",
    "debug_ice_cream_flavor_status_styx",
    "debug_ice_cream_flavor_status_feather",
    "debug_ice_cream_flavor_status_bonded_pair",
    "debug_ice_cream_flavor_status_masterblaster",
    "debug_ice_cream_flavor_force_coop"
};
static short mcc_skull_globals[NUMBER_OF_MCC_SKULLS];
static unsigned long mcc_skulls_received;

void mcc_skulls_reset(void)
{
    short skull;

    for (skull = 0; skull < NUMBER_OF_MCC_SKULLS; skull++)
        mcc_skull_globals[skull] = NONE;
    mcc_skulls_received = 0;
}

short mcc_skull_find(char const *name)
{
    short skull;

    for (skull = 0; name && skull < NUMBER_OF_MCC_SKULLS; skull++)
    {
        if (!csstrcasecmp(name, mcc_skull_names[skull]))
            return skull;
    }
    return NONE;
}

void mcc_skulls_bind(short skull, short scenario_global)
{
    if (skull >= 0 && skull < NUMBER_OF_MCC_SKULLS)
        mcc_skull_globals[skull] = scenario_global;
}

boolean mcc_skull_active(short skull)
{
    if (skull < 0 || skull >= NUMBER_OF_MCC_SKULLS || !mcc_cache_tags_loaded())
        return FALSE;
    if (game_connection() == _game_connection_network_client)
        return skull < NUMBER_OF_MCC_REPLICATED_SKULLS && (mcc_skulls_received & (1u << skull)) != 0;
    return mcc_skull_globals[skull] != NONE &&
        (mcc_script_scenario_global_value(mcc_skull_globals[skull]) & 0xFF) != 0;
}

unsigned long mcc_skulls_replicated(void)
{
    unsigned long mask = 0;
    short skull;

    for (skull = 0; skull < NUMBER_OF_MCC_REPLICATED_SKULLS; skull++)
    {
        if (mcc_skull_active(skull))
            mask |= 1u << skull;
    }
    return mask;
}

void mcc_skulls_receive(unsigned long mask)
{
    mcc_skulls_received = mask & MCC_SKULLS_REPLICATED_MASK;
}

short mcc_skulls_ammunition_scale(boolean player_weapon)
{
    return player_weapon && mcc_skull_active(_mcc_skull_recession) ? 2 : 1;
}

/* A battery weapon, by weapon_set_current_amount's rule: no magazine, or a
 * trigger whose rounds age it. */
static boolean mcc_skulls_weapon_uses_age(struct weapon_definition const *definition)
{
    long trigger, count = definition->weapon.triggers.count;

    if (definition->weapon.magazines.count == 0)
        return TRUE;
    if (count > (long)NUMBEROF(((struct weapon_datum *)0)->weapon.triggers))
        count = (long)NUMBEROF(((struct weapon_datum *)0)->weapon.triggers);
    for (trigger = 0; trigger < count; trigger++)
    {
        if (((struct weapon_trigger_definition const *)definition->weapon.triggers.address)[trigger].age_generated_per_round > 0.0f)
            return TRUE;
    }
    return FALSE;
}

/* As MCC does it: a magazine's rounds by integer division, and a battery's
 * use halfway to empty. The host's alone: a client's skull is off. */
void mcc_skulls_famine(long unit_index)
{
    struct unit_datum *unit;
    short slot, magazine;

    if (!mcc_skull_active(_mcc_skull_famine) || !(unit = unit_try_and_get(unit_index)))
        return;
    for (slot = 0; slot < MAXIMUM_WEAPONS_PER_UNIT; slot++)
    {
        long weapon_index = unit->unit.weapon_object_indices[slot];
        struct weapon_datum *weapon = weapon_index != NONE ? weapon_try_and_get(weapon_index) : NULL;
        struct weapon_definition const *definition;

        if (!weapon)
            continue;
        definition = weapon_definition_get(weapon->definition_index);
        for (magazine = 0; magazine < (short)NUMBEROF(weapon->weapon.magazines) &&
            magazine < definition->weapon.magazines.count; magazine++)
        {
            weapon->weapon.magazines[magazine].rounds_total /= 2;
            weapon->weapon.magazines[magazine].rounds_loaded /= 2;
        }
        if (mcc_skulls_weapon_uses_age(definition))
            weapon->weapon.age = PIN((weapon->weapon.age + 1.0f) * 0.5f, 0.0f, 1.0f);
    }
}

static boolean mcc_skulls_enemy(long object_index, long object_mask)
{
    struct object_datum *object = object_try_and_get(object_index);

    return object && TEST_FLAG(object_mask, object->object.type) &&
        game_team_is_enemy(_game_team_player, object->object.owner_team_index);
}

real mcc_skulls_body_vitality_scale(long object_index)
{
    return mcc_skull_active(_mcc_skull_mythic) && mcc_skulls_enemy(object_index, _object_mask_biped) ? 2.0f : 1.0f;
}

real mcc_skulls_explosion_radius_scale(void)
{
    return mcc_skull_active(_mcc_skull_boom) ? 2.0f : 1.0f;
}

boolean mcc_skulls_shield_recharge_blocked(long object_index)
{
    struct unit_datum *unit;

    if (!mcc_skull_active(_mcc_skull_black_eye))
        return FALSE;
    unit = unit_try_and_get(object_index);
    return unit && unit->unit.player_index != NONE;
}

/* MCC fills the shields when a player's melee damages an enemy, without
 * lowering an overshield. The host deals the damage (clients report it). */
void mcc_skulls_melee_struck(long player_index, long victim_object_index, boolean melee, boolean damaged)
{
    struct player_datum *player;
    struct object_datum *object;

    if (!melee || !damaged || player_index == NONE || !mcc_skull_active(_mcc_skull_black_eye) ||
        !mcc_skulls_enemy(victim_object_index, _object_mask_unit))
        return;
    player = player_try_and_get(player_index);
    object = player && player->unit_index != NONE ? object_try_and_get(player->unit_index) : NULL;
    if (object && object->object.shield_vitality < 1.0f && object->object.maximum_shield_vitality > 0.0f)
        object->object.shield_vitality = 1.0f;
}

boolean mcc_skulls_iron(void)
{
    return mcc_skull_active(_mcc_skull_iron);
}

/* MCC cloaks every enemy unit an actor controls, every tick. The camouflage
 * reaches clients with each actor's state (network_actors.c). */
void mcc_skulls_update(void)
{
    struct object_iterator iterator;

    if (!mcc_skull_active(_mcc_skull_assassins))
        return;
    object_iterator_new(&iterator, _object_mask_unit, 0);
    while (object_iterator_next(&iterator))
    {
        struct unit_datum *unit = unit_try_and_get(iterator.index);

        if (unit && (unit->unit.actor_index != NONE || unit->unit.swarm_actor_index != NONE) &&
            unit->unit.player_index == NONE && !TEST_FLAG(unit->object.damage_flags, _object_dead_bit) &&
            game_team_is_enemy(_game_team_player, unit->object.owner_team_index) &&
            !TEST_FLAG(unit->unit.flags, _unit_active_camouflaged_bit))
        {
            SET_FLAG(unit->unit.flags, _unit_active_camouflaged_bit, TRUE);
            unit->unit.active_camouflage = 1.0f;
        }
    }
}
