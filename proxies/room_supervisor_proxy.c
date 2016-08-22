#include "common.h"

static void (* g_room_appeared_callback)(const char * opath, const char * name, const char * template) = NULL;
static void (* g_room_disappeared_callback)(const char * opath, const char * name, const char * template) = NULL;
static void (* g_room_changed_callback)(const char * opath, const char * name, const char * template) = NULL;

static void (* g_template_appeared_callback)(const char * name) = NULL;
static void (* g_template_disappeared_callback)(const char * name) = NULL;

static void (* g_template_port_added_callback)(const char * port_name, const char * template_name, bool is_capture, bool is_midi) = NULL;
static void (* g_template_port_removed_callback)(const char * port_name, const char * template_name, bool is_capture, bool is_midi) = NULL;

static bool extract_room_info(DBusMessageIter * iter_ptr, const char ** opath, const char ** name, const char ** template)
{
  dbus_message_iter_get_basic(iter_ptr, opath);
  //log_info("opath is \"%s\"", *opath);
  dbus_message_iter_next(iter_ptr);

  if (!cdbus_iter_get_dict_entry_string(iter_ptr, "name", name))
  {
    log_error("dbus_iter_get_dict_entry() failed");
    return false;
  }
  //log_info("name is \"%s\"", *name);

  if (!cdbus_iter_get_dict_entry_string(iter_ptr, "template", template))
  {
    *template = NULL;
  }
  //log_info("template is \"%s\"", *template);

  return true;
}

static bool extract_room_info_from_signal(DBusMessage * message_ptr, const char ** opath, const char ** name, const char ** template)
{
  const char * signature;
  DBusMessageIter iter;

  signature = dbus_message_get_signature(message_ptr);
  if (strcmp(signature, "sa{sv}") != 0)
  {
    log_error("Invalid signature of room signal");
    return false;
  }

  dbus_message_iter_init(message_ptr, &iter);

  return extract_room_info(&iter, opath, name, template);
}

static void on_room_appeared(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * opath;
  const char * name;
  const char * template;

  log_info("RoomAppeared");

  if (g_room_appeared_callback != NULL && extract_room_info_from_signal(message_ptr, &opath, &name, &template))
  {
    g_room_appeared_callback(opath, name, template);
  }
}

static void on_room_disappeared(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * opath;
  const char * name;
  const char * template;

  log_info("RoomDisappeared");

  if (g_room_disappeared_callback != NULL && extract_room_info_from_signal(message_ptr, &opath, &name, &template))
  {
    g_room_disappeared_callback(opath, name, template);
  }
}

static void on_room_changed(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * opath;
  const char * name;
  const char * template;

  log_info("RoomChanged");

  if (g_room_changed_callback != NULL && extract_room_info_from_signal(message_ptr, &opath, &name, &template))
  {
    g_room_changed_callback(opath, name, template);
  }
}

static bool extract_template_info_from_signal(DBusMessage * message_ptr, const char ** name)
{
  const char * signature;
  DBusMessageIter iter;

  signature = dbus_message_get_signature(message_ptr);
  if (strcmp(signature, "s") != 0)
  {
    log_error("Invalid signature of template signal");
    return false;
  }

  dbus_message_iter_init(message_ptr, &iter);

  dbus_message_iter_get_basic(&iter, name);

  return true;
}

static void on_template_appeared(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * name;

  log_info("TemplateAppeared");

  if (g_template_appeared_callback != NULL && extract_template_info_from_signal(message_ptr, &name))
  {
    g_template_appeared_callback(name);
  }
}

static void on_template_disappeared(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * name;

  log_info("TemplateDisappeared");

  if (g_template_disappeared_callback != NULL && extract_template_info_from_signal(message_ptr, &name))
  {
    g_template_disappeared_callback(name);
  }
}

static bool extract_template_port_info_from_signal(DBusMessage * message_ptr, const char ** template_name, const char ** port_name, const bool * is_capture, const bool * is_midi)
{
  const char * signature;
  DBusMessageIter iter;

  signature = dbus_message_get_signature(message_ptr);
  if (strcmp(signature, "ssbb") != 0)
  {
    log_error("Invalid signature of template port signal");
    return false;
  }

  dbus_message_iter_init(message_ptr, &iter);

  dbus_message_iter_get_basic(&iter, template_name);
  dbus_message_iter_next(&iter);
  dbus_message_iter_get_basic(&iter, port_name);
  dbus_message_iter_next(&iter);
  dbus_message_iter_get_basic(&iter, is_capture);
  dbus_message_iter_next(&iter);
  dbus_message_iter_get_basic(&iter, is_midi);

  return true;
}

