/******************************************************************************
*         __  __           _     _         ____  _       _                   *
*        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
*        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
*        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
*        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
*                                  |___/                                     *
*                                                                            *
*    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
*              Many thanks to creators of muds before me.                     *
*                                                                            *
*        In order to use any part of this Mud, you must comply with the      *
*     license in 'license.txt'.  In particular, you may not remove either    *
*                        of these copyright notices.                         *
*                                                                            *
*       Much time and thought has gone into this software and you are        *
*     benefitting.  I hope that you share your changes too.  What goes       *
*                            around, comes around.                           *
******************************************************************************/

#include <muddled/engine.h>
#include "importer.h"
#include "config.h"
#include <libgen.h>
#include <inttypes.h>
#include <muddled/string.h>
#include <muddled/log.h>
#include <muddled/area.h>
#include <muddled/room.h>
#include <muddled/class.h>
#include <muddled/exit.h>
#include <muddled/character.h>
#include <muddled/nonplayer.h>
#include <muddled/help.h>
#include <muddled/db.h>
#include <muddled/flag.h>
#include <muddled/lookup.h>
#include <muddled/object.h>
#include <muddled/affect.h>
#include <muddled/social.h>
#include <muddled/race.h>
#include <muddled/skill.h>
#include <muddled/hashmap.h>
#include <ctype.h>

#define A           1
#define B           2
#define C           4
#define D           8
#define E           16
#define F           32
#define G           64
#define H           128

#define I           256
#define J           512
#define K           1024
#define L           2048
#define M           4096
#define N           8192
#define O           16384
#define P           32768

#define Q           65536
#define R           131072
#define S           262144
#define T           524288
#define U           1048576
#define V           2097152
#define W           4194304
#define X           8388608

#define Y           16777216
#define Z           33554432
#define aa          67108864    /* doubled due to conflicts */
#define bb          134217728
#define cc          268435456
#define dd          536870912
#define ee          1073741824

extern const char *const DB_FILE;

typedef struct temp_reset
{
    int id;
    char type;
    long arg1;
    long arg2;
    long arg3;
    long arg4;
    Room *room;
    struct temp_reset *next;
    struct temp_reset *next_sub;
    struct temp_reset *subresets;
} TmpReset;

TmpReset *temp_resets;

Area *area_last = 0;

hashmap *npc_indexes = 0;

hashmap *room_indexes = 0;

hashmap *obj_indexes = 0;

Area *import_areas = 0;

Social *import_socials = 0;

Help *import_helps = 0;

void import_finalize_exits(Room *room)
{
    for (int e = 0; e < MAX_DIR; e++)
    {
        if (!room->exits[e])
            continue;

        identifier_t id = room->exits[e]->to.id;
        identifier_t key = room->exits[e]->key;

        Room *toRoom = (Room *) hm_get(room_indexes, id);
        Object *keyObj = (Object *) hm_get(obj_indexes, key);

        if (!toRoom)
        {
            toRoom = get_room_by_id(id);
        }
        if (!toRoom)
        {
            log_bug("exit (%" PRId64 ") with no room %" PRId64,
                    room->exits[e]->id, id);
            destroy_exit(room->exits[e]);
            room->exits[e] = 0;
        }
        else
        {
            room->exits[e]->to.room = toRoom;

            if (keyObj && keyObj->id != 0)
            {
                room->exits[e]->key = key;
            }
            else
            {
                room->exits[e]->key = 0;
            }

            save_exit(room->exits[e], e);
        }
    }
}

void npc_import_cleanup(void *data)
{
    destroy_char((Character *) data);
}

void obj_import_cleanup(void *data)
{
    destroy_object((Object *) data);
}

void room_import_cleanup(void *data)
{
    Room *room = (Room *) data;

    for (int i = 0; i < MAX_DIR; i++)
        if (room->exits[i] != 0)
            destroy_exit(room->exits[i]);

    destroy_room(room);
}

void import_cleanup()
{
    hm_foreach(npc_indexes, npc_import_cleanup);

    destroy_hashmap(npc_indexes);

    hm_foreach(obj_indexes, obj_import_cleanup);

    destroy_hashmap(obj_indexes);

    hm_foreach(room_indexes, room_import_cleanup);

    destroy_hashmap(room_indexes);

    npc_indexes = obj_indexes = room_indexes = 0;

    for (TmpReset *tmp = temp_resets; tmp; tmp = tmp->next)
    {
        for (TmpReset *sub = tmp->subresets; sub; sub = sub->next_sub)
            free_mem(sub);
        free_mem(tmp);
    }
    temp_resets = 0;

    for (Area * a_next, *area = import_areas; area; area = a_next)
    {
        a_next = area->next;

        destroy_area(area);
    }
};

bool import_rom_area_list(const char *dir, FILE *fpList)
{
    for (;;)
    {
        char strArea[BUF_SIZ];
        FILE *fpArea;

        sprintf(strArea, "%s/%s", dir, fread_word(fpList));

        if (!str_suffix("$", strArea))
            break;

        if ((fpArea = fopen(strArea, "r")) == NULL)
        {
            log_error("%s", strArea);
            return false;
        }
        log_info("importing %s", strArea);

        import_rom_file(fpArea);

        fclose(fpArea);
        fpArea = NULL;
    }
    fclose(fpList);
    return true;
}

void import_finalize_reset(TmpReset *res, char *buf, int tab)
{
    Character *npc;
    Object *obj;

    for (int i = 0; i < tab; i++)
        strcat(buf, "\t");

    switch (res->type)
    {
    case 'M':
        npc = (Character *) hm_get(npc_indexes, res->arg1);
        if (npc->id == 0)
        {
            log_bug("npc %ld not saved", res->arg1);
        }
        sprintf(buf + strlen(buf), "npc(%" PRId64, npc->id);
        if (res->arg4 > 1)
            sprintf(buf + strlen(buf), ",%ld)\n", res->arg4);
        else
            strcat(buf, ")\n");

        break;
    case 'O':
        obj = (Object *) hm_get(obj_indexes, res->arg1);
        if (obj->id == 0)
        {
            log_bug("obj %ld not saved", res->arg1);
        }
        sprintf(buf + strlen(buf), "obj(%" PRId64, obj->id);
        if (res->arg4 > 0)
            sprintf(buf + strlen(buf), ",%ld)\n", res->arg4);
        else
            strcat(buf, ")\n");
        break;
    case 'P':
        obj = (Object *) hm_get(obj_indexes, res->arg1);
        if (obj->id == 0)
        {
            log_bug("obj %ld not saved", res->arg1);
        }
        sprintf(buf + strlen(buf), "put(%" PRId64 ")\n", obj->id);
        /*
         * if ( res->arg2 > 1 ) sprintf( buf + strlen( buf ),
         * ",%d)\n", res->arg2 ); else strcat( buf, ")\n" );
         */
        break;
    case 'G':
        obj = (Object *) hm_get(obj_indexes, res->arg1);
        if (obj->id == 0)
        {
            log_bug("obj %ld not saved", res->arg1);
        }
        sprintf(buf + strlen(buf), "give(%" PRId64 ")\n", obj->id);
        /*
         * if ( res->arg4 > 0 ) sprintf( buf + strlen( buf ),
         * ",%d)\n", res->arg4 ); else strcat( buf, ")\n" );
         */
        break;
    case 'E':
        obj = (Object *) hm_get(obj_indexes, res->arg1);
        if (obj->id == 0)
        {
            log_bug("obj %ld not saved", res->arg1);
        }
        sprintf(buf + strlen(buf), "equip(%" PRId64 ")\n", obj->id);
        /*
         * lookup_name( wear_flags, wear_type_to_flag( res->arg2 ) ) );
         */

        break;
    case 'R':
        break;
    }

    for (TmpReset *sub = res->subresets; sub; sub = sub->next_sub)
    {
        import_finalize_reset(sub, buf, tab + 1);
    }
}

