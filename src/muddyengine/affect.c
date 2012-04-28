
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *	           Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#include <muddyengine/affect.h>
#include <muddyengine/engine.h>
#include <muddyengine/log.h>
#include <muddyengine/character.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/object.h>
#include <muddyengine/room.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/lookup.h>

const Lookup apply_types[] = {
	{APPLY_NONE, "none"},
	{APPLY_STR, "strength"},
	{APPLY_DEX, "dexterity"},
	{APPLY_INT, "intelligence"},
	{APPLY_WIS, "wisdom"},
	{APPLY_CON, "constitution"},
	{APPLY_SEX, "sex"},
	{APPLY_LEVEL, "level"},
	{APPLY_AGE, "age"},
	{APPLY_WEIGHT, "weight"},
	{APPLY_MANA, "mana"},
	{APPLY_HIT, "hit"},
	{APPLY_MOVE, "move"},
	{APPLY_GOLD, "gold"},
	{APPLY_EXP, "experience"},
	{APPLY_HITROLL, "hitroll"},
	{APPLY_DAMROLL, "damroll"},
	{APPLY_RESIST, "resistence"},
	{APPLY_LUCK, "luck"},
	{APPLY_SIZE, "size"},
	{APPLY_ALIGN, "align"},
	{APPLY_SKILL, "skill"},
	{APPLY_SPELL_AFFECT, "spell_affect"},
	{0, 0}
};

const Lookup where_types[] = {
	{TO_AFFECTS, "affects"},
	{TO_OBJECT, "object"},
	{TO_RESIST, "resistencies"},
	{TO_WEAPON, "weapon"},
	{0, 0}
};
const Lookup affect_flags[] = {
	{AFF_BLIND, "blindness"},
	{AFF_INVISIBLE, "invisibility"},
	{AFF_DETECT_EVIL, "detect_evil"},
	{AFF_DETECT_INVIS, "detect_invis"},
	{AFF_DETECT_MAGIC, "detect_magic"},
	{AFF_DETECT_HIDDEN, "detect_hidden"},
	{AFF_DETECT_GOOD, "detect_good"},
	{AFF_SANCTUARY, "sactuary"},
	{AFF_FAERIE_FIRE, "faerie_fire"},
	{AFF_INFRARED, "infrared"},
	{AFF_CURSE, "curse"},
	{AFF_POISON, "poison"},
	{AFF_PROTECT_EVIL, "protect_evil"},
	{AFF_PROTECT_GOOD, "protect_good"},
	{AFF_SNEAK, "sneak"},
	{AFF_HIDE, "hide"},
	{AFF_SLEEP, "sleep"},
	{AFF_CHARM, "charm"},
	{AFF_FLYING, "flying"},
	{AFF_PASS_DOOR, "pass_door"},
	{AFF_HASTE, "haste"},
	{AFF_CALM, "calm"},
	{AFF_PLAGUE, "plague"},
	{AFF_WEAKEN, "weaken"},
	{AFF_DARK_VISION, "dark_vision"},
	{AFF_BERSERK, "berserk"},
	{AFF_REGENERATION, "regeneration"},
	{AFF_SLOW, "slow"},
	{0, 0}
};

Affect *new_affect(  )
{
	Affect *aff = ( Affect * ) alloc_mem( 1, sizeof( Affect ) );

	aff->location = APPLY_NONE;
	aff->where = TO_AFFECTS;

	return aff;
}

void destroy_affect( Affect * aff )
{
	free_mem( aff );
}

void affect_remove( Character * ch, Affect * paf )
{
	int where;
	Flag *vector;
	identifier_t type;

	if ( ch->affects == 0 )
	{
		log_bug( "no affects." );
		return;
	}
	//affect_modify(ch, paf, false);
	where = paf->where;
	vector = paf->flags;
	type = paf->type;

	UNLINK( ch->affects, Affect, paf, next );

	//affect_check(ch, where, vector);
}