static void on_template_port_added(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * template_name;
  const char * port_name;
  const bool is_capture;
  const bool is_midi;

  log_info("TemplatePortAdded");

  if (g_template_port_added_callback != NULL && extract_template_port_info_from_signal(message_ptr, &template_name, &port_name, &is_capture, &is_midi))
  {
    g_template_port_added_callback(template_name, port_name, is_capture, is_midi);
  }
}

static void on_template_port_removed(void * UNUSED(context), DBusMessage * message_ptr)
{
  const char * template_name;
  const char * port_name;
  const bool is_capture;
  const bool is_midi;

  log_info("TemplatePortRemoved");

  if (g_template_port_removed_callback != NULL && extract_template_port_info_from_signal(message_ptr, &template_name, &port_name, &is_capture, &is_midi))
  {
    g_template_port_removed_callback(template_name, port_name, is_capture, is_midi);
  }
}

/* this must be static because it is referenced by the
 * dbus helper layer when hooks are active */
static struct cdbus_signal_hook g_signal_hooks[] =
{
  {"RoomAppeared", on_room_appeared},
  {"RoomDisappeared", on_room_disappeared},
  {"RoomChanged", on_room_changed},
  {"TemplateAppeared", on_template_appeared},
  {"TemplateDisappeared", on_template_disappeared},
  {"TemplatePortAdded", on_template_port_added},
  {"TemplatePortRemoved", on_template_port_removed},
  {NULL, NULL}
};

bool ladish_room_supervisor_proxy_init(void)
{
  if (!cdbus_register_object_signal_hooks(
        cdbus_g_dbus_connection,
        SERVICE_NAME,
        STUDIO_OBJECT_PATH,
        IFACE_ROOM_SUPERVISOR,
        NULL,
        g_signal_hooks))
  {
    log_error("dbus_register_object_signal_hooks() failed");
    return false;
  }

  return true;
}

void ladish_room_supervisor_proxy_uninit(void)
{
  cdbus_unregister_object_signal_hooks(cdbus_g_dbus_connection, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR);
}

void
ladish_app_supervisor_proxy_set_room_callbacks(
  void (* appeared)(const char * opath, const char * name, const char * template),
  void (* disappeared)(const char * opath, const char * name, const char * template),
  void (* changed)(const char * opath, const char * name, const char * template))
{
  DBusMessage * reply_ptr;
  const char * signature;
  DBusMessageIter top_iter;
  DBusMessageIter array_iter;
  DBusMessageIter struct_iter;
  const char * opath;
  const char * name;
  const char * template;

  g_room_appeared_callback = appeared;
  g_room_disappeared_callback = disappeared;
  g_room_changed_callback = changed;

  if (!cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "GetRoomList", "", NULL, &reply_ptr))
  {
    /* Don't log error if there is no studio loaded */
    if (!cdbus_call_last_error_is_name(DBUS_ERROR_UNKNOWN_METHOD))
    {
      log_error("Cannot fetch studio room list: %s", cdbus_call_last_error_get_message());
    }

    return;
  }

  signature = dbus_message_get_signature(reply_ptr);
  if (strcmp(signature, "a(sa{sv})") != 0)
  {
    log_error("Invalid signature of GetRoomList reply");
    goto unref;
  }

  dbus_message_iter_init(reply_ptr, &top_iter);

  for (dbus_message_iter_recurse(&top_iter, &array_iter);
       dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID;
       dbus_message_iter_next(&array_iter))
  {
    dbus_message_iter_recurse(&array_iter, &struct_iter);

    if (!extract_room_info(&struct_iter, &opath, &name, &template))
    {
      log_error("extract_room_info() failed.");
      goto unref;
    }

    g_room_appeared_callback(opath, name, template);
  }

unref:
  dbus_message_unref(reply_ptr);
}

void
ladish_room_supervisor_proxy_set_template_callbacks(
  void (* appeared)(const char * name),
  void (* disappeared)(const char * name))
{
  DBusMessage * reply_ptr;
  const char * signature;
  DBusMessageIter top_iter;
  DBusMessageIter array_iter;
  const char * name;

  g_template_appeared_callback = appeared;
  g_template_disappeared_callback = disappeared;

  if (!cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "GetTemplateList", "", NULL, &reply_ptr))
  {
    /* Don't log error if there is no studio loaded */
    if (!cdbus_call_last_error_is_name(DBUS_ERROR_UNKNOWN_METHOD))
    {
      log_error("Cannot fetch studio template list: %s", cdbus_call_last_error_get_message());
    }

    return;
  }

  signature = dbus_message_get_signature(reply_ptr);
  if (strcmp(signature, "as") != 0)
  {
    log_error("Invalid signature of GetTemplateList reply");
    goto unref;
  }

  dbus_message_iter_init(reply_ptr, &top_iter);

  for (dbus_message_iter_recurse(&top_iter, &array_iter);
       dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID;
       dbus_message_iter_next(&array_iter))
  {
    dbus_message_iter_get_basic(&array_iter, &name);

    g_template_appeared_callback(name);
  }