void import_save_area(Area *area)
{
    save_area_only(area);

    for (Character *npc = area->npcs; npc; npc = npc->next_in_area)
    {
        save_npc(npc);
    }

    for (Object *obj = area->objects; obj; obj = obj->next_in_area)
    {
        save_object(obj);
    }
}

void import_commit(bool load)
{
    db_begin_transaction();

    for (Area *area = import_areas; area; area = area->next)
    {
        import_save_area(area);
    }

    for (TmpReset * tmp_next, *tmp = temp_resets; tmp; tmp = tmp_next)
    {
        tmp_next = tmp->next;

        char buf[OUT_SIZ * 2] = { 0 };

        import_finalize_reset(tmp, buf, 0);

        tmp->room->reset = str_dup(buf);

        // save_room_only(tmp->room);

        for (TmpReset *sub_next = 0, *sub = tmp->subresets; sub;
                sub = sub->next_sub)
        {
            sub_next = sub->next;
            free_mem(sub);
        }

        free_mem(tmp);
    }

    temp_resets = 0;

    for (Area *area = import_areas; area; area = area->next)
    {
        for (Room *room = area->rooms; room; room = room->next_in_area)
        {
            save_room_only(room);
        }
    }

    for (Area *area = import_areas; area; area = area->next)
    {

        for (Room *room = area->rooms; room; room = room->next_in_area)
        {
            import_finalize_exits(room);
        }

        if (load)
            load_area(area->id);
    }

    db_end_transaction();

    import_cleanup();
}

void server_import(const char *db_path, const char *file)
{
    FILE *fp = fopen(file, "r");
    bool success;

    if (fp == 0)
    {
        log_bug("Could not open %s for importing.", file);
        return;
    }
    if (db_open(DB_FILE, db_path))
    {
        log_data("Can't open database");
        db_close();
        exit(EXIT_FAILURE);
    }
    log_info("loaded %d races", load_races());

    log_info("loaded %d classes", load_classes());

    log_info("loaded %d skills", load_skills());

    if (!str_suffix(".lst", file))
    {
        if (!
                (success = import_rom_area_list(dirname((char *)file), fp)))
            log_bug("Could not import areas.");
        else
            log_info("Areas imported.");
    }
    else
    {
        if (!(success = import_rom_file(fp)))
            log_bug("Could not import file.");
        else
            log_info("File imported.");
    }

    if (success)
    {
        import_commit(false);
        log_info("Areas commited.");
    }
    db_close();
}

bool import_rom_file(FILE *fpArea)
{

    if (npc_indexes == 0)
        npc_indexes = new_hashmap(0);

    if (obj_indexes == 0)
        obj_indexes = new_hashmap(0);

    if (room_indexes == 0)
        room_indexes = new_hashmap(0);

    for (;;)
    {
        const char *word;

        if (fread_letter(fpArea) != '#')
        {
            log_bug("# not found.");
            fclose(fpArea);
            import_cleanup();
            return false;
        }
        word = fread_word(fpArea);

        if (word[0] == '$')
            break;
        else if (!str_cmp(word, "AREA"))
            import_rom_area(fpArea);
        else if (!str_cmp(word, "AREADATA"))
            import_rom_olc_area(fpArea);
        else if (!str_cmp(word, "HELPS"))
            import_rom_helps(fpArea);
        else if (!str_cmp(word, "MOBILES"))
            import_rom_mobiles(fpArea);
        else if (!str_cmp(word, "OBJECTS"))
            import_rom_objects(fpArea);
        else if (!str_cmp(word, "RESETS"))
            import_rom_resets(fpArea);
        else if (!str_cmp(word, "ROOMS"))
            import_rom_rooms(fpArea);
        else if (!str_cmp(word, "SHOPS"))
            import_rom_shops(fpArea);
        else if (!str_cmp(word, "SOCIALS"))
            import_rom_socials(fpArea);
        else if (!str_cmp(word, "SPECIALS"))
            import_rom_specials(fpArea);
        else
        {
            log_bug("bad section name.");
            fclose(fpArea);
            import_cleanup();
            return false;
        }
    }
    return true;

}

bool import_rom_area(FILE *fp)
{
    Area *pArea;

    pArea = new_area();

    set_bit(pArea->flags, AREA_CHANGED);

    fread_string(fp);
    // filename
    pArea->name = fread_string(fp);
    fread_string(fp);
    // credits
    fread_number(fp);
    // lvnum
    fread_number(fp);
    // uvnum

    LINK(import_areas, pArea, next);

    area_last = pArea;

    return true;
}

bool import_rom_olc_area(FILE *fp)
{
    Area *pArea;
    const char *word;
    bool fMatch;

    pArea = new_area();

    for (;;)
    {
        word = feof(fp) ? "End" : fread_word(fp);
        fMatch = false;

        switch (UPPER(word[0]))
        {
        case 'C':
            if (!str_cmp(word, "Credits"))
            {
                free_str(fread_string(fp));
                fMatch = true;
                break;
            }
            break;
        case 'N':
            if (!str_cmp(word, "Name"))
            {
                pArea->name = fread_string(fp);
                fMatch = true;
                break;
            }
            break;
        case 'S':
            if (!str_cmp(word, "Security"))
            {
                fread_number(fp);
                fMatch = true;
                break;
            }
            break;
        case 'V':
            if (!str_cmp(word, "VNUMs"))
            {
                fread_number(fp);
                fread_number(fp);
                fMatch = true;
                break;
            }
            break;
        case 'E':
            if (!str_cmp(word, "End"))
            {
                fMatch = true;
                set_bit(pArea->flags, AREA_CHANGED);
                LINK(import_areas, pArea, next);
                area_last = pArea;
                return true;
            }
            break;
        case 'B':
            if (!str_cmp(word, "Builders"))
            {
                free_str(fread_string(fp));
                fMatch = true;
                break;
            }
            break;
        }
    }
    return false;
}

void rom_act_flag_convert(Flag *flags, long bits)
{
    static const struct
    {
        long rombit;
        int bit;
    } actconvert[] =
    {

        {
            B, NPC_SENTINEL
        },  // ACT_SENTINEL
        {
            C, NPC_SCAVENGER
        }, {
            F, NPC_AGGRESSIVE
        }, {
            G, NPC_STAY_AREA
        }, {
            H, NPC_WIMPY
        }
    };

    for (int i = 0; i < sizeof(actconvert) / sizeof(actconvert[0]); i++)
    {
        if (actconvert[i].rombit & bits)
        {
            set_bit(flags, actconvert[i].bit);
        }
    }
}

