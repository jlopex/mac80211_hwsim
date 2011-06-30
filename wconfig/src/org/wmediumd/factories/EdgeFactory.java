package org.wmediumd.factories;

import org.apache.commons.collections15.Factory;
import org.wmediumd.entities.MyLink;

public class EdgeFactory implements Factory<MyLink> {

	private static EdgeFactory instance = new EdgeFactory();

	private EdgeFactory() {
		// Nothing to do here 
	}

	public static EdgeFactory getInstance() {
		return instance;
	}

	public MyLink create() {
		return new MyLink();
	}
}
