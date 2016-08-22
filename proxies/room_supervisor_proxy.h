#ifndef __LASHD_DBUS_PROXY_ROOM_SUPERVISOR_H__
#define __LASHD_DBUS_PROXY_ROOM_SUPERVISOR_H__

#include "common.h"

bool ladish_room_supervisor_proxy_init(void);
void ladish_room_supervisor_proxy_uninit(void);

void
ladish_room_supervisor_proxy_set_room_callbacks(
  void (* appeared)(const char * opath, const char * name, const char * template),
  void (* disappeared)(const char * opath, const char * name, const char * template),
  void (* changed)(const char * opath, const char * name, const char * template));

void
ladish_room_supervisor_proxy_set_template_callbacks(
  void (* appeared)(const char * name),
  void (* disappeared)(const char * name));

void
ladish_room_supervisor_proxy_set_template_port_callbacks(
  void (* added)(const char * template_name, const char * port_name, bool is_capture, bool is_midi),
  void (* removed)(const char * template_name, const char * port_name, bool is_capture, bool is_midi));

bool ladish_room_supervisor_proxy_create_room(const char * name, const char * template);
bool ladish_room_supervisor_proxy_delete_room(const char * name);

bool ladish_room_supervisor_proxy_create_template(const char * name);
bool ladish_room_supervisor_proxy_delete_template(const char * name);

bool ladish_room_supervisor_proxy_template_add_port();
bool ladish_room_supervisor_proxy_Template_remove_port();

#endif
