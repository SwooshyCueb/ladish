/* This file is part of Machina.
 * Copyright (C) 2007 Dave Robillard <http://drobilla.net>
 * 
 * Machina is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * Machina is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <cassert>
#include <raul/RDFWriter.h>
#include "machina/Node.hpp"
#include "machina/Edge.hpp"

namespace Machina {


Node::Node(FrameCount duration, bool initial)
	: _is_initial(initial)
	, _is_active(false)
	, _enter_time(0)
	, _duration(duration)
{
}


void
Node::add_enter_action(SharedPtr<Action> action)
{
	_enter_action = action;
}


void
Node::remove_enter_action(SharedPtr<Action> /*action*/)
{
	_enter_action.reset();
}



void
Node::add_exit_action(SharedPtr<Action> action)
{
	_exit_action = action;
}


void
Node::remove_exit_action(SharedPtr<Action> /*action*/)
{
	_exit_action.reset();
}

//using namespace std;

void
Node::enter(Timestamp time)
{
	//cerr << "ENTER " << time << endl;
	_is_active = true;
	_enter_time = time;
	if (_enter_action)
		_enter_action->execute(time);
}


void
Node::exit(Timestamp time)
{
	//cerr << "EXIT " << time << endl;
	if (_exit_action)
		_exit_action->execute(time);
	_is_active = false;
	_enter_time = 0;
}


void
Node::add_outgoing_edge(SharedPtr<Edge> edge)
{
	assert(edge->src().lock().get() == this);
	
	_outgoing_edges.push_back(edge);
}


void
Node::remove_outgoing_edge(SharedPtr<Edge> edge)
{
	_outgoing_edges.erase(_outgoing_edges.find(edge));
}


void
Node::write_state(Raul::RDFWriter& writer)
{
	using Raul::RdfId;
	
	writer.write(_id,
			RdfId(RdfId::RESOURCE, "rdf:type"),
			RdfId(RdfId::RESOURCE, "machina:Node"));

	writer.write(_id,
	             RdfId(RdfId::RESOURCE, "machina:duration"),
				 Raul::Atom((int)_duration));
}



} // namespace Machina

