/*
 *	wconfig - visual configuration tool for wmediumd simulator
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

import java.util.Vector;

import org.wmediumd.factories.EdgeFactory;

import edu.uci.ics.jung.graph.util.Pair;

public class ProbMatrixList {
	
	private Vector<ProbMatrix> list = new Vector<ProbMatrix>();

	public void add(ProbMatrix p) {
		list.add(p);
	}
	
	public Pair <MyLink> getEdge(int i, int j) {
		
		boolean symmetric = true;
		MyLink e1 = EdgeFactory.getInstance().create();
		MyLink e2 = EdgeFactory.getInstance().create();
		
		for (int rate=0; rate < MyLink.rates; rate++) {
			ProbMatrix p = list.get(rate);
			if (!p.isLinkSymmetric(i, j)) {
				symmetric = false;
			}
			e1.setPloss(rate, p.getValue(i, j));
			e2.setPloss(rate, p.getValue(j, i));
		}

		if (symmetric)
			return new Pair<MyLink>(e1,e1);
			
		return new Pair<MyLink>(e1,e2);
		
	}

	public int rates() {
		return list.size();
	}

	public int nodes() {
		return list.get(0).size();
	}
	

}
