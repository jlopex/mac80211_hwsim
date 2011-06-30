package org.wmediumd.factories;

import org.apache.commons.collections15.Factory;
import org.wmediumd.entities.MyNode;

public class VertexFactory implements Factory<MyNode> {

	private static VertexFactory instance = new VertexFactory();

	private VertexFactory() {
		// Nothing to do here 
	}

	public static VertexFactory getInstance() {
		return instance;
	}

	public MyNode create() {
		return new MyNode();
	}
}
