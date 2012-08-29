
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

#include <stdio.h>
#include <time.h>
#include <muddyengine/character.h>
#include <muddyengine/player.h>
#include <muddyengine/engine.h>
#include <muddyengine/forum.h>
#include <muddyengine/string.h>
#include <muddyengine/log.h>
#include <muddyengine/buffer.h>
#include <muddyengine/util.h>
#include <muddyengine/account.h>
#include <muddyengine/connection.h>
#include <muddyengine/db.h>
#include <inttypes.h>

Forum *forum_table = 0;

int max_forum = 0;

time_t last_note_stamp = 0;

Note *new_note()
{

	Note *note = (Note *) alloc_mem(1, sizeof(Note));

	note->toList = str_empty;

	note->from = str_empty;

	note->subject = str_empty;

	note->text = str_empty;

	return note;

}

void destroy_note(Note * note)
{

	free_str(note->toList);

	free_str(note->from);

	free_str(note->subject);

	free_str(note->text);

	free_mem(note);

}

void finish_note(Forum * forum, Note * note)
{

	if (last_note_stamp >= current_time) {

		note->date = ++last_note_stamp;

	} else {

		note->date = current_time;

		last_note_stamp = current_time;

	}

	LINK_LAST(forum->notes, Note, note, next);

	forum->flags |= FORUM_CHANGED;

}

bool is_note_to(Character * ch, Note * note)
{

	if (!str_cmp(ch->name, note->from))
		return true;

	if (is_exact_name("all", note->toList))
		return true;

	if (is_immortal(ch)
	    && (is_exact_name("imm", note->toList)
		|| is_exact_name("imms", note->toList)
		|| is_exact_name("immortal", note->toList)
		|| is_exact_name("god", note->toList)
		|| is_exact_name("gods", note->toList)
		|| is_exact_name("imp", note->toList)
		|| is_exact_name("imps", note->toList)
		|| is_exact_name("implementor", note->toList)
		|| is_exact_name("implementors", note->toList)
		|| is_exact_name("immortals", note->toList)))
		return true;

	if (is_exact_name(ch->name, note->toList))
		return true;

	if (is_number(note->toList) && ch->level >= atoi(note->toList))
		return true;

	return false;

}

int lookup_forum_by_id(const identifier_t id)
{

	for (int i = 0; i < max_forum; i++)
		if (id == forum_table[i].id)
			return i;

	return FORUM_ERROR;

}

int lookup_forum_by_name(const char *name)
{

	for (int i = 0; i < max_forum; i++)
		if (!str_cmp(forum_table[i].name, name))
			return i;

	return FORUM_ERROR;

}

Note *find_note(Character * ch, Forum * forum, int num)
{

	int count = 0;

	Note *p;

	for (p = forum->notes; p; p = p->next)
		if (++count == num)
			break;

	if ((count == num) && is_note_to(ch, p))
		return p;

	else
		return NULL;

}

void show_note_to_char(Character * ch, Note * note, int num)
{

	Buffer *buffer = new_buf();

	writeln(buffer, "");

	writelnf(buffer, "~YForum~x: %s", ch->pc->account->forum->name);

	writelnf(buffer, "[~x%4d~x] ~Y%s~x: ~g%s~x", num, note->from,
		 note->subject);

	writelnf(buffer, "~YDate~x:  %s",
		 str_time(note->date, ch->pc->account->timezone, 0));

	writelnf(buffer, "~YTo~x:    %s", note->toList);

	writelnf(buffer, "~g%s~x", fillstr(0, scrwidth(ch)));

	writelnf(buffer, "%s", note->text);

	ch->page(ch, buf_string(buffer));

}

void check_notes(Forum * b)
{

	for (Note * p_next, *p = b->notes; p; p = p_next) {

		p_next = p->next;

		if (p->expire < current_time) {

			UNLINK(b->notes, Note, p, next);

			destroy_note(p);

			b->flags |= FORUM_CHANGED;

		}
	}

}

bool is_subscribed(Account * acc, int forum)
{

	return !acc->forumData[forum].unsubscribed;

}

void set_subscribed(Account * acc, int forum)
{

	acc->forumData[forum].unsubscribed = false;

}

void remove_subscribed(Account * acc, int forum)
{

	acc->forumData[forum].unsubscribed = true;

}

