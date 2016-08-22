#ifndef __LASHD_INTERNAL_ROOM_SUPERVISOR_H__
#define __LASHD_INTERNAL_ROOM_SUPERVISOR_H__

#include "room_supervisor.h"

struct ladish_room_supervisor
{
  struct list_head templates;
  struct list_head rooms;
};

extern struct ladish_room_supervisor g_room_supervisor;

#endif
