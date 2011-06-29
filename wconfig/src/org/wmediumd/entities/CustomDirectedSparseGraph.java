/*
 *  wconfig - visual configuration tool for wmediumd simulator
 *	Copyright (C) 2011  Javier Lopez (jlopex@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *	02110-1301, USA.
 */

package org.wmediumd.entities;

import edu.uci.ics.jung.graph.DirectedSparseGraph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Pair;

public class CustomDirectedSparseGraph<V, E> extends DirectedSparseGraph<V, E> {

	private static final long serialVersionUID = 877780359322408214L;

	@Override
	public boolean addEdge(E edge, Pair<? extends V> endpoints, EdgeType edgeType) {
		if (endpoints.getFirst().equals(endpoints.getSecond()))
			return false;

		super.addEdge(edge, endpoints, edgeType);
		return true;
	}

	public String toList() {
		StringBuffer sb = new StringBuffer("Edges:");
		sb.append("\n");
		for (E e : getEdges()) {
			Pair<V> ep = getEndpoints(e);
			sb.append(e + "[" + ep.getFirst() + "," + ep.getSecond()
					+ "] ");
		}
		return sb.toString();
	}

	public ProbMatrix toMatrix(int rate) {
		ProbMatrix p = new ProbMatrix(MyNode.edgeCount); 

		for (E e : getEdges()) {

			MyLink ml = (MyLink)e;
			Pair<V> ep = getEndpoints(e);
			p.setValue(Integer.parseInt(ep.getFirst().toString()),
					Integer.parseInt(ep.getSecond().toString()),
					ml.getPloss(rate));	
		}
		return p;
	}
}
