/* This file is part of Ingen.  Copyright (C) 2006 Mario Lang.
 * 
 * Om is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * Om is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <jack/jack.h>

#include "Om.h"
#include "OmApp.h"
#include "OSCReceiver.h"
#include "JackAudioDriver.h"
#ifdef HAVE_LASH
#include "LashDriver.h"
#endif

extern "C"
{
	int  jack_initialize(jack_client_t* client, const char* load_init);
	void jack_finish(void* arg);
}


void*
run_main(void* arg)
{
	Om::om->main();
#ifdef HAVE_LASH

	delete Om::lash_driver;
#endif

	delete Om::om;
	return 0;
}


pthread_t main_thread;


int
jack_initialize(jack_client_t* client, const char* load_init)
{
	if ((Om::om = new Om::OmApp(load_init, new Om::JackAudioDriver(client))) != NULL) {
		pthread_create(&main_thread, NULL, run_main, NULL);
		return 0; // Success
	} else {
		return 1;
	}
}


void
jack_finish(void* arg)
{
	void* ret;
	Om::om->quit();
	pthread_join(main_thread, &ret);
}