void rom_resists_flag_convert(Character *ch, long bits, int mod)
{
    static const struct
    {
        int resist;
        long rombit;
    } resistconv[] =
    {
        {
            DAM_BASH, E
        }, {
            DAM_PIERCE, F
        }, {
            DAM_SLASH, G
        }, {
            DAM_FIRE, H
        }, {
            DAM_COLD, I
        }, {
            DAM_ACID, J
        }, {
            DAM_POISON, K
        }, {
            DAM_SHADOW, L
        }, {
            DAM_HOLY, M
        }, {
            DAM_ENERGY, N
        }, {
            DAM_MENTAL, O
        }, {
            DAM_DISEASE, P
        }, {
            DAM_LIGHT, S
        }
    };

    for (int i = 0; i < sizeof(resistconv) / sizeof(resistconv[0]); i++)
    {
        if (resistconv[i].rombit & bits)
        {
            ch->resists[resistconv[i].resist] += (mod * ch->level);
        }
    }
}

float rom_convert_size(const char *word)
{
    static const struct
    {
        float size;
        const char *name;
    } sizetable[] =
    {
        {
            1.8f, "tiny"
        }, {
            2.7f, "small"
        }, {
            4.8f, "medium"
        }, {
            6.7f, "large"
        }, {
            8.8f, "huge1"
        }, {
            10.0f, "giant"
        }
    };

    for (int i = 0; i < sizeof(sizetable) / sizeof(sizetable[0]); i++)
    {
        if (!str_cmp(word, sizetable[i].name))
            return sizetable[i].size;
    }
    return 4.8f;
}

bool import_rom_mobiles(FILE *fp)
{
    Character *pMobIndex;

    if (!area_last)     /* OLC */
    {
        log_bug("no area seen yet.");
        return false;
    }
    for (;;)
    {
        long vnum;
        char letter;

        letter = fread_letter(fp);
        if (letter != '#')
        {
            log_bug("# not found.");
            return false;
        }
        vnum = fread_number(fp);
        if (vnum == 0)
            break;

        if (hm_get(npc_indexes, vnum) != NULL)
        {
            log_bug("vnum %ld duplicated.", vnum);
            return false;
        }
        pMobIndex = new_char();
        pMobIndex->npc = new_npc();
        hm_insert(npc_indexes, pMobIndex, vnum);
        pMobIndex->npc->area = area_last;
        pMobIndex->name = fread_string(fp);
        pMobIndex->npc->shortDescr = fread_string(fp);
        pMobIndex->npc->longDescr = fread_string(fp);
        pMobIndex->description = fread_string(fp);
        const char *tmp = fread_string(fp);
        pMobIndex->race = race_lookup(tmp);
        free_str(tmp);

        if (pMobIndex->race == 0)
            pMobIndex->race = first_race;

        rom_act_flag_convert(pMobIndex->flags, fread_flag(fp));
        /*
         * | race_table[pMobIndex->race].act;
         */
        /*
         * pMobIndex->affected_by =
         */
        fread_flag(fp)
        /*
         * | race_table[pMobIndex->race].aff
         */ ;

        pMobIndex->alignment = (int)fread_number(fp);
        /*
         * pMobIndex->group =
         */
        fread_number(fp);

        pMobIndex->level = fread_number(fp);
        /*
         * pMobIndex->hitroll =
         */
        fread_number(fp);

        /*
         * read hit dice
         */
        pMobIndex->npc->hit[DICE_NUMBER] = (int)fread_number(fp);
        /*
         * 'd'
         */
        fread_letter(fp);
        pMobIndex->npc->hit[DICE_TYPE] = (int)fread_number(fp);
        /*
         * '+'
         */
        fread_letter(fp);
        pMobIndex->npc->hit[DICE_BONUS] = (int)fread_number(fp);

        /*
         * read mana dice
         */
        pMobIndex->npc->mana[DICE_NUMBER] = (int)fread_number(fp);
        fread_letter(fp);
        pMobIndex->npc->mana[DICE_TYPE] = (int)fread_number(fp);
        fread_letter(fp);
        pMobIndex->npc->mana[DICE_BONUS] = (int)fread_number(fp);

        /*
         * read damage dice
         */
        pMobIndex->npc->damage[DICE_NUMBER] = (int)fread_number(fp);
        fread_letter(fp);
        pMobIndex->npc->damage[DICE_TYPE] = (int)fread_number(fp);
        fread_letter(fp);
        pMobIndex->npc->damage[DICE_BONUS] = (int)fread_number(fp);
        /*
         * pMobIndex->dam_type = attack_lookup(
         */
        fread_word(fp)
        /*
         * )
         */ ;

        /*
         * read armor class
         */
        pMobIndex->resists[DAM_PIERCE] = (int)fread_number(fp);
        pMobIndex->resists[DAM_BASH] = (int)fread_number(fp);
        pMobIndex->resists[DAM_SLASH] = (int)fread_number(fp);
        /*
         * pMobIndex->resists[DAM_MAGIC] =
         */
        int res = (int)fread_number(fp);
        for (int i = DAM_SLASH; i < MAX_DAM; i++)
            pMobIndex->resists[i] = res;

        /*
         * read flags and add in data from the race table
         */
        /*
         * pMobIndex->off_flags =
         */
        fread_flag(fp)
        /*
         * | race_table[pMobIndex->race].off
         */ ;
        /*
         * pMobIndex->imm_flags =
         */
        rom_resists_flag_convert(pMobIndex, fread_flag(fp), 1000);  /*
                                         * |
                                         * race_table[pMobIndex->race].imm
                                         */ ;
        /*
         * pMobIndex->res_flags =
         */
        rom_resists_flag_convert(pMobIndex, fread_flag(fp), 100);   /*
                                         * |
                                         * race_table[pMobIndex->race].res
                                         */ ;
        /*
         * pMobIndex->vuln_flags =
         */
        rom_resists_flag_convert(pMobIndex, fread_flag(fp), -100);  /*
                                         * |
                                         * race_table[pMobIndex->race].vuln
                                         */ ;

        int e = value_lookup(position_table, fread_word(fp));

        /*
         * vital statistics
         */
        pMobIndex->position = e == -1 ? POS_STANDING : ((position_t) e);
        e = value_lookup(position_table, fread_word(fp));
        pMobIndex->npc->startPosition =
            e == -1 ? POS_STANDING : ((position_t) e);
        e = value_lookup(sex_table, fread_word(fp));
        pMobIndex->sex = e == -1 ? SEX_NEUTRAL : ((sex_t) e);

        pMobIndex->gold = fread_number(fp);

        /*
         * pMobIndex->form =
         */
        fread_flag(fp)
        /*
         * | race_table[pMobIndex->race].form
         */ ;
        /*
         * pMobIndex->parts =
         */
        fread_flag(fp)
        /*
         * | race_table[pMobIndex->race].parts
         */ ;
        /*
         * size
         */
        /*
         * pMobIndex->size = size_lookup(
         */
        pMobIndex->size = rom_convert_size(fread_word(fp)) /* ) */ ;
        /*
         * pMobIndex->material = str_dup(
         */
        fread_word(fp);
        /*
         * )
         */

        for (;;)
        {
            letter = fread_letter(fp);

            if (letter == 'F')
            {
                const char *word;
                long vector;

                word = fread_word(fp);
                vector = fread_flag(fp);

                if (!str_prefix(word, "act"))
                    /*
                     * REMOVE_BIT(pMobIndex->act,vector)
                     */ ;
                else if (!str_prefix(word, "aff"))
                    /*
                     * REMOVE_BIT(pMobIndex->affected_by,v
                     * ector)
                     */ ;
                else if (!str_prefix(word, "off"))
                    /*
                     * REMOVE_BIT(pMobIndex->off_flags,vec
                     * tor)
                     */ ;
                else if (!str_prefix(word, "imm"))
                    /*
                     * REMOVE_BIT(pMobIndex->imm_flags,vec
                     * tor)
                     */ ;
                else if (!str_prefix(word, "res"))
                    /*
                     * REMOVE_BIT(pMobIndex->res_flags,vec
                     * tor)
                     */ ;
                else if (!str_prefix(word, "vul"))
                    /*
                     * REMOVE_BIT(pMobIndex->vuln_flags,ve
                     * ctor)
                     */ ;
                else if (!str_prefix(word, "for"))
                    /*
                     * REMOVE_BIT(pMobIndex->form,vector)
                     */ ;
                else if (!str_prefix(word, "par"))
                    /*
                     * REMOVE_BIT(pMobIndex->parts,vector)
                     *
                     */ ;
                else
                {
                    log_bug("flag not found.");
                    destroy_char(pMobIndex);
                    return false;
                }

            }
            else
            {
                ungetc(letter, fp);
                break;
            }
        }

        LINK(area_last->npcs, pMobIndex, next_in_area);
    }
    return true;
}

