
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

#include <muddyengine/skill.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/spellfun.h>
#include <muddyengine/engine.h>
#include <muddyengine/log.h>
#include <muddyengine/class.h>
#include <muddyengine/character.h>
#include <muddyengine/lookup.h>
#include <muddyengine/affect.h>
#include <muddyengine/util.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

Skill *skill_table = 0;

int max_skill = 0;

#define DEFINE_GSNS
#include <muddyengine/gsn.h>
#undef DEFINE_GSNS

const struct gsn_type gsn_table[] = {
#define IN_GSN_TABLE
#include <muddyengine/gsn.h>
#undef IN_GSN_TABLE
	{0, 0}
};

const struct spellfun_type spellfun_table[] = {
#define IN_SPELL_TABLE
#include <muddyengine/spellfun.h>
#undef IN_SPELL_TABLE
	{0, 0}
};


const Lookup skill_flags[] = {
	{0, 0}
};

Skill *new_skill(  )
{

	Skill *skill = ( Skill * ) alloc_mem( 1, sizeof( Skill ) );

	skill->name = str_empty;

	skill->damage = str_empty;

	skill->msgObj = str_empty;

	skill->msgOff = str_empty;

	return skill;

}

void destroy_skill( Skill * skill )
{

	free_str( skill->name );

	free_str( skill->damage );

	free_str( skill->msgOff );

	free_str( skill->msgObj );

	free_mem( skill );

}

const char *gsn_name( int *pgsn )
{

	if ( !pgsn )

		return 0;

	for ( int i = 0; gsn_table[i].name != 0; i++ )

		if ( *gsn_table[i].pgsn == *pgsn )

			return gsn_table[i].name;

	return 0;

}

int *gsn_lookup( const char *name )
{

	if ( nullstr( name ) )
		return 0;

	for ( int i = 0; gsn_table[i].name != 0; i++ )

		if ( !str_cmp( name, gsn_table[i].name ) )

			return gsn_table[i].pgsn;

	return 0;

}

Skill *skill_lookup( const char *arg )
{

	if ( nullstr( arg ) )
		return 0;

	for ( int i = 0; i < max_skill; i++ )

		if ( !str_prefix( arg, skill_table[i].name ) )

			return &skill_table[i];

	return 0;

}

const char *spellfun_name( SpellFun * fun )
{

	if ( !fun )
		return 0;

	for ( int i = 0; spellfun_table[i].name != 0; i++ )

		if ( spellfun_table[i].fun == fun )

			return spellfun_table[i].name;

	return 0;

}

SpellFun *spellfun_lookup( const char *name )
{

	if ( nullstr( name ) )
		return 0;

	for ( int i = 0; spellfun_table[i].name != 0; i++ )

		if ( !str_cmp( spellfun_table[i].name, name ) )

			return spellfun_table[i].fun;

	return 0;

}

int load_skill_levels( Skill * skill )
{

	char buf[400];

	db_stmt *stmt;

	int total = 0;

	int len = sprintf( buf, "select * from skill_level where skillId=%"PRId64,
					   skill->id );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare statement" );

		return 0;

	}

	skill->levels = ( int * ) alloc_mem( max_class, sizeof( int ) );

	while ( db_step( stmt ) != DB_DONE )
	{

		int count = db_column_count( stmt );

		int classId = 0;

		short level = -1;

		int index;

		for ( int i = 0; i < count; i++ )
		{

			const char *colname = db_column_name( stmt, i );

			if ( !str_cmp( colname, "skillId" ) )
			{

				if ( db_column_int( stmt, i ) != skill->id )

					log_error
						( "load_skill_levels: sql returned invalid skill" );

			}
			else if ( !str_cmp( colname, "classId" ) )
			{

				classId = db_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "level" ) )
			{

				level = db_column_int( stmt, i );

			}
			else
			{

				log_error( "unknown skill level column '%s'", colname );

			}

		}

		for ( index = 0; index < max_class; index++ )

			if ( class_table[index].id == classId )

				break;

		if ( index != max_class )

			skill->levels[index] = level;

		total++;

	}

	if (db_finalize(stmt) != DB_OK) {
		log_data("unable to finalize skill sql statement");
	}

	return total;

}

bool valid_skill( identifier_t sn )
{

	return sn >= 0 && sn < max_skill;

}

