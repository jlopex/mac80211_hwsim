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
