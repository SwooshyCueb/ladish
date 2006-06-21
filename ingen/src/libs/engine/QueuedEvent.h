/* This file is part of Ingen.  Copyright (C) 2006 Dave Robillard.
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

#ifndef QUEUEDEVENT_H
#define QUEUEDEVENT_H

#include "Event.h"

namespace Om {

class Responder;
class QueuedEventSource;


/** An Event with a not-time-critical preprocessing stage.
 *
 * These events are events that aren't able to be executed immediately by the
 * Jack thread (because they allocate memory or whatever).  They are pushed
 * on to the QueuedEventQueue where they are preprocessed then pushed on
 * to the realtime Event Queue when they are ready.
 *
 * Lookups for these events should go in the pre_process() method, since they are
 * not time critical and shouldn't waste time in the audio thread doing
 * lookups they can do beforehand.  (This applies for any expensive operation that
 * could be done before the execute() method).
 *
 * \ingroup engine
 */
class QueuedEvent : public Event
{
public:
	/** Process this event into a realtime-suitable event.
	 */
	virtual void pre_process() {
		assert(m_pre_processed == false);
		m_pre_processed = true;
	}

	virtual void execute(samplecount offset) {
		assert(m_pre_processed);
		Event::execute(offset);
	}

	virtual void post_process() {}

	/** If this event blocks the prepare phase of other slow events */
	bool is_blocking() { return m_blocking; }

	bool is_prepared() { return m_pre_processed; }
	
protected:
	// Prevent copies
	QueuedEvent(const QueuedEvent& copy);
	QueuedEvent& operator=(const QueuedEvent&);
	
	QueuedEvent(CountedPtr<Responder> responder, bool blocking = false, QueuedEventSource* source=NULL)
	: Event(responder), m_pre_processed(false), m_blocking(blocking), m_source(source)
	{}
	
	// NULL event base (for internal events only!)
	QueuedEvent()
	: Event(), m_pre_processed(false), m_blocking(false), m_source(NULL)
	{}

	bool               m_pre_processed;
	bool               m_blocking;
	QueuedEventSource* m_source;
};


} // namespace Om

#endif // QUEUEDEVENT_H
