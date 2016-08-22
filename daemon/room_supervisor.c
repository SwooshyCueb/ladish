/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * LADI Session Handler (ladish)
 *
 * Copyright (C) 2008,2009,2010,2011,2012 Nedko Arnaudov <nedko@arnaudov.name>
 * Copyright (C) 2008 Juuso Alasuutari <juuso.alasuutari@gmail.com>
 * Copyright (C) 2016 Markus Kitsinger <root@swooshalicio.us>
 *
 **************************************************************************
 * This file contains <put something here>
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

/* WIP/RFC code for managing templates.
 *
 **************************************************************************
 * What follows are some thoughts I had on the whole room/template system:
 *
 * I thought it would make the most sense to split off room management into
 * another interface of the studio object. This way we can avoid cluttering the
 * studio or control interface with template management methods and signals.
 *
 * Maybe we should deprecate templates alltogether and modify rooms directly?
 *
 * Another option for avoiding clutter in the interfaces would be to have
 * individual objects for each template? This seems to be trading one kind of
 * clutter for another, though.
 *
 * Template saving/loading?
 *
 * Method for template renaming?
 *
 * Method for template port renaming?
 *
 * Should we deprecate builtin templates?
 *
 */

#include "common.h"
#include "studio.h"
#include "studio_internal.h"
#include "room_supervisor.h"
#include "room_supervisor_internal.h"
#include "control.h"
#include "room.h"
#include "../dbus_constants.h"
#include "cmd.h"
#include "room.h"
#include "../lib/wkports.h"
#include "../proxies/conf_proxy.h"
#include "conf.h"

#define INTERFACE_NAME IFACE_ROOM_SUPERVISOR

struct ladish_room_supervisor g_room_supervisor;

bool
ladish_room_supervisor_init(void)
{

  INIT_LIST_HEAD(&g_room_supervisor.templates);
  INIT_LIST_HEAD(&g_room_supervisor.rooms);

  return true;
}

bool ladish_room_supervisor_clear(void)
{
  /* delete rooms, delete templates */

  return true;
}

void ladish_room_supervisor_uninit(void)
{
  ladish_room_supervisor_clear();
}