unref:
  dbus_message_unref(reply_ptr);
}

void
ladish_room_supervisor_proxy_set_template_port_callbacks(
  void (* added)(const char * template_name, const char * port_name, bool is_capture, bool is_midi),
  void (* removed)(const char * template_name, const char * port_name, bool is_capture, bool is_midi))
{
  DBusMessage * templates_reply_ptr;
  DBusMessage * ports_reply_ptr;
  const char * template_signature;
  const char * port_signature;
  DBusMessageIter template_iter;
  DBusMessageIter template_array_iter;
  DBusMessageIter port_iter;
  DBusMessageIter port_array_iter;
  DBusMessageIter port_struct_iter;
  const char * template_name;
  const char * port_name;
  const bool is_capture;
  const bool is_midi;

  g_template_port_added_callback = added;
  g_template_port_removed_callback = removed;

  if (!cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "GetTemplateList", "", NULL, &templates_reply_ptr))
  {
    /* Don't log error if there is no studio loaded */
    if (!cdbus_call_last_error_is_name(DBUS_ERROR_UNKNOWN_METHOD))
    {
      log_error("Cannot fetch studio template list: %s", cdbus_call_last_error_get_message());
    }

    return;
  }

  template_signature = dbus_message_get_signature(templates_reply_ptr);
  if (strcmp(template_signature, "as") != 0)
  {
    log_error("Invalid signature of GetTemplateList reply");
    goto templateunref;
  }

  dbus_message_iter_init(templates_reply_ptr, &template_iter);

  for (dbus_message_iter_recurse(&template_iter, &template_array_iter);
       dbus_message_iter_get_arg_type(&template_array_iter) != DBUS_TYPE_INVALID;
       dbus_message_iter_next(&template_array_iter))
  {
    dbus_message_iter_get_basic(&template_array_iter, &template_name);

    if (!cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "TemplateGetPortList", "s", &template_name, NULL, &ports_reply_ptr))
    {
      log_error("Cannot fetch template port list: %s", cdbus_call_last_error_get_message());

      goto templateunref;
    }

    port_signature = dbus_message_get_signature(ports_reply_ptr);
    if (strcmp(port_signature, "a(sbb)") != 0)
    {
      log_error("Invalid signature of TemplateGetPortList reply");
      goto portunref;
    }

    dbus_message_iter_init(ports_reply_ptr, &port_iter);

    for (dbus_message_iter_recurse(&port_iter, &port_array_iter);
         dbus_message_iter_get_arg_type(&port_array_iter) != DBUS_TYPE_INVALID;
         dbus_message_iter_next(&port_array_iter))
    {
      dbus_message_iter_recurse(&port_array_iter, &port_struct_iter);

      dbus_message_iter_get_basic(&port_struct_iter, &port_name);
      dbus_message_iter_next(&port_struct_iter);
      dbus_message_iter_get_basic(&port_struct_iter, &is_capture);
      dbus_message_iter_next(&port_struct_iter);
      dbus_message_iter_get_basic(&port_struct_iter, &is_midi);

      g_template_port_added_callback(template_name, port_name, is_capture, is_midi);
    }
  }

portunref:
  dbus_message_unref(ports_reply_ptr);

templateunref:
  dbus_message_unref(templates_reply_ptr);
}

bool ladish_room_supervisor_proxy_create_room(const char * name, const char * template)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "CreateRoom", "ss", &name, &template, "");
}

bool ladish_room_supervisor_proxy_delete_room(const char * name)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "DeleteRoom", "s", &name, "");
}

bool ladish_room_supervisor_proxy_create_template(const char * name)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "CreateTemplate", "s", &name, "");
}

bool ladish_room_supervisor_proxy_delete_template(const char * name)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "DeleteTemplate", "s", &name, "");
}

bool ladish_room_supervisor_proxy_template_port_add(const char * template_name, const char * port_name, bool is_capture, bool is_midi)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "TemplateAddPort", "ssbb", &template_name, &port_name, &is_capture, &is_midi, "");
}

bool ladish_room_supervisor_proxy_template_port_remove(const char * template_name, const char * port_name, bool is_capture, bool is_midi)
{
  return cdbus_call(0, SERVICE_NAME, STUDIO_OBJECT_PATH, IFACE_ROOM_SUPERVISOR, "TemplateRemovePort", "ssbb", &template_name, &port_name, &is_capture, &is_midi, "");
}