int lookup_rom_obj_type(const char *word)
{
    static const struct
    {
        int romtype;
        const char *name;
        object_type type;
    } table[] =
    {
        {
            1, "light", OBJ_LIGHT
        }, {
            2, "scroll", OBJ_SCROLL
        }, {
            3, "wand", OBJ_WAND
        }, {
            4, "staff", OBJ_STAFF
        }, {
            5, "weapon", OBJ_WEAPON
        }, {
            8, "treasure", OBJ_TREASURE
        }, {
            9, "armor", OBJ_ARMOR
        }, {
            10, "potion", OBJ_POTION
        }, {
            11, "clothing", OBJ_TRASH
        }, {
            12, "furniture", OBJ_FURNITURE
        }, {
            13, "trash", OBJ_TRASH
        }, {
            15, "container", OBJ_CONTAINER
        }, {
            17, "drink_cont", OBJ_DRINK_CON
        }, {
            17, "drink", OBJ_DRINK_CON
        }, {
            18, "key", OBJ_KEY
        }, {
            19, "food", OBJ_FOOD
        }, {
            20, "money", OBJ_MONEY
        }, {
            22, "boat", OBJ_BOAT
        }, {
            23, "corpse_npc", OBJ_CORPSE_NPC
        }, {
            24, "corpse_pc", OBJ_CORPSE_PC
        }, {
            25, "fountain", OBJ_FOUNTAIN
        }, {
            26, "pill", OBJ_PILL
        }, {
            27, "protect", OBJ_TRASH
        }, {
            28, "map", OBJ_TRASH
        }, {
            29, "portal", OBJ_PORTAL
        }, {
            30, "warp_stone", OBJ_WARP_STONE
        }, {
            31, "room_key", OBJ_ROOM_KEY
        }, {
            32, "gem", OBJ_TREASURE
        }, {
            33, "jewelry", OBJ_JEWELRY
        }, {
            34, "jukebox", OBJ_TRASH
        }, {
            0, 0
        }
    };

    for (int i = 0; table[i].name != 0; i++)
    {
        if (!str_cmp(word, table[i].name))
            return table[i].romtype;
    }

    return 13;
}

Flag *rom_convert_weapon_flags(long bits)
{
    static const struct
    {
        int rombit;
        int bit;
    } convert[] =
    {
        {
            A, WEAPON_FLAMING
        }, {
            B, WEAPON_FROST
        }, {
            C, WEAPON_VAMPIRIC
        }, {
            D, WEAPON_SHARP
        }, {
            E, WEAPON_VORPAL
        }, {
            F, WEAPON_TWO_HANDS
        }, {
            G, WEAPON_SHOCKING
        }, {
            H, WEAPON_POISON
        }
    };

    Flag *flag = new_flag();

    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if ((convert[i].rombit & bits) != 0)
            set_bit(flag, convert[i].bit);
    }

    return flag;
}

Flag *rom_convert_container_flags(long bits)
{
    static const struct
    {
        int rombit;
        int bit;
    } convert[] =
    {
        {
            A, CONT_CLOSEABLE
        }, {
            B, CONT_PICKPROOF
        }, {
            C, CONT_CLOSED
        }, {
            D, CONT_LOCKED
        }, {
            E, CONT_PUT_ON
        },
    };

    Flag *flag = new_flag();

    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if ((convert[i].rombit & bits) != 0)
        {
            set_bit(flag, convert[i].bit);
        }
    }

    return flag;
}

wear_type import_rom_convert_wear_flags(long flag)
{
    if (flag & B)
        return WEAR_FINGER;

    if (flag & C)
        return WEAR_NECK;

    if (flag & D)
        return WEAR_TORSO;

    if (flag & E)
        return WEAR_HEAD;

    if (flag & F)
        return WEAR_LEGS;

    if (flag & G)
        return WEAR_FEET;

    if (flag & H)
        return WEAR_HANDS;

    if (flag & I)
        return WEAR_ARMS;

    if (flag & J)
        return WEAR_SHIELD;

    if (flag & K)
        return WEAR_ABOUT;

    if (flag & L)
        return WEAR_WAIST;

    if (flag & M)
        return WEAR_WRIST;

    if (flag & N)
        return WEAR_WIELD;

    if (flag & O)
        return WEAR_HOLD;

    if (flag & Q)
        return WEAR_FLOAT;

    return WEAR_NONE;
}