static void ladish_dbus_create_room(struct cdbus_method_call * call_ptr)
{
  const char * room_name;
  const char * template_name;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &room_name, DBUS_TYPE_STRING, &template_name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("New room request (name: %s, template: %s)", room_name, template_name);

  if (ladish_command_create_room(call_ptr, ladish_studio_get_cmd_queue(), room_name, template_name))
  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_delete_room(struct cdbus_method_call * call_ptr)
{
  const char * name;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("Delete room request (%s)", name);

  if (ladish_command_delete_room(call_ptr, ladish_studio_get_cmd_queue(), name))
  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_get_room_list(struct cdbus_method_call * call_ptr)
{
  DBusMessageIter iter, array_iter;
  DBusMessageIter struct_iter;
  struct list_head * node_ptr;
  ladish_room_handle room;

  log_info("Get room list request");

  call_ptr->reply = dbus_message_new_method_return(call_ptr->message);
  if (call_ptr->reply == NULL)
  {
    goto fail;
  }

  dbus_message_iter_init_append(call_ptr->reply, &iter);

  if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(sa{sv})", &array_iter))
  {
    goto fail_unref;
  }

  list_for_each(node_ptr, &g_studio.rooms)
  {
    room = ladish_room_from_list_node(node_ptr);

    if (!dbus_message_iter_open_container(&array_iter, DBUS_TYPE_STRUCT, NULL, &struct_iter))
      goto fail_unref;

    //if (!ladish_studio_fill_room_info(&struct_iter, room))
    //  goto fail_unref;

    if (!dbus_message_iter_close_container(&array_iter, &struct_iter))
      goto fail_unref;
  }

  if (!dbus_message_iter_close_container(&iter, &array_iter))
  {
    goto fail_unref;
  }

  return;

fail_unref:
  dbus_message_unref(call_ptr->reply);
  call_ptr->reply = NULL;

fail:
  log_error("Ran out of memory trying to construct method return");
}

static void ladish_dbus_create_template(struct cdbus_method_call * call_ptr)
{
  const char * name;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("New room template request (%s)", name);

  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_delete_template(struct cdbus_method_call * call_ptr)
{
  const char * name;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("Delete room template request (%s)", name);

  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_get_template_list(struct cdbus_method_call * call_ptr)
{
  DBusMessageIter iter, array_iter;

  log_info("Get template list request");

  call_ptr->reply = dbus_message_new_method_return(call_ptr->message);
  if (call_ptr->reply == NULL)
  {
    goto fail;
  }

  dbus_message_iter_init_append(call_ptr->reply, &iter);

  if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "s", &array_iter))
  {
    goto fail_unref;
  }

  /*
  if (!room_templates_enum(&array_iter, room_template_list_filler))
  {
    goto fail_unref;
  }
  */

  if (!dbus_message_iter_close_container(&iter, &array_iter))
  {
    goto fail_unref;
  }

  return;

fail_unref:
  dbus_message_unref(call_ptr->reply);
  call_ptr->reply = NULL;

fail:
  log_error("Ran out of memory trying to construct method return");
}

static void ladish_dbus_template_add_port(struct cdbus_method_call * call_ptr)
{
  const char * template_name;
  const char * port_name;
  const bool is_capture;
  const bool is_midi;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error,
    DBUS_TYPE_STRING, &template_name,
    DBUS_TYPE_STRING, &port_name,
    DBUS_TYPE_BOOLEAN, &is_capture,
    DBUS_TYPE_BOOLEAN, &is_midi,
    DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("Template add port request (template: %s, name: %s, is_capture: %s, is_midi: %s)",
    template_name,
    port_name,
    is_capture ? "true" : "false",
    is_midi ? "true" : "false");

  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_template_remove_port(struct cdbus_method_call * call_ptr)
{
  const char * template_name;
  const char * port_name;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &template_name, DBUS_TYPE_STRING, &port_name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("Template remove port request (template: %s, name: %s)", template_name, port_name);

  {
    cdbus_method_return_new_void(call_ptr);
  }
}

static void ladish_dbus_template_get_port_list(struct cdbus_method_call * call_ptr)
{
  const char * template_name;
  DBusMessageIter iter;
  DBusMessageIter array_iter;

  dbus_error_init(&cdbus_g_dbus_error);

  if (!dbus_message_get_args(call_ptr->message, &cdbus_g_dbus_error, DBUS_TYPE_STRING, &template_name, DBUS_TYPE_INVALID))
  {
    cdbus_error(call_ptr, DBUS_ERROR_INVALID_ARGS, "Invalid arguments to method \"%s\": %s",  call_ptr->method_name, cdbus_g_dbus_error.message);
    dbus_error_free(&cdbus_g_dbus_error);
    return;
  }

  log_info("Template get port list request (%s)", template_name);

  call_ptr->reply = dbus_message_new_method_return(call_ptr->message);
  if (call_ptr->reply == NULL)
  {
    goto fail;
  }

  dbus_message_iter_init_append(call_ptr->reply, &iter);

  if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(sbb)", &array_iter))
  {
    goto fail_unref;
  }

  /* do stuff here */

  if (!dbus_message_iter_close_container(&iter, &array_iter))
  {
    goto fail_unref;
  }

  return;

fail_unref:
  dbus_message_unref(call_ptr->reply);
  call_ptr->reply = NULL;

fail:
  log_error("Ran out of memory trying to construct method return");
}


void emit_room_appeared(ladish_room_handle room)
{
  DBusMessage * message_ptr;
  DBusMessageIter iter;

  message_ptr = dbus_message_new_signal(STUDIO_OBJECT_PATH, INTERFACE_NAME, "RoomAppeared");
  if (message_ptr == NULL)
  {
    log_error("dbus_message_new_signal() failed.");
    return;
  }

  dbus_message_iter_init_append(message_ptr, &iter);

  /*
  if (ladish_studio_fill_room_info(&iter, room))
  {
    cdbus_signal_send(cdbus_g_dbus_connection, message_ptr);
  }
  */

  dbus_message_unref(message_ptr);
}

void emit_room_disappeared(ladish_room_handle room)
{
  DBusMessage * message_ptr;
  DBusMessageIter iter;

  message_ptr = dbus_message_new_signal(STUDIO_OBJECT_PATH, INTERFACE_NAME, "RoomDisappeared");
  if (message_ptr == NULL)
  {
    log_error("dbus_message_new_signal() failed.");
    return;
  }

  dbus_message_iter_init_append(message_ptr, &iter);

  /*
  if (ladish_studio_fill_room_info(&iter, room))
  {
    cdbus_signal_send(cdbus_g_dbus_connection, message_ptr);
  }
  */

  dbus_message_unref(message_ptr);
}

void emit_template_appeared(/* ladish_template_handle template */)
{
  const char * name;

  /*
  cdbus_signal_emit(cdbus_g_dbus_connection,
    STUDIO_OBJECT_PATH,
    INTERFACE_NAME,
    "TemplateAppeared",
    "s",
    &name);
  */
}

void emit_template_disappeared(/* ladish_template_handle template */)
{
  const char * name;

  /*
  cdbus_signal_emit(cdbus_g_dbus_connection,
    STUDIO_OBJECT_PATH,
    INTERFACE_NAME,
    "TemplateDisappeared",
    "s",
    &name);
  */
}

void emit_template_port_added(/* stuff goes here */)
{
  const char * template_name;
  const char * port_name;
  bool is_capture;
  bool is_midi;

  /*
  cdbus_signal_emit(cdbus_g_dbus_connection,
    STUDIO_OBJECT_PATH,
    INTERFACE_NAME,
    "TemplatePortAdded",
    "s",
    &template_name,
    &port_name,
    &is_capture,
    &is_midi);
  */
}

void emit_template_port_removed(/* stuff goes here */)
{
  const char * template_name;
  const char * port_name;
  bool is_capture;
  bool is_midi;

  /*
  cdbus_signal_emit(cdbus_g_dbus_connection,
    STUDIO_OBJECT_PATH,
    INTERFACE_NAME,
    "TemplatePortRemoved",
    "s",
    &template_name,
    &port_name,
    &is_capture,
    &is_midi);
  */
}

CDBUS_METHOD_ARGS_BEGIN(CreateRoom, "Create new studio room")
  CDBUS_METHOD_ARG_DESCRIBE_IN("room_name", "s", "Studio room name")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Room template name")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(DeleteRoom, "Delete studio room")
  CDBUS_METHOD_ARG_DESCRIBE_IN("room_name", "s", "Name of studio room to delete")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(GetRoomList, "Get list of rooms in this studio")
  CDBUS_METHOD_ARG_DESCRIBE_OUT("room_list", "a(sa{sv})", "List of studio rooms: opaths and properties")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(CreateTemplate, "New room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Name of the room template")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(DeleteTemplate, "Delete room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Name of room template to delete")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(GetTemplateList, "Get list of room templates")
  CDBUS_METHOD_ARG_DESCRIBE_OUT("template_list", "as", "List of room templates")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(TemplateAddPort, "Add new port to room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Name of the room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("port_name", "s", "Name of the port")
  CDBUS_METHOD_ARG_DESCRIBE_IN("is_capture", "b", "Is port capture port?")
  CDBUS_METHOD_ARG_DESCRIBE_IN("is_midi", "b", "Is port MIDI port?")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(TemplateRemovePort, "Remove port from room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Name of room template from which to remove port")
  CDBUS_METHOD_ARG_DESCRIBE_IN("port_name", "s", "Name of the port to remove")
CDBUS_METHOD_ARGS_END

CDBUS_METHOD_ARGS_BEGIN(TemplateGetPortList, "Get list of ports in room template")
  CDBUS_METHOD_ARG_DESCRIBE_IN("template_name", "s", "Name of room template from which to list ports")
  CDBUS_METHOD_ARG_DESCRIBE_OUT("port_list", "a(sbb)", "List of ports in template (name, is_capture, is_midi)")
CDBUS_METHOD_ARGS_END

CDBUS_METHODS_BEGIN
  CDBUS_METHOD_DESCRIBE(CreateRoom, ladish_dbus_create_room)
  CDBUS_METHOD_DESCRIBE(DeleteRoom, ladish_dbus_delete_room)
  CDBUS_METHOD_DESCRIBE(GetRoomList, ladish_dbus_get_room_list)
  CDBUS_METHOD_DESCRIBE(CreateTemplate, ladish_dbus_create_template)
  CDBUS_METHOD_DESCRIBE(DeleteTemplate, ladish_dbus_delete_template)
  CDBUS_METHOD_DESCRIBE(GetTemplateList, ladish_dbus_get_template_list)
  CDBUS_METHOD_DESCRIBE(TemplateAddPort, ladish_dbus_template_add_port)
  CDBUS_METHOD_DESCRIBE(TemplateRemovePort, ladish_dbus_template_remove_port)
  CDBUS_METHOD_DESCRIBE(TemplateGetPortList, ladish_dbus_template_get_port_list)
CDBUS_METHODS_END

CDBUS_SIGNAL_ARGS_BEGIN(RoomAppeared, "Room D-Bus object appeared")
  CDBUS_SIGNAL_ARG_DESCRIBE("opath", "s", "room object path")
  CDBUS_SIGNAL_ARG_DESCRIBE("properties", "a{sv}", "room properties")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(RoomChanged, "Room D-Bus object changed")
  CDBUS_SIGNAL_ARG_DESCRIBE("opath", "s", "room object path")
  CDBUS_SIGNAL_ARG_DESCRIBE("properties", "a{sv}", "room properties")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(RoomDisappeared, "Room D-Bus object disappeared")
  CDBUS_SIGNAL_ARG_DESCRIBE("opath", "s", "room object path")
  CDBUS_SIGNAL_ARG_DESCRIBE("properties", "a{sv}", "room properties")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(TemplateAppeared, "")
  CDBUS_SIGNAL_ARG_DESCRIBE("template_name", "s", "room template name")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(TemplateDisappeared, "")
  CDBUS_SIGNAL_ARG_DESCRIBE("template_name", "s", "room template name")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(TemplatePortAdded, "Port added to template")
  CDBUS_SIGNAL_ARG_DESCRIBE("template_name", "s", "room template name")
  CDBUS_SIGNAL_ARG_DESCRIBE("port_name", "s", "template port name")
  CDBUS_SIGNAL_ARG_DESCRIBE("is_capture", "b", "")
  CDBUS_SIGNAL_ARG_DESCRIBE("is_midi", "b", "")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNAL_ARGS_BEGIN(TemplatePortRemoved, "Port removed from template")
  CDBUS_SIGNAL_ARG_DESCRIBE("template_name", "s", "room template name")
  CDBUS_SIGNAL_ARG_DESCRIBE("port_name", "s", "template port name")
  CDBUS_SIGNAL_ARG_DESCRIBE("is_capture", "b", "")
  CDBUS_SIGNAL_ARG_DESCRIBE("is_midi", "b", "")
CDBUS_SIGNAL_ARGS_END

CDBUS_SIGNALS_BEGIN
  CDBUS_SIGNAL_DESCRIBE(RoomAppeared)
  CDBUS_SIGNAL_DESCRIBE(RoomChanged)
  CDBUS_SIGNAL_DESCRIBE(RoomDisappeared)
  CDBUS_SIGNAL_DESCRIBE(TemplateAppeared)
  CDBUS_SIGNAL_DESCRIBE(TemplateDisappeared)
  CDBUS_SIGNAL_DESCRIBE(TemplatePortAdded)
  CDBUS_SIGNAL_DESCRIBE(TemplatePortRemoved)
CDBUS_SIGNALS_END

/*
 * Interface description.
 */

CDBUS_INTERFACE_DEFAULT_HANDLER_METHODS_AND_SIGNALS(g_interface_room_supervisor, INTERFACE_NAME)
