/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*
 * LADI Session Handler (ladish)
 *
 * Copyright (C) 2010 Nedko Arnaudov <nedko@arnaudov.name>
 *
 **************************************************************************
 * This file contains implementation save releated helper functions
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

#include <unistd.h>

#include "save.h"
#include "escape.h"
#include "studio.h"

bool ladish_write_string(int fd, const char * string)
{
  size_t len;

  len = strlen(string);

  if (write(fd, string, len) != len)
  {
    log_error("write() failed to write config file.");
    return false;
  }

  return true;
}

bool ladish_write_indented_string(int fd, int indent, const char * string)
{
  ASSERT(indent >= 0);
  while (indent--)
  {
    if (!ladish_write_string(fd, LADISH_XML_BASE_INDENT))
    {
      return false;
    }
  }

  if (!ladish_write_string(fd, string))
  {
    return false;
  }

  return true;
}

#define fd (((struct ladish_write_context *)context)->fd)
#define indent (((struct ladish_write_context *)context)->indent)

static
bool
write_dict_entry(
  void * context,
  const char * key,
  const char * value)
{
  if (!ladish_write_indented_string(fd, indent, "<key name=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, key))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\">"))
  {
    return false;
  }

  if (!ladish_write_string(fd, value))
  {
    return false;
  }

  if (!ladish_write_string(fd, "</key>\n"))
  {
    return false;
  }

  return true;
}

static
bool
ladish_save_vgraph_client_begin(
  void * context,
  ladish_graph_handle graph,
  ladish_client_handle client_handle,
  const char * client_name,
  void ** client_iteration_context_ptr_ptr)
{
  uuid_t uuid;
  char str[37];

  ladish_client_get_uuid(client_handle, uuid);
  uuid_unparse(uuid, str);

  log_info("saving vgraph client '%s' (%s)", client_name, str);

  if (!ladish_write_indented_string(fd, indent, "<client name=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, client_name))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" uuid=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, str))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\">\n"))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent + 1, "<ports>\n"))
  {
    return false;
  }

  return true;
}

static
bool
ladish_save_vgraph_client_end(
  void * context,
  ladish_graph_handle graph,
  ladish_client_handle client_handle,
  const char * client_name,
  void * client_iteration_context_ptr)
{
  if (!ladish_write_indented_string(fd, indent + 1, "</ports>\n"))
  {
    return false;
  }

  if (!ladish_write_dict(fd, indent + 1, ladish_client_get_dict(client_handle)))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "</client>\n"))
  {
    return false;
  }

  return true;
}

static bool ladish_get_vgraph_port_uuids(ladish_graph_handle vgraph, ladish_port_handle port, uuid_t uuid, uuid_t link_uuid)
{
  bool link;

  if (vgraph != ladish_studio_get_studio_graph())
  {
    link = false;               /* room ports are saved using their fixed uuids */
  }
  else
  {
    link = ladish_port_is_link(port);
    if (link)
    {
      /* get the generated port uuid that is used for identification in the virtual graph */
      ladish_graph_get_port_uuid(vgraph, port, uuid);
    }
  }

  if (!link || link_uuid != NULL)
  {
    /* get the real port uuid that is same in both room and studio graphs */
    ladish_port_get_uuid(port, link ? link_uuid : uuid);
  }

  return link;
}