void rom_load_obj_values(Object *pObjIndex, int item_type, FILE *fp)
{

    switch (item_type)
    {
    /*
     * case ITEM_WEAPON:
     */
    case 5:
        vset_long(&pObjIndex->value[0],
                  value_lookup(weapon_types, fread_word(fp)));
        vset_long(&pObjIndex->value[1], fread_number(fp));
        vset_long(&pObjIndex->value[2], fread_number(fp));
        vset_long(&pObjIndex->value[3],
                  value_lookup(dam_types, fread_word(fp)));
        vset_ptr(&pObjIndex->value[4],
                 rom_convert_weapon_flags(fread_flag(fp)));
        break;
    // case ITEM_CONTAINER:
    case 15:
        vset_long(&pObjIndex->value[0], fread_number(fp));
        vset_ptr(&pObjIndex->value[1],
                 rom_convert_container_flags(fread_flag(fp)));
        vset_long(&pObjIndex->value[2], fread_number(fp));
        vset_long(&pObjIndex->value[3], fread_number(fp));
        vset_long(&pObjIndex->value[4], fread_number(fp));
        break;
    case 17:
    case 25:
        vset_long(&pObjIndex->value[0], fread_number(fp));
        vset_long(&pObjIndex->value[1], fread_number(fp));
        vset_ptr(&pObjIndex->value[2], fread_word(fp));
        vset_long(&pObjIndex->value[3], fread_number(fp));
        vset_long(&pObjIndex->value[4], fread_number(fp));
        break;
    // case ITEM_WAND:
    // case ITEM_STAFF:
    case 3:
    case 4:
        vset_long(&pObjIndex->value[0], fread_number(fp));
        vset_long(&pObjIndex->value[1], fread_number(fp));
        vset_long(&pObjIndex->value[2], fread_number(fp));
        vset_ptr(&pObjIndex->value[3], skill_lookup(fread_word(fp)));
        vset_long(&pObjIndex->value[4], fread_number(fp));
        break;
    // case ITEM_POTION:
    // case ITEM_PILL:
    // case ITEM_SCROLL:
    case 10:
    case 26:
    case 2:
        vset_long(&pObjIndex->value[0], fread_number(fp));
        vset_ptr(&pObjIndex->value[1], skill_lookup(fread_word(fp)));
        vset_ptr(&pObjIndex->value[2], skill_lookup(fread_word(fp)));
        vset_ptr(&pObjIndex->value[3], skill_lookup(fread_word(fp)));
        vset_ptr(&pObjIndex->value[4], skill_lookup(fread_word(fp)));
        break;
    default:
        vset_long(&pObjIndex->value[0], fread_flag(fp));
        vset_long(&pObjIndex->value[1], fread_flag(fp));
        vset_long(&pObjIndex->value[2], fread_flag(fp));
        vset_long(&pObjIndex->value[3], fread_flag(fp));
        vset_long(&pObjIndex->value[4], fread_flag(fp));
        break;
    }
}

void rom_convert_obj_flags(Flag *flags, long flag)
{
    static const struct
    {
        int bit;
        long rombit;
    } convert[] =
    {
        {
            ITEM_GLOW, (A)
        }, {
            ITEM_HUM, (B)
        }, {
            ITEM_DARK, (C)
        }, {
            ITEM_EVIL, (E)
        }, {
            ITEM_INVIS, (F)
        }, {
            ITEM_MAGIC, (G)
        }, {
            ITEM_NODROP, (H)
        }, {
            ITEM_BLESS, (I)
        }, {
            ITEM_ANTI_GOOD, (J)
        }, {
            ITEM_ANTI_EVIL, (K)
        }, {
            ITEM_ANTI_NEUTRAL, (L)
        }, {
            ITEM_NOREMOVE, (M)
        }, {
            ITEM_INVENTORY, (N)
        }, {
            ITEM_NOPURGE, (O)
        }, {
            ITEM_ROT_DEATH, (P)
        }, {
            ITEM_BURN_PROOF, (Y)
        }
    };

    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if ((convert[i].rombit & flag) != 0)
            set_bit(flags, convert[i].bit);
    }
}

Flag *rom_convert_affect_flags(long bits)
{
    static const struct
    {
        int bit;
        long rombit;
    } convert[] =
    {
        {
            AFF_BLIND, (A)
        }, {
            AFF_INVISIBLE, (B)
        }, {
            AFF_DETECT_EVIL, (C)
        }, {
            AFF_DETECT_INVIS, (D)
        }, {
            AFF_DETECT_MAGIC, (E)
        }, {
            AFF_DETECT_HIDDEN, (F)
        }, {
            AFF_DETECT_GOOD, (G)
        }, {
            AFF_SANCTUARY, (H)
        }, {
            AFF_FAERIE_FIRE, (I)
        }, {
            AFF_INFRARED, (J)
        }, {
            AFF_CURSE, (K)
        }, {
            AFF_POISON, (M)
        }, {
            AFF_PROTECT_EVIL, (N)
        }, {
            AFF_PROTECT_GOOD, (O)
        }, {
            AFF_SNEAK, (P)
        }, {
            AFF_HIDE, (Q)
        }, {
            AFF_SLEEP, (R)
        }, {
            AFF_CHARM, (S)
        }, {
            AFF_FLYING, (T)
        }, {
            AFF_PASS_DOOR, (U)
        }, {
            AFF_HASTE, (V)
        }, {
            AFF_CALM, (W)
        }, {
            AFF_PLAGUE, (X)
        }, {
            AFF_WEAKEN, (Y)
        }, {
            AFF_DARK_VISION, (Z)
        }, {
            AFF_BERSERK, (aa)
        }, {
            AFF_REGENERATION, (cc)
        }, {
            AFF_SLOW, (dd)
        }
    };

    Flag *flags = new_flag();
    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if ((convert[i].rombit & bits) != 0)
            set_bit(flags, convert[i].bit);
    }
    return flags;
}

void *convert_affect_location(int location)
{
    static const struct
    {
        void *callback;
        int location;
    } convert[] =
    {
        {
            &affect_apply_str, 1
        }, {
            &affect_apply_dex, 2
        }, {
            &affect_apply_int, 3
        }, {
            &affect_apply_wis, 4
        }, {
            &affect_apply_con, 5
        }, {
            &affect_apply_sex, 6
        }, {
            &affect_apply_level, 8
        }, {
            &affect_apply_mana, 12
        }, {
            &affect_apply_hit, 13
        }, {
            &affect_apply_move, 14
        }
    };
    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if (convert[i].location == location)
            return convert[i].callback;
    }
    return 0;
}

