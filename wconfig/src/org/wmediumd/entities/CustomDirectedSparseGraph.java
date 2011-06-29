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
