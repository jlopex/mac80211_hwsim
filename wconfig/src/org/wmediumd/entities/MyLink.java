package org.wmediumd.entities;

public class MyLink {

	public static int edgeCount;
	private int id;
	private double[] pLoss;
	public static int rates = 12;

	public MyLink() {
		this.id = edgeCount++;
		this.pLoss = new double[rates];
	}

	public String toString() {
		return String.valueOf(id);
	}

	public String prettyPrint() {
		return "Link: "+ id + " with Ploss " + pLoss;
	}


	public void setPloss(int pos, double value) {
		this.pLoss[pos] = value;
	}

	public double getPloss(int i) {
		return pLoss[i];
	}

	public double getPlossSum() {
		double ret = 0;
		for (double val : pLoss) {
			ret=ret+val;
		}
		return ret;
	}

	public float getPlossSumFloat() {
		String ret = Double.toString(getPlossSum());
		return Float.parseFloat(ret);
	}
}