static
bool
ladish_save_vgraph_port(
  void * context,
  ladish_graph_handle graph,
  void * client_iteration_context_ptr,
  ladish_client_handle client_handle,
  const char * client_name,
  ladish_port_handle port_handle,
  const char * port_name,
  uint32_t port_type,
  uint32_t port_flags)
{
  uuid_t uuid;
  bool link;
  uuid_t link_uuid;
  char str[37];
  char link_str[37];
  ladish_dict_handle dict;

  link = ladish_get_vgraph_port_uuids(graph, port_handle, uuid, link_uuid);
  uuid_unparse(uuid, str);
  if (link)
  {
    uuid_unparse(link_uuid, link_str);
    log_info("saving vgraph link port '%s':'%s' (%s link=%s)", client_name, port_name, str, link_str);
  }
  else
  {
    log_info("saving vgraph port '%s':'%s' (%s)", client_name, port_name, str);
  }

  if (!ladish_write_indented_string(fd, indent + 2, "<port name=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, port_name))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" uuid=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, str))
  {
    return false;
  }

  if (link)
  {
    if (!ladish_write_string(fd, "\" link_uuid=\""))
    {
      return false;
    }

    if (!ladish_write_string(fd, link_str))
    {
      return false;
    }
  }

  dict = ladish_port_get_dict(port_handle);
  if (ladish_dict_is_empty(dict))
  {
    if (!ladish_write_string(fd, "\" />\n"))
    {
      return false;
    }
  }
  else
  {
    if (!ladish_write_string(fd, "\">\n"))
    {
      return false;
    }

    if (!ladish_write_dict(fd, indent + 3, dict))
    {
      return false;
    }

    if (!ladish_write_indented_string(fd, indent + 2, "</port>\n"))
    {
      return false;
    }
  }

  return true;
}

static
bool
ladish_save_vgraph_connection(
  void * context,
  ladish_graph_handle graph,
  ladish_port_handle port1_handle,
  ladish_port_handle port2_handle,
  ladish_dict_handle dict)
{
  uuid_t uuid;
  char str[37];

  log_info("saving vgraph connection");

  if (!ladish_write_indented_string(fd, indent, "<connection port1=\""))
  {
    return false;
  }

  ladish_get_vgraph_port_uuids(graph, port1_handle, uuid, NULL);
  uuid_unparse(uuid, str);

  if (!ladish_write_string(fd, str))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" port2=\""))
  {
    return false;
  }

  ladish_get_vgraph_port_uuids(graph, port2_handle, uuid, NULL);
  uuid_unparse(uuid, str);

  if (!ladish_write_string(fd, str))
  {
    return false;
  }

  if (ladish_dict_is_empty(dict))
  {
    if (!ladish_write_string(fd, "\" />\n"))
    {
      return false;
    }
  }
  else
  {
    if (!ladish_write_string(fd, "\">\n"))
    {
      return false;
    }

    if (!ladish_write_dict(fd, indent + 1, dict))
    {
      return false;
    }

    if (!ladish_write_indented_string(fd, indent, "</connection>\n"))
    {
      return false;
    }
  }

  return true;
}

static bool ladish_save_app(void * context, const char * name, bool running, const char * command, bool terminal, uint8_t level, pid_t pid)
{
  char buf[100];
  const char * unescaped_string;
  char * escaped_string;
  char * escaped_buffer;
  bool ret;

  log_info("saving app: name='%s', %srunning, %s, level %u, commandline='%s'", name, running ? "" : "not ", terminal ? "terminal" : "shell", (unsigned int)level, command);

  ret = false;

  escaped_buffer = malloc(ladish_max(strlen(name), strlen(command)) * 3 + 1); /* encode each char in three bytes (percent encoding) */
  if (escaped_buffer == NULL)
  {
    log_error("malloc() failed.");
    goto exit;
  }

  if (!ladish_write_indented_string(fd, indent, "<application name=\""))
  {
    goto free_buffer;
  }

  unescaped_string = name;
  escaped_string = escaped_buffer;
  escape(&unescaped_string, &escaped_string);
  *escaped_string = 0;
  if (!ladish_write_string(fd, escaped_buffer))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, "\" terminal=\""))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, terminal ? "true" : "false"))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, "\" level=\""))
  {
    goto free_buffer;
  }

  sprintf(buf, "%u", (unsigned int)level);

  if (!ladish_write_string(fd, buf))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, "\" autorun=\""))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, running ? "true" : "false"))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, "\">"))
  {
    goto free_buffer;
  }

  unescaped_string = command;
  escaped_string = escaped_buffer;
  escape(&unescaped_string, &escaped_string);
  *escaped_string = 0;
  if (!ladish_write_string(fd, escaped_buffer))
  {
    goto free_buffer;
  }

  if (!ladish_write_string(fd, "</application>\n"))
  {
    goto free_buffer;
  }

  ret = true;

free_buffer:
  free(escaped_buffer);

exit:
  return ret;
}

