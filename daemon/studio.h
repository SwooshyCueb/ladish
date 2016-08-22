/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * LADI Session Handler (ladish)
 *
 * Copyright (C) 2009, 2010, 2012 Nedko Arnaudov <nedko@arnaudov.name>
 *
 **************************************************************************
 * This file contains interface of the studio singleton object
 **************************************************************************
 *
 * LADI Session Handler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * LADI Session Handler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LADI Session Handler. If not, see <http://www.gnu.org/licenses/>
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef STUDIO_H__0BEDE85E_4FB3_4D74_BC08_C373A22409C0__INCLUDED
#define STUDIO_H__0BEDE85E_4FB3_4D74_BC08_C373A22409C0__INCLUDED

#include "app_supervisor.h"
#include "graph.h"
#include "room.h"
#include "room_supervisor.h"
#include "virtualizer.h"

bool ladish_studio_init(void);
void ladish_studio_uninit(void);
void ladish_studio_run(void);
bool ladish_studio_is_loaded(void);
bool ladish_studio_is_started(void);

bool ladish_studios_iterate(void * call_ptr, void * context, bool (* callback)(void * call_ptr, void * context, const char * studio, uint32_t modtime));
bool ladish_studio_delete(void * call_ptr, const char * studio_name);

void ladish_studio_on_child_exit(pid_t pid, int exit_status);

bool
ladish_studio_iterate_virtual_graphs(
  void * context,
  bool (* callback)(
    void * context,
    ladish_graph_handle graph,
    ladish_app_supervisor_handle app_supervisor));

bool
ladish_studio_iterate_rooms(
  void * context,
  bool (* callback)(
    void * context,
    ladish_room_handle room));

void ladish_studio_stop_app_supervisors(void);
ladish_app_supervisor_handle ladish_studio_find_app_supervisor(const char * opath);
ladish_app_handle ladish_studio_find_app_by_uuid(const uuid_t app_uuid);
struct ladish_cqueue * ladish_studio_get_cmd_queue(void);
ladish_virtualizer_handle ladish_studio_get_virtualizer(void);
ladish_graph_handle ladish_studio_get_jack_graph(void);
ladish_graph_handle ladish_studio_get_studio_graph(void);
ladish_app_supervisor_handle ladish_studio_get_studio_app_supervisor(void);
bool ladish_studio_has_rooms(void);

unsigned int ladish_studio_get_room_index(void);
void ladish_studio_release_room_index(unsigned int index);

void ladish_studio_room_appeared(ladish_room_handle room);
void ladish_studio_room_disappeared(ladish_room_handle room);

ladish_room_handle ladish_studio_find_room_by_uuid(const uuid_t room_uuid_ptr);
bool ladish_studio_check_room_name(const char * room_name);

#endif /* #ifndef STUDIO_H__0BEDE85E_4FB3_4D74_BC08_C373A22409C0__INCLUDED */
