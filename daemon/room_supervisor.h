/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * LADI Session Handler (ladish)
 *
 * Copyright (C) 2008,2009,2010,2011 Nedko Arnaudov <nedko@arnaudov.name>
 * Copyright (C) 2008 Juuso Alasuutari <juuso.alasuutari@gmail.com>
 *
 **************************************************************************
 * This file contains interface to the D-Bus control interface helpers
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

#ifndef __LASHD_DBUS_IFACE_ROOM_SUPERVISOR_H__
#define __LASHD_DBUS_IFACE_ROOM_SUPERVISOR_H__

#include "room.h"

extern const struct cdbus_interface_descriptor g_iface_room_supervisor;

bool ladish_room_supervisor_init(void);
void ladish_room_supervisor_uninit(void);

/*****************************************************************************/
/* Signals                                                                   */
/*****************************************************************************/

void emit_room_appeared(ladish_room_handle room);
void emit_room_disappeared(ladish_room_handle room);
void emit_template_appeared(/* ladish_template_handle template */);
void emit_template_disappeared(/* ladish_template_handle template */);
void emit_template_port_added(/* stuff goes here */);
void emit_template_port_removed(/* stuff goes here */);

#endif /* __LASHD_DBUS_IFACE_ROOM_SUPERVISOR_H__ */