bool import_rom_objects(FILE *fp)
{
    Object *pObjIndex;

    if (!area_last)     /* OLC */
    {
        log_bug("no area seen yet.");
        return false;
    }
    for (;;)
    {
        long vnum;
        char letter;

        letter = fread_letter(fp);
        if (letter != '#')
        {
            log_bug("# not found.");
            return false;
        }
        vnum = fread_number(fp);
        if (vnum == 0)
            break;

        if (hm_get(obj_indexes, vnum) != NULL)
        {
            log_bug("vnum %ld duplicated.", vnum);
            return false;
        }
        pObjIndex = new_object();
        hm_insert(obj_indexes, pObjIndex, vnum);
        pObjIndex->area = area_last;
        pObjIndex->name = fread_string(fp);
        pObjIndex->shortDescr = fread_string(fp);
        pObjIndex->description = fread_string(fp);
        /*
         * pObjIndex->material = str_dup(
         */
        fread_string(fp)
        /*
         * )
         */ ;

        const char *item_type = fread_word(fp);

        pObjIndex->type =
            (object_type) value_lookup(object_types, item_type);

        rom_convert_obj_flags(pObjIndex->flags, fread_flag(fp));

        pObjIndex->wearFlags =
            import_rom_convert_wear_flags(fread_flag(fp));

        rom_load_obj_values(pObjIndex, lookup_rom_obj_type(item_type),
                            fp);

        pObjIndex->level = fread_number(fp);
        pObjIndex->weight = (float)fread_number(fp);
        pObjIndex->cost = fread_number(fp);

        /*
         * condition
         */
        letter = fread_letter(fp);
        switch (letter)
        {
        case ('P'):
            pObjIndex->condition = 100.0f;
            break;
        case ('G'):
            pObjIndex->condition = 90.0f;
            break;
        case ('A'):
            pObjIndex->condition = 75.0f;
            break;
        case ('W'):
            pObjIndex->condition = 50.0f;
            break;
        case ('D'):
            pObjIndex->condition = 25.0f;
            break;
        case ('B'):
            pObjIndex->condition = 10.0f;
            break;
        case ('R'):
            pObjIndex->condition = 0.0f;
            break;
        default:
            pObjIndex->condition = 100.0f;
            break;
        }

        for (;;)
        {
            char letter;

            letter = fread_letter(fp);

            if (letter == 'A')
            {
                Affect *paf;

                paf = new_affect();
                paf->level = pObjIndex->level;
                paf->duration = -1;
                paf->callback =
                    convert_affect_location(fread_number(fp));
                paf->modifier = (int)fread_number(fp);
                paf->flags = new_flag();
                LINK(pObjIndex->affects, paf, next);
            }
            else if (letter == 'F')
            {
                Affect *paf;

                paf = new_affect();
                letter = fread_letter(fp);
                switch (letter)
                {
                case 'A':
                    break;
                case 'I':
                case 'R':
                case 'V':
                    paf->callback = &affect_apply_resists;
                    break;

                default:
                    log_bug("Bad where on flag set.");
                    destroy_object(pObjIndex);
                    return false;
                }
                paf->level = pObjIndex->level;
                paf->duration = -1;
                paf->callback =
                    convert_affect_location(fread_number(fp));
                paf->modifier = (int)fread_number(fp);
                /*
                 * paf->flag =
                 */
                paf->flags =
                    rom_convert_affect_flags(fread_flag(fp));
                LINK(pObjIndex->affects, paf, next);
            }
            else if (letter == 'E')
            {
                // EXTRA_DESCR_DATA * ed;
                // Property * prop = new_property();

                const char *key = fread_string(fp);
                const char *value = fread_string(fp);

                sm_insert(&pObjIndex->extraDescr, value, key);

                // ed = alloc_perm(sizeof(*ed));
                // prop->key = fread_string(fp);
                // prop->value = fread_string(fp);

                // LINK(pObjIndex->extraDescr, prop, next);
            }
            else
            {
                ungetc(letter, fp);
                break;
            }
        }

        LINK(area_last->objects, pObjIndex, next_in_area);

    }

    return true;
}

void rom_convert_room_flags(Flag *flags, long bits)
{
    static const struct
    {
        int bit;
        long rombit;
    } convert[] =
    {
        {
            ROOM_SAFE, (K)
        }, {
            ROOM_NO_RECALL, (N)
        }
    };

    for (int i = 0; i < sizeof(convert) / sizeof(convert[0]); i++)
    {
        if ((convert[i].rombit & bits) != 0)
            set_bit(flags, convert[i].bit);
    }
}

bool import_rom_rooms(FILE *fp)
{

    Room *pRoomIndex;

    if (area_last == NULL)
    {
        log_bug("no area seen yet.");
        return false;
    }
    for (;;)
    {
        char letter;
        int door;

        letter = fread_letter(fp);
        if (letter != '#')
        {
            log_bug("# not found.");
            return false;
        }
        long vnum = fread_number(fp);
        if (vnum == 0)
            break;

        if (hm_get(room_indexes, vnum) != NULL)
        {
            log_bug("vnum %ld duplicated.", vnum);
            return false;
        }
        pRoomIndex = new_room();
        hm_insert(room_indexes, pRoomIndex, vnum);
        pRoomIndex->area = area_last;
        pRoomIndex->name = fread_string(fp);
        pRoomIndex->description = fread_string(fp);
        /*
         * Area number
         */
        fread_number(fp);
        rom_convert_room_flags(pRoomIndex->flags, fread_flag(fp));

        pRoomIndex->sector = (sector_t) fread_number(fp);

        for (;;)
        {
            letter = fread_letter(fp);

            if (letter == 'S')
                break;

            if (letter == 'H')  /* healing room */
                fread_number(fp);

            else if (letter == 'M') /* mana room */
                fread_number(fp);

            else if (letter == 'C')     /* clan */
            {

                free_str(fread_string(fp));
            }
            else if (letter == 'D')
            {
                Exit *pexit;
                long locks;

                door = (int)fread_number(fp);
                if (door < 0 || door > 5)
                {
                    log_bug("%s bad door number.",
                            pRoomIndex->name);
                    destroy_room(pRoomIndex);
                    return false;
                }
                pexit = new_exit();
                free_str(fread_string(fp));
                free_str(fread_string(fp));
                pexit->fromRoom = pRoomIndex;

                locks = fread_number(fp);
                pexit->key = fread_number(fp);
                pexit->to.id = fread_number(fp);
                // pexit->orig_door = door; /* OLC */

                switch (locks)
                {
                case 0:
                    break;
                case 1:
                    set_bit(set_bit
                            (pexit->flags, EXIT_ISDOOR),
                            EXIT_CLOSED);
                    break;
                case 2:
                    set_bit(set_bit
                            (set_bit
                             (pexit->flags,
                              EXIT_ISDOOR), EXIT_CLOSED),
                            EXIT_LOCKED);
                    break;
                case 3:
                    // pexit->flags = EX_ISDOOR | EX_CLOSED | EX_LOCKED |
                    // EX_NOPASS;
                    break;
                    set_bit(set_bit
                            (set_bit
                             (pexit->flags,
                              EXIT_ISDOOR), EXIT_CLOSED),
                            EXIT_LOCKED);
                    break;
                case 4:
                    // pexit->flags = EX_ISDOOR | EX_CLOSED | EX_LOCKED |
                    // EX_PICKPROOF;
                    break;
                    set_bit(set_bit
                            (set_bit
                             (pexit->flags,
                              EXIT_ISDOOR), EXIT_CLOSED),
                            EXIT_LOCKED);
                    break;
                case 5:
                    // pexit->flags = EX_ISDOOR | EX_CLOSED | EX_LOCKED |
                    // EX_PICKPROOF | EX_NOPASS;
                    break;
                    set_bit(set_bit
                            (set_bit
                             (pexit->flags,
                              EXIT_ISDOOR), EXIT_CLOSED),
                            EXIT_LOCKED);
                    break;
                }

                pRoomIndex->exits[door] = pexit;
            }
            else if (letter == 'E')
            {
                // Property * ed = new_property();

                const char *key = fread_string(fp);
                const char *value = fread_string(fp);

                sm_insert(&pRoomIndex->extraDescr, value, key);
            }
            else if (letter == 'O')
            {

                /*
                 * pRoomIndex->owner =
                 */ fread_string(fp);
            }
            else
            {
                log_bug("room %s has flag not 'DES'.",
                        pRoomIndex->name);
                destroy_room(pRoomIndex);
                return false;
            }
        }

        LINK(area_last->rooms, pRoomIndex, next_in_area);
    }
    return true;
}

