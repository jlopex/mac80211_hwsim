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

import java.util.Comparator;
import java.util.PriorityQueue;

public class MyNode {

	public static PriorityQueue<Integer> ids = new PriorityQueue<Integer>(20, 
		new Comparator<Integer>() {
			public int compare(Integer i, Integer j) {
				int result = i-j;
				return result;
		}
	});
	public static int nodeCount;
	private int id;

	private String mac;

	public MyNode() {

		if (ids.isEmpty())
			ids.offer(nodeCount++);

		this.id = ids.poll();
		this.mac = "42:00:00:00:"+ String.format("%02d", id) + ":00";

	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public void releaseId() {
		ids.offer(Integer.valueOf(getId()));
	}

	public String toString() {
		return String.valueOf(id);
	}

	public String prettyPrint() {
		return "Node: "+ id + " MAC: " + mac;
	}

}