int unread_notes(Character * ch, Forum * forum)
{

	int f = lookup_forum_by_id(forum->id);

	if (f == FORUM_ERROR || forum->readLevel > ch->level)
		return FORUM_ERROR;

	time_t last_read = ch->pc->account->forumData[f].lastNote;

	int count = 0;

	for (Note * note = forum->notes; note; note = note->next)
		if (is_note_to(ch, note)
		    && ((long)last_read < (long)note->date))
			count++;

	return count;

}

Note *last_note(Character * ch, Forum * forum)
{

	if (forum->readLevel > ch->level)
		return NULL;

	for (Note * note = forum->notes; note; note = note->next)
		if (is_note_to(ch, note))
			return note;

	return NULL;

}

void next_forum(Character * ch)
{

	int i = lookup_forum_by_id(ch->pc->account->forum->id) + 1;

	while ((i < max_forum)
	       && (unread_notes(ch, &forum_table[i]) == FORUM_ERROR
		   || !is_subscribed(ch->pc->account, i)))
		i++;

	if (i == max_forum) {

		write(ch, "End of forums. ");

		i = 0;

	}
	ch->pc->account->forum = &forum_table[i];

	return;

}

void note_check(Character * ch)
{

	int i, count = 0, unread = 0;

	for (i = 0; i < max_forum; i++) {

		unread = unread_notes(ch, &forum_table[i]);

		if (unread != FORUM_ERROR && !is_subscribed(ch->pc->account, i))
			count += unread;

	}

	if (count < 1)
		writeln(ch, "You have no new notes on the forum.");

	else
		writelnf(ch, "You have %d unread note(s) on the forum.", count);

}

void show_forum(Character * ch, bool fAll)
{

	int unread, count, i, last;

	Note *p;

	Forum *b;

	bool found = false;

	count = 0;

	writeln(ch, "~RNum         Name  Unread Last Description~x");

	writeln(ch, "~R=== ============= ====== ==== ===========~x");

	for (i = 0; i < max_forum; i++) {

		unread = unread_notes(ch, &forum_table[i]);

		if (unread == FORUM_ERROR)
			continue;

		count++;

		if (!is_subscribed(ch->pc->account, i))
			continue;

		if (unread == 0 && fAll == false)
			continue;

		last = 0;

		b = &forum_table[i];

		for (Note * p = b->notes; p; p = p->next)
			if (is_note_to(ch, p))
				last++;

		found = true;

		writelnf(ch, "~W%2d~x> ~G%13s~x [~%c%4d~x] ~G%4d ~Y%s~x",
			 count, forum_table[i].name,
			 unread == 0 ? 'r' : 'R',
			 unread, last, forum_table[i].description);

	}

	if (!found) {

		writeln(ch, "You have no unread notes on any subscribed forum");

	}
	writeln(ch, "");

	writef(ch, "Your current forum is ~W%s~x",
	       ch->pc->account->forum->name);

	if ((p = last_note(ch, ch->pc->account->forum)) != NULL)
		writelnf(ch, ". Last message was from ~W%s~x.", p->from);

	else
		writeln(ch, ".");

	if (ch->pc->account->forum->readLevel > ch->level)
		writeln(ch, "You cannot read nor write notes on this forum.");

	else if (ch->pc->account->forum->writeLevel > ch->level)
		writeln(ch, "You can only read notes from this forum.");

	else
		writeln(ch, "You can both read and write on this forum.");

	writeln(ch, "Use 'forum all' to see all subscribed forums.");

	writeln(ch,
		"Use 'subscribe' to see what forums you are subscribed to.");

}

void
make_note(const char *forum_name, const char *sender,
	  const char *to, const char *subject,
	  const int expire_days, const char *text)
{

	int forum_index = lookup_forum_by_name(forum_name);

	Forum *forum;

	Note *note;

	if (forum_index == FORUM_ERROR) {

		log_bug("forum not found");

		return;

	}
	forum = &forum_table[forum_index];

	note = new_note();

	note->from = str_dup(sender);

	note->toList = str_dup(to);

	note->subject = str_dup(subject);

	note->expire = current_time + (time_t) (expire_days * 24 * 60 * 60);

	note->text = str_dup(text);

	note->date = current_time;

	finish_note(forum, note);

}

int delete_note(Forum * forum, Note * note)
{

	char buf[BUF_SIZ];

	sprintf(buf, "delete from note where noteId=%" PRId64, note->id);

	if (sql_exec(buf) != SQL_OK) {

		log_data("could not delete note");

		return 0;

	}
	UNLINK(forum->notes, Note, note, next);

	destroy_note(note);

	return 1;

}