int load_skills(  )
{

	char buf[400];

	db_stmt *stmt;

	int total = 0;

	int len = sprintf( buf, "select count(*) from skill" );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare statement" );

		return 0;

	}

	if ( db_step( stmt ) == DB_DONE )
	{

		log_data( "could not count skills" );

		return 0;

	}

	max_skill = db_column_int( stmt, 0 );

	if ( db_finalize( stmt ) != DB_OK )
	{

		log_data( "could not finalize statement" );

	}

	skill_table = ( Skill * ) alloc_mem( max_skill, sizeof( Skill ) );

	len = sprintf( buf, "select * from skill" );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare statement" );

		return 0;

	}

	while ( db_step( stmt ) != DB_DONE )
	{

		int count = db_column_count( stmt );

		for ( int i = 0; i < count; i++ )
		{

			const char *colname = db_column_name( stmt, i );

			if ( !str_cmp( colname, "name" ) )
			{

				skill_table[total].name =
					str_dup( db_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "skillId" ) )
			{

				skill_table[total].id = db_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "msgOff" ) )
			{

				skill_table[total].msgOff =
					str_dup( db_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "msgObj" ) )
			{

				skill_table[total].msgObj =
					str_dup( db_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "mana" ) )
			{

				skill_table[total].mana = db_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "wait" ) )
			{

				skill_table[total].wait = db_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "cost" ) )
			{

				skill_table[total].cost = sqlite3_column_double( stmt, i );

			}
			else if ( !str_cmp( colname, "damage" ) )
			{

				skill_table[total].damage =
					str_dup( db_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "flags" ) )
			{

				parse_flags( &skill_table[total].flags,
							 db_column_str( stmt, i ), skill_flags );

			}
			else if ( !str_cmp( colname, "minPos" ) )
			{

				skill_table[total].minPos = db_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "spell" ) )
			{

				skill_table[total].spellfun =
					spellfun_lookup( db_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "gsn" ) )
			{

				skill_table[total].pgsn =
					gsn_lookup( db_column_str( stmt, i ) );

				if ( skill_table[total].pgsn != 0 )

					*skill_table[total].pgsn = total;

			}
			else
			{

				log_warn( "unknown skill column '%s'", colname );

			}

		}

		load_skill_levels( &skill_table[total] );

		total++;

	}

	if ( db_finalize( stmt ) != DB_OK )
	{

		log_data( "could not finalize statement" );

	}

	if ( total != max_skill )
	{

		log_warn( "counted skills did not match number read" );

	}

	return total;

}

static const char *save_gsn( void *data )
{

	return gsn_name( *( ( int ** ) data ) );

}

static const char *save_skillspell( void *data )
{

	return spellfun_name( *( ( SpellFun ** ) data ) );

}

int save_skill( Skill * skill )
{

	char buf[OUT_SIZ];

	struct dbvalues skillvals[] = {
		{"name", &skill->name, DB_TEXT},
		{"damage", &skill->damage, DB_TEXT},
		{"msgOff", &skill->msgOff, DB_TEXT},
		{"msgObj", &skill->msgObj, DB_TEXT},
		{"cost", &skill->cost, DB_FLOAT},
		{"minPos", &skill->minPos, DB_INTEGER},
		{"mana", &skill->mana, DB_INTEGER},
		{"wait", &skill->wait, DB_INTEGER},
		{"gsn", &skill->pgsn, DBTYPE_CUSTOM, save_gsn},
		{"spell", &skill->spellfun, DBTYPE_CUSTOM, save_skillspell},
		{"flags", &skill->flags, DB_FLAG, skill_flags},
		{0}
	};

	if ( skill->id == 0 )
	{

		char names[BUF_SIZ] = { 0 };

		char values[OUT_SIZ] = { 0 };

		build_insert_values( skillvals, names, values );

		sprintf( buf, "insert into skill (%s) values(%s)", names, values );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not insert skill" );

			return 0;

		}

		skill->id = db_last_insert_rowid();

	}
	else
	{

		char values[OUT_SIZ] = { 0 };

		build_update_values( skillvals, values );

		sprintf( buf, "update skill set %s where skillId=%"PRId64, values,
				 skill->id );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not update skill" );

			return 0;

		}

	}

	return 1;

}

SPELL( magic_missile )
{

	writeln( ch, "Got magic missile" );

}

SPELL( energy_shield )
{

	if ( is_affected( ch, sn ) )
	{

		writeln( ch, "You already surrounded with an energy shield." );

		return;

	}

	Affect *paf = new_affect(  );

	paf->from = sn;

	paf->level = ch->level;

	paf->duration = 24;

	paf->modifier = 20;

	paf->callback = &affect_apply_resists;

	affect_to_char( ch, paf );

	writeln( ch, "You surround yourself with an energy shield." );

	act( TO_ROOM, ch, 0, 0, "$n surrounds $mself with an energy shield." );

}