static
bool
ladish_write_room_port(
  void * context,
  ladish_port_handle port,
  const char * name,
  uint32_t type,
  uint32_t flags)
{
  uuid_t uuid;
  char str[37];
  bool midi;
  const char * type_str;
  bool playback;
  const char * direction_str;
  ladish_dict_handle dict;

  ladish_port_get_uuid(port, uuid);
  uuid_unparse(uuid, str);

  playback = (flags & JACKDBUS_PORT_FLAG_INPUT) != 0;
  ASSERT(playback || (flags & JACKDBUS_PORT_FLAG_OUTPUT) != 0); /* playback or capture */
  ASSERT(!(playback && (flags & JACKDBUS_PORT_FLAG_OUTPUT) != 0)); /* but not both */
  direction_str = playback ? "playback" : "capture";

  midi = type == JACKDBUS_PORT_TYPE_MIDI;
  ASSERT(midi || type == JACKDBUS_PORT_TYPE_AUDIO); /* midi or audio */
  ASSERT(!(midi && type == JACKDBUS_PORT_TYPE_AUDIO)); /* but not both */
  type_str = midi ? "midi" : "audio";

  log_info("saving room %s %s port '%s' (%s)", direction_str, type_str, name, str);

  if (!ladish_write_indented_string(fd, indent, "<port name=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, name))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" uuid=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, str))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" type=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, type_str))
  {
    return false;
  }

  if (!ladish_write_string(fd, "\" direction=\""))
  {
    return false;
  }

  if (!ladish_write_string(fd, direction_str))
  {
    return false;
  }

  dict = ladish_port_get_dict(port);
  if (ladish_dict_is_empty(dict))
  {
    if (!ladish_write_string(fd, "\" />\n"))
    {
      return false;
    }
  }
  else
  {
    if (!ladish_write_string(fd, "\">\n"))
    {
      return false;
    }

    if (!ladish_write_dict(fd, indent + 1, dict))
    {
      return false;
    }

    if (!ladish_write_indented_string(fd, indent, "</port>\n"))
    {
      return false;
    }
  }

  return true;
}

#undef indent
#undef fd

bool ladish_write_dict(int fd, int indent, ladish_dict_handle dict)
{
  struct ladish_write_context context;

  if (ladish_dict_is_empty(dict))
  {
    return true;
  }

  context.fd = fd;
  context.indent = indent + 1;

  if (!ladish_write_indented_string(fd, indent, "<dict>\n"))
  {
    return false;
  }

  if (!ladish_dict_iterate(dict, &context, write_dict_entry))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "</dict>\n"))
  {
    return false;
  }

  return true;
}

bool ladish_write_vgraph(int fd, int indent, ladish_graph_handle vgraph, ladish_app_supervisor_handle app_supervisor)
{
  struct ladish_write_context context;

  context.fd = fd;
  context.indent = indent + 1;

  if (!ladish_write_indented_string(fd, indent, "<clients>\n"))
  {
    return false;
  }

  if (!ladish_graph_iterate_nodes(
        vgraph,
        true,
        NULL,
        &context,
        ladish_save_vgraph_client_begin,
        ladish_save_vgraph_port,
        ladish_save_vgraph_client_end))
  {
    log_error("ladish_graph_iterate_nodes() failed");
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "</clients>\n"))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "<connections>\n"))
  {
    return false;
  }

  if (!ladish_graph_iterate_connections(vgraph, true, &context, ladish_save_vgraph_connection))
  {
    log_error("ladish_graph_iterate_connections() failed");
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "</connections>\n"))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "<applications>\n"))
  {
    return false;
  }

  if (!ladish_app_supervisor_enum(app_supervisor, &context, ladish_save_app))
  {
    return false;
  }

  if (!ladish_write_indented_string(fd, indent, "</applications>\n"))
  {
    return false;
  }

  return true;
}

bool ladish_write_room_link_ports(int fd, int indent, ladish_room_handle room)
{
  struct ladish_write_context context;

  context.fd = fd;
  context.indent = indent;

  if (!ladish_room_iterate_link_ports(room, &context, ladish_write_room_port))
  {
    log_error("ladish_room_iterate_link_ports() failed");
    return false;
  }

  return true;
}
