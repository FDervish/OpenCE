/* MCC skulls an MCC map's scripts turn on and off. Their status globals are
 * map-local scenario globals (mcc_scripts.c); the host reads them, and the
 * clients receive those whose effects they draw or predict. */
#ifndef MCC_SKULLS_H
#define MCC_SKULLS_H
#include "cseries.h"

enum mcc_skull
{
    /* drawn or predicted by every machine: in the campaign snapshot */
    _mcc_skull_fog,
    _mcc_skull_blind,
    _mcc_skull_eye_patch,
    _mcc_skull_recession,
    _mcc_skull_boom,
    NUMBER_OF_MCC_REPLICATED_SKULLS,

    /* the host's alone */
    _mcc_skull_iron = NUMBER_OF_MCC_REPLICATED_SKULLS,
    _mcc_skull_famine,
    _mcc_skull_mythic,
    _mcc_skull_assassins,
    _mcc_skull_black_eye,
    _mcc_skull_catch,
    _mcc_skull_anger,
    _mcc_skull_thunderstorm,
    _mcc_skull_thats_just_wrong,
    _mcc_skull_ghost,
    _mcc_skull_sputnik,
    _mcc_skull_foreign,
    _mcc_skull_malfunction,
    _mcc_skull_pinata,
    _mcc_skull_grunt_funeral,
    _mcc_skull_grunt_birthday_party,
    _mcc_skull_iwhbyd,
    _mcc_skull_streaking,
    _mcc_skull_styx,
    _mcc_skull_feather,
    _mcc_skull_bonded_pair,
    _mcc_skull_masterblaster,
    _mcc_skull_force_coop,
    NUMBER_OF_MCC_SKULLS
};

#define MCC_SKULLS_REPLICATED_MASK ((1u << NUMBER_OF_MCC_REPLICATED_SKULLS) - 1u)

/* Map loading: forget the last map's globals, then record this map's. */
void mcc_skulls_reset(void);
/* The skull whose MCC status global this is (NONE: not a skull's). */
short mcc_skull_find(char const *name);
void mcc_skulls_bind(short skull, short scenario_global);

/* Whether the skull is on. FALSE outside MCC maps, so a check costs the
 * Xbox and Custom Edition paths nothing but the call. */
boolean mcc_skull_active(short skull);

/* The replicated skulls, as bits of enum mcc_skull: what the host sends,
 * and what a client received (mcc_campaign.c). */
unsigned long mcc_skulls_replicated(void);
void mcc_skulls_receive(unsigned long mask);

/* The effects, where the engine applies them. Each is neutral (one, FALSE,
 * nothing) when the skull is off or outside MCC maps. */
/* Recession: a player's weapon spends this many times the rounds and battery. */
short mcc_skulls_ammunition_scale(boolean player_weapon);
/* Famine: the weapons a unit no player controlled drops when it dies keep
 * half their ammunition. */
void mcc_skulls_famine(long unit_index);
/* Mythic: an enemy biped's maximum body vitality is multiplied by this. */
real mcc_skulls_body_vitality_scale(long object_index);
/* Boom: an explosion's radii are multiplied by this. */
real mcc_skulls_explosion_radius_scale(void);
/* Black Eye: a player's unit has no shield recharge of its own; its melee
 * striking an enemy fills its shields. */
boolean mcc_skulls_shield_recharge_blocked(long object_index);
void mcc_skulls_melee_struck(long player_index, long victim_object_index, boolean melee, boolean damaged);
/* Iron: a solo player's death restarts the level; a co-op player's reverts
 * every player to the last checkpoint. */
boolean mcc_skulls_iron(void);
/* Each tick of the host's (or a local game's) simulation: Assassins. */
void mcc_skulls_update(void);

/* Implemented beside the interpreter's private global data (mcc_script_runtime.inl). */
long mcc_script_scenario_global_value(short scenario_global);

#endif