int convert_rom_wear_loc(int val)
{
    int table[] =
    {
        WEAR_LIGHT, WEAR_FINGER, WEAR_FINGER_2, WEAR_NECK, WEAR_NECK,
        WEAR_TORSO,
        WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS, WEAR_ARMS,
        WEAR_SHIELD, WEAR_ABOUT,
        WEAR_WAIST, WEAR_WRIST, WEAR_WRIST_2, WEAR_WIELD, WEAR_HOLD,
        WEAR_FLOAT
    };

    if (val >= 0 && val < sizeof(table) / sizeof(table[0]))
        return table[val];

    return WEAR_NONE;
}

bool import_rom_resets(FILE *fp)
{

    TmpReset *pReset = 0;
    long iLastRoom = 0;
    long iLastObj = 0;

    if (!area_last)
    {
        log_bug("no area seen yet.");
        return false;
    }
    TmpReset *last_reset = 0;

    for (;;)
    {
        Exit *pexit;
        Room *pRoomIndex;
        char letter;

        if ((letter = fread_letter(fp)) == 'S')
            break;

        if (letter == '*')
        {
            fread_to_eol(fp);
            continue;
        }
        pReset = alloc_mem(1, sizeof(TmpReset));

        pReset->type = letter;
        /*
         * if_flag
         */
        fread_number(fp);
        pReset->arg1 = fread_number(fp);
        pReset->arg2 = fread_number(fp);
        pReset->arg3 = (letter == 'G'
                        || letter == 'R') ? 0 : fread_number(fp);
        pReset->arg4 = (letter == 'P'
                        || letter == 'M') ? fread_number(fp) : 0;

        fread_to_eol(fp);

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch (letter)
        {
        default:
            log_bug("bad command '%c'.", letter);
            free_mem(pReset);
            break;

        case 'M':
            if (hm_get(npc_indexes, pReset->arg1) == 0)
            {
                log_bug("bad npc for reset.");
                free_mem(pReset);
                break;
            }
            if ((pRoomIndex = hm_get(room_indexes, pReset->arg3)))
            {

                LINK(temp_resets, pReset, next);
                pReset->room = pRoomIndex;
                iLastRoom = pReset->arg3;
                last_reset = pReset;
            }
            break;

        case 'O':
            if (hm_get(obj_indexes, pReset->arg1) == 0)
            {
                log_bug("bad object for O reset.");
                free_mem(pReset);
                break;
            }
            if ((pRoomIndex = hm_get(room_indexes, pReset->arg3)))
            {
                LINK(temp_resets, pReset, next);
                pReset->room = pRoomIndex;
                iLastObj = pReset->arg3;
                last_reset = pReset;
            }
            break;

        case 'P':
            if (hm_get(obj_indexes, pReset->arg1) == 0)
            {
                log_bug("bad object for P reset.");
                free_mem(pReset);
                break;
            }
            if ((pRoomIndex = hm_get(room_indexes, iLastObj)))
            {
                if (last_reset == 0
                        || (last_reset->type != 'O'
                            && last_reset->type != 'G'
                            && last_reset->type != 'E'))
                {
                    log_bug("bad container reset");
                    free_mem(pReset);
                    break;
                }
                pReset->room = pRoomIndex;
                LINK(last_reset->subresets, pReset, next_sub);
            }
            break;

        case 'G':
            if (hm_get(obj_indexes, pReset->arg1) == 0)
            {
                log_bug("bad object for G reset %ld.",
                        pReset->arg1);
                free_mem(pReset);
                break;
            }
            if ((pRoomIndex = hm_get(room_indexes, iLastRoom)))
            {
                if (last_reset == 0
                        || (last_reset->type != 'M'))
                {
                    log_bug("bad give reset");
                    free_mem(pReset);
                    break;
                }
                pReset->room = pRoomIndex;
                LINK(last_reset->subresets, pReset, next_sub);
                iLastObj = iLastRoom;
            }
            break;

        case 'E':
            if (hm_get(obj_indexes, pReset->arg1) == 0)
            {
                log_bug("bad object for E reset.");
                free_mem(pReset);
                break;
            }
            if ((pRoomIndex = hm_get(room_indexes, iLastRoom)))
            {
                if (last_reset == 0
                        || (last_reset->type != 'M'))
                {
                    log_bug("bad give reset");
                    free_mem(pReset);
                    break;
                }
                // pReset->arg1 = arg3;
                pReset->room = pRoomIndex;
                LINK(last_reset->subresets, pReset, next_sub);
                iLastObj = iLastRoom;
            }
            break;

        case 'D':
            if ((pRoomIndex =
                        hm_get(room_indexes, pReset->arg1)) == 0)
            {
                log_bug("bad room for reset");
                free_mem(pReset);
                break;
            }
            if (pReset->arg2 < 0
                    || pReset->arg2 > 5
                    || !pRoomIndex
                    || !(pexit = pRoomIndex->exits[pReset->arg2])
                    || !is_set(pexit->flags, EXIT_ISDOOR))
            {
                log_bug("exit %ld not door, room %ld.",
                        pReset->arg2, pReset->arg1);
                free_mem(pReset);
                break;
            }
            switch (pReset->arg3)
            {
            default:
                log_bug("bad 'locks': %ld.", pReset->arg3);
            case 0:
                break;
            case 1: /* SET_BIT( pexit->rs_flags,
                     * EX_ISDOOR|EX_CLOSED ); */
                break;
            case 2: /* SET_BIT( pexit->rs_flags,
                     * EX_ISDOOR|EX_CLOSED|EX_LOCKED ); */
                break;
            case 3: /* SET_BIT( pexit->rs_flags,
                     * EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_NOPASS
                     * ); */
                break;
            case 4: /* SET_BIT( pexit->rs_flags,
                     * EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_PICKPROOF
                     * ); */
                break;
            case 5: /* SET_BIT( pexit->rs_flags,
                     * EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_NOPASS|EX_
                     * PICKPROOF ); */
                break;
            }

            pReset->room = pRoomIndex;
            LINK(temp_resets, pReset, next);

            last_reset = pReset;

            break;

        case 'R':
            if (pReset->arg2 < 0 || pReset->arg2 > 6)   /* Last Door. */
            {
                log_bug("bad exit %ld.", pReset->arg2);
                free_mem(pReset);
                break;
            }
            // if (pReset->arg1 < max_room_index && (pRoomIndex =
            // room_indexes[pReset->arg1]))
            // LINK(pRoomIndex->resets, pReset, next);

            break;
        }
    }

    return true;
}

bool import_rom_shops(FILE *fp)
{
    for (;;)
    {
        int iTrade;

        long keeper = fread_number(fp);
        if (keeper == 0)
            break;
        for (iTrade = 0; iTrade < 5; iTrade++)
            fread_number(fp);
        fread_number(fp);
        fread_number(fp);
        fread_number(fp);
        fread_number(fp);
        fread_to_eol(fp);

    }
    return true;
}

