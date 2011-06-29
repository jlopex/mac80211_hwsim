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
	public static int edgeCount;
	private int id;

	private String mac;

	public MyNode() {

		if (ids.isEmpty())
			ids.offer(edgeCount++);

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