int load_notes(Forum * forum)
{

	char buf[BUF_SIZ];

	sql_stmt *stmt;

	int total = 0;

	int len = sprintf(buf, "select * from note where forumId=%" PRId64,
			  forum->id);

	if (sql_query(buf, len, &stmt) != SQL_OK) {

		log_data("could not prepare statement");

		return 0;

	}
	while (sql_step(stmt) != SQL_DONE) {

		int count = sql_column_count(stmt);

		Note *note = new_note();

		for (int i = 0; i < count; i++) {

			const char *colname = sql_column_name(stmt, i);

			if (!str_cmp(colname, "noteId")) {

				note->id = sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "forumId")) {

				if (forum->id != sql_column_int(stmt, i))
					log_error
					    ("sql returned invalid note for forum");

			} else if (!str_cmp(colname, "toList")) {

				note->toList = str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "sender")) {

				note->from = str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "subject")) {

				note->subject =
				    str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "text")) {

				note->text = str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "timestamp")) {

				note->date = sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "expire")) {

				note->expire = sql_column_int(stmt, i);

			} else {

				log_warn("unknown note column '%s'", colname);

			}

		}

		LINK_LAST(forum->notes, Note, note, next);

		total++;

	}

	if (sql_finalize(stmt) != SQL_OK) {

		log_data("could not finalize statement");

	}
	return total;

}

int load_forums()
{

	char buf[BUF_SIZ];

	sql_stmt *stmt;

	int total = 0;

	int len = sprintf(buf, "select count(*) from forum");

	if (sql_query(buf, len, &stmt) != SQL_OK) {

		log_data("could not prepare statement");

		return 0;

	}
	if (sql_step(stmt) == SQL_DONE) {

		log_data("could not count forums");

		return 0;

	}
	max_forum = sql_column_int(stmt, 0);

	if (sql_finalize(stmt) != SQL_OK) {

		log_data("could not finalize statement");

	}
	forum_table = (Forum *) alloc_mem(max_forum, sizeof(Forum));

	len = sprintf(buf, "select * from forum");

	if (sql_query(buf, len, &stmt) != SQL_OK) {

		log_data("could not prepare statement");

		return 0;

	}
	while (sql_step(stmt) != SQL_DONE) {

		int count = sql_column_count(stmt);

		for (int i = 0; i < count; i++) {

			const char *colname = sql_column_name(stmt, i);

			if (!str_cmp(colname, "forumId")) {

				forum_table[total].id = sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "name")) {

				forum_table[total].name =
				    str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "description")) {

				forum_table[total].description =
				    str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "defaultTo")) {

				forum_table[total].defaultTo =
				    str_dup(sql_column_str(stmt, i));

			} else if (!str_cmp(colname, "readLevel")) {

				forum_table[total].readLevel =
				    sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "writeLevel")) {

				forum_table[total].writeLevel =
				    sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "type")) {

				forum_table[total].type =
				    sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "purgeDays")) {

				forum_table[total].purgeDays =
				    sql_column_int(stmt, i);

			} else if (!str_cmp(colname, "flags")) {

				forum_table[total].flags =
				    sql_column_int(stmt, i);

			} else {

				log_warn("unknown note column '%s'", colname);

			}

		}

		load_notes(&forum_table[total]);

		total++;

	}

	if (sql_finalize(stmt) != SQL_OK) {

		log_data("could not finalize statement");

	}
	return total;

}

int save_note(Forum * forum, Note * note)
{
	field_map note_values[] = {
		{"text", &note->text, SQL_TEXT},
		{"subject", &note->subject, SQL_TEXT},
		{"date_stamp", &note->date, SQL_INT},
		{"expire", &note->expire, SQL_INT},
		{"toList", &note->toList, SQL_TEXT},
		{"sender", &note->from, SQL_TEXT},
		{"forumId", &forum->id, SQL_INT},
		{0}
	};

	if (note->id == 0) {
		if (sql_insert_query(note_values, "note") != SQL_OK) {

			log_data("could not insert note");

			return 0;

		}
		note->id = db_last_insert_rowid();

	} else {
		if (sql_update_query(note_values, "note", note->id) != SQL_OK) {

			log_data("could not update note");

			return 0;

		}
	}

	return 1;

}

void save_forums()
{

	for (int i = 0; i < max_forum; i++) {

		if (!(forum_table[i].flags & FORUM_CHANGED))
			continue;

		for (Note * p = forum_table[i].notes; p; p = p->next) {

			save_note(&forum_table[i], p);

		}

	}

}