bool import_rom_specials(FILE *fp)
{
    for (;;)
    {

        char letter;

        switch (letter = fread_letter(fp))
        {
        default:
            log_bug("letter '%c' not *MS.", letter);
            return false;

        case 'S':
            return true;

        case '*':
            break;

        case 'M':
            fread_number(fp);
            fread_word(fp);

            break;
        }

        fread_to_eol(fp);
    }
    return true;
}

bool import_rom_helps(FILE *fp)
{
    Help *pHelp;

    for (;;)
    {
        pHelp = new_help();
        /*
         * pHelp->level =
         */
        fread_number(fp);
        pHelp->keywords = fread_string(fp);
        if (pHelp->keywords[0] == '$')
        {
            destroy_help(pHelp);
            break;
        }
        pHelp->text = fread_string(fp);
        LINK(import_helps, pHelp, next);

    }
    return true;
}

bool import_rom_socials(FILE *fp)
{
    for (;;)
    {
        const char *temp;

        temp = fread_word(fp);
        if (!strcmp(temp, "#0"))
            return true;    /* done */

        Social *social = new_social();

        social->minPosition = POS_RESTING;
        social->name = str_dup(temp);
        fread_to_eol(fp);

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->charNoArg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->othersNoArg = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->charFound = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->othersFound = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->victFound = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->charNotFound = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->charAuto = temp;

        temp = fread_string_eol(fp);
        if (!strcmp(temp, "#"))
        {
            LINK(import_socials, social, next);
            continue;
        }
        else if (strcmp(temp, "$"))
            social->othersAuto = temp;

        LINK(import_socials, social, next);
    }
    return false;
}

char fread_letter(FILE *fp)
{
    char c;

    do
    {
        c = fgetc(fp);
    }
    while (isspace((int)c));

    return c;
}

long fread_number(FILE *fp)
{
    long number;
    bool sign;
    char c;

    do
    {
        c = fgetc(fp);
    }
    while (isspace((int)c));

    number = 0;

    sign = false;
    if (c == '+')
    {
        c = fgetc(fp);
    }
    else if (c == '-')
    {
        sign = true;
        c = fgetc(fp);
    }
    if (!isdigit((int)c))
    {
        log_bug("bad format '%c'... %s", c, "aborting");

        abort();
    }
    while (isdigit((int)c))
    {
        number = number * 10 + c - '0';
        c = fgetc(fp);
    }

    if (sign)
        number = 0 - number;

    if (c == '|')
        number += fread_number(fp);
    else if (c != ' ')
        ungetc(c, fp);

    return number;
}

long flag_convert(char letter)
{
    long bitsum = 0;
    char i;

    if ('A' <= letter && letter <= 'Z')
    {
        bitsum = 1;
        for (i = letter; i > 'A'; i--)
            bitsum *= 2;
    }
    else if ('a' <= letter && letter <= 'z')
    {
        bitsum = 67108864;  /* 2^26 */
        for (i = letter; i > 'a'; i--)
            bitsum *= 2;
    }
    return bitsum;
}

long fread_flag(FILE *fp)
{
    long number;
    long flag;
    long temp = 1;
    char c;
    bool negative = false;

    do
    {
        c = fgetc(fp);
    }
    while (isspace((int)c));

    if (c != '+')
    {
        if (c == '-')
        {
            negative = true;
            c = fgetc(fp);
        }
        number = 0;

        if (!isdigit((int)c))
        {
            while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'))
            {
                number += flag_convert(c);
                c = fgetc(fp);
            }
        }
        while (isdigit((int)c))
        {
            number = number * 10 + c - '0';
            c = fgetc(fp);
        }

        if (c == '|')
            number += fread_flag(fp);

        else if (c != ' ')
            ungetc(c, fp);

        if (negative)
            return -1 * number;

        return number;
    }
    else
    {
        number = 0;
        flag = 0;
        do
        {
            c = fgetc(fp);
            flag += (temp << number) * (c == 'Y');
            number++;
        }
        while (c == 'Y' || c == 'n');

        if (c == '\n' || c == '\r')
            ungetc(c, fp);

        return flag;
    }
}

void fread_to_eol(FILE *fp)
{
    char c;

    do
    {
        c = fgetc(fp);
    }
    while (c != '\n' && c != '\r');

    do
    {
        c = fgetc(fp);
    }
    while (c == '\n' || c == '\r');

    ungetc(c, fp);
    return;
}

const char *fread_word(FILE *fp)
{
    static char word[OUT_SIZ];
    char *pword;
    char cEnd;

    do
    {
        if (feof(fp))
        {
            log_bug("EOF encountered on read.");
            word[0] = 0;
            return word;
        }
        cEnd = fgetc(fp);
    }
    while (isspace((int)cEnd));

    if (cEnd == '\'' || cEnd == '"')
    {
        pword = word;
    }
    else
    {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for (; pword < word + OUT_SIZ; pword++)
    {
        *pword = fgetc(fp);
        if (cEnd == ' ' ? isspace((int)*pword) : *pword == cEnd)
        {
            if (cEnd == ' ')
                ungetc(*pword, fp);
            *pword = 0;
            return word;
        }
    }

    log_bug("word too long... aborting");

    abort();
    return NULL;
}

const char *fread_string(FILE *fp)
{
    char buf[OUT_SIZ * 4];  /* extra 2 bytes on the end for \0
                 * and 1b slack */
    long i = 0;
    register char c;
    bool sFull = false;

    /*
     * skip blanks
     */
    do
    {
        c = fgetc(fp);
    }
    while (isspace((int)c));

    /*
     * empty string
     */
    if (c == '~')
        return &str_empty[0];

    buf[i++] = c;

    for (;;)
    {
        if (i >= sizeof(buf) && !sFull)
        {
            log_bug("String [%20.20s...] exceeded maximum.", buf);
            sFull = true;
        }
        switch (c = fgetc(fp))
        {
        default:
            if (!sFull)
            {
                buf[i++] = c;
            }
            break;

        case EOF:
        case 0:
            /*
             * temp fix
             */
            log_bug("String [%20.20s...] EOF", buf);
            abort();
            break;

        case '\n':
            if (!sFull)
            {
                buf[i++] = '\n';
                buf[i++] = '\r';
            }
            break;

        case '\r':
            break;

        case '~':
            buf[i] = 0;
            return str_dup(buf);
        }
    }
}

const char *fread_string_eol(FILE *fp)
{
    static bool char_special[256 - EOF];
    char buf[OUT_SIZ * 4];
    long i = 0;
    register char c;
    bool sFull = false;

    if (char_special[EOF - EOF] != true)
    {
        char_special[EOF - EOF] = true;
        char_special['\n' - EOF] = true;
        char_special['\r' - EOF] = true;
    }
    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = fgetc(fp);
    }
    while (isspace((int)c));

    if ((buf[i++] = c) == '\n')
        return &str_empty[0];

    for (;;)
    {
        if (!char_special[(c = fgetc(fp)) - EOF])
        {
            if (!sFull)
                buf[i++] = c;
            continue;
        }
        switch (c)
        {
        default:
            break;
        case EOF:
            log_bug("EOF");
            abort();
            break;

        case '\n':
        case '\r':
            buf[i] = 0;
            return str_dup(buf);
        }
    }
}