void affect_remove_obj( Object * obj, Affect * paf )
{
	where_t where;
	Flag *vector;

	if ( obj->affects == 0 )
	{
		log_bug( "no affects on object." );
		return;
	}

	if ( obj->carriedBy != NULL && obj->wearLoc != WEAR_NONE )
		affect_modify( obj->carriedBy, paf, false );

	where = paf->where;
	vector = paf->flags;

	if ( paf->flags )
	{
		switch ( paf->where )
		{
			case TO_OBJECT:
				//remove_flags(obj->flags, paf->flags);
				break;
				/* case TO_WEAPON:
				   if (obj->item_type == ITEM_WEAPON)
				   REMOVE_BIT(obj->value[4], paf->flags);
				   break; */
			default:
				break;
		}
	}
	UNLINK( obj->affects, Affect, paf, next );

	if ( obj->carriedBy != NULL && obj->wearLoc != WEAR_NONE )
		affect_check( obj->carriedBy, where, vector );
}

void affect_modify( Character * ch, Affect * paf, bool fAdd )
{
	int mod = paf->modifier;

	if ( fAdd )
	{
		switch ( paf->where )
		{
			case TO_AFFECTS:
				//set_bit(ch->affectedBy, paf->flags);
				break;
			case TO_RESIST:
				//SET_BIT(ch->res_flags, paf->flags);
				break;
			default:
				break;
		}
	}
	else
	{
		switch ( paf->where )
		{
			case TO_AFFECTS:
				//REMOVE_BIT(ch->affected_by, paf->flags);
				break;
			case TO_RESIST:
				//REMOVE_BIT(ch->res_flags, paf->flags);
				break;
			default:
				break;
		}
		mod = 0 - mod;
	}

	switch ( paf->location )
	{
		default:
			log_bug( "bad location: %d on %s room %"PRId64, paf->location,
					 NAME( ch ), ch->inRoom ? ch->inRoom->id : 0 );
			return;

		case APPLY_NONE:
			break;
		case APPLY_STR:
			ch->statMods[STAT_STR] += mod;
			break;
		case APPLY_DEX:
			ch->statMods[STAT_DEX] += mod;
			break;
		case APPLY_INT:
			ch->statMods[STAT_INT] += mod;
			break;
		case APPLY_WIS:
			ch->statMods[STAT_WIS] += mod;
			break;
		case APPLY_CON:
			ch->statMods[STAT_CON] += mod;
			break;
		case APPLY_LUCK:
			ch->statMods[STAT_LUCK] += mod;
			break;
		case APPLY_SEX:
		{
			int s = ch->sex;
			s += mod;
			ch->sex = ( sex_t ) mod;
		}
			break;
		case APPLY_LEVEL:
			break;
		case APPLY_AGE:
			break;
		case APPLY_WEIGHT:
			break;
		case APPLY_MANA:
			ch->maxMana += mod;
			break;
		case APPLY_HIT:
			ch->maxHit += mod;
			break;
		case APPLY_MOVE:
			ch->maxMove += mod;
			break;
		case APPLY_GOLD:
			break;
		case APPLY_ALIGN:
			ch->alignment =
				URANGE( -MAX_ALIGN, ch->alignment + mod, MAX_ALIGN );
			break;
		case APPLY_SIZE:
			ch->size += mod;
			break;
		case APPLY_EXP:
			break;
		case APPLY_SKILL:
			/*    if (!IS_NPC(ch) && IS_VALID_SKILL(paf->type))
			   ch->pcdata->skill_mod[paf->type] += mod; */
			break;
		case APPLY_HITROLL:
			//ch->hitroll += mod;
			break;
		case APPLY_DAMROLL:
			//ch->damroll += mod;
			break;
		case APPLY_RESIST:
			break;
		case APPLY_SPELL_AFFECT:
			break;
	}

	/*if (ch->pc && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
	   && get_obj_weight(wield) > (str_app[get_curr_stat(ch, STAT_STR)].wield * 10)
	   && !IS_WEAPON_STAT(wield, WEAPON_NODISARM))
	   {
	   static int depth;

	   if (depth == 0)
	   {
	   depth++;
	   act("You drop $p.", ch, wield, NULL, TO_CHAR);
	   act("$n drops $p.", ch, wield, NULL, TO_ROOM);
	   obj_from_char(wield);
	   obj_to_room(wield, ch->in_room);
	   depth--;
	   }
	   } */

	return;
}

Affect *affect_find( Affect * paf, identifier_t sn )
{

	for ( Affect * paf_find = paf; paf_find; paf_find = paf_find->next )
	{
		if ( paf_find->type == sn )
			return paf_find;
	}

	return 0;
}

void affect_check( Character * ch, where_t where, Flag * vector )
{
	Affect *paf;
	Object *obj;

	if ( where == TO_OBJECT || where == TO_WEAPON || vector == 0 )
		return;

	for ( paf = ch->affects; paf; paf = paf->next )
	{
		if ( paf->where == where && flags_set( paf->flags, vector ) )
		{
			switch ( where )
			{
				case TO_AFFECTS:
					//SET_BIT(ch->affected_by, vector);
					break;

				case TO_RESIST:
					//SET_BIT(ch->res_flags, vector);
					break;

				default:
					break;
			}
			return;
		}
	}

	for ( obj = ch->carrying; obj; obj = obj->next_content )
	{
		for ( Affect * aff = obj->affects; aff; aff = aff->next )
		{
			if ( paf->where == where && flags_set( paf->flags, vector ) )
			{
				switch ( where )
				{
					case TO_AFFECTS:
						//SET_BIT(ch->affected_by, vector);
						break;

					case TO_RESIST:
						//SET_BIT(ch->res_flags, vector);
						break;

					default:
						break;
				}
				return;
			}
		}

	}
}
void affect_to_char( Character * ch, Affect * paf )
{
	LINK( ch->affects, paf, next );

	affect_modify( ch, paf, true );

	return;
}

void affect_to_obj( Object * obj, Affect * paf )
{
	LINK( obj->affects, paf, next );

	if ( paf->flags )
	{
		switch ( paf->where )
		{
			case TO_OBJECT:
				//SET_BIT(obj->extra_flags, paf->flags);
				break;
			case TO_WEAPON:

				break;
			default:
				break;
		}

		return;
	}
}

bool is_affected( Character * ch, identifier_t sn )
{
	if ( !ch )
		return false;

	return affect_find( ch->affects, sn ) != NULL;
}

Affect *load_affect_by_id( identifier_t id )
{
	char buf[400];
	sqlite3_stmt *stmt;

	int len = sprintf( buf, "select * from affect where affectId=%"PRId64, id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare sql statement" );
		return 0;
	}

	Affect *paf = 0;

	if ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		paf = new_affect(  );
		paf->id = id;

		int count = sqlite3_column_count( stmt );

		for ( int i = 0; i < count; i++ )
		{
			const char *colname = sqlite3_column_name( stmt, i );

			if ( !str_cmp( colname, "affectId" ) )
			{
				if ( id != sqlite3_column_int( stmt, i ) )
					log_error( "sql statement did not return correct affect" );
			}
			else if ( !str_cmp( colname, "type" ) )
			{
				paf->type = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "level" ) )
			{
				paf->level = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "duration" ) )
			{
				paf->duration = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "modifier" ) )
			{
				paf->modifier = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "flags" ) )
			{
				parse_flags( paf->flags,
							 sqlite3_column_str( stmt, i ), affect_flags );
			}
			else if ( !str_cmp( colname, "whereTo" ) )
			{
				paf->where = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "location" ) )
			{
				paf->location = sqlite3_column_int( stmt, i );
			}
			else
			{
				log_warn( "unknown coloumn for affect '%s'", colname );
			}
		}
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "unable to finalize statement" );
	}

	return paf;
}

int save_affect( Affect * paf )
{
	char buf[OUT_SIZ * 2];

	struct dbvalues affvalues[] = {
		{"type", &paf->type, SQLITE_INTEGER},
		{"duration", &paf->duration, SQLITE_INTEGER},
		{"modifier", &paf->modifier, SQLITE_INTEGER},
		{"level", &paf->level, SQLITE_INTEGER},
		{"flags", &paf->flags, DBTYPE_FLAG, affect_flags},
		{"whereTo", &paf->where, SQLITE_INTEGER},
		{"location", &paf->location, SQLITE_INTEGER},
		{0}
	};

	if ( paf->id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( affvalues, names, values );

		sprintf( buf, "insert into affect (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert affect" );
			return 0;
		}

		paf->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( affvalues, values );

		sprintf( buf, "update affect set %s where affectId=%"PRId64, values,
				 paf->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update character" );
			return 0;
		}
	}

	return 1;
}
