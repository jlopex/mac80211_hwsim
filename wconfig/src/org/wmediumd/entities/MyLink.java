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

public class MyLink {

	public static int linkCount;
	private int id;
	private double[] pLoss;
	public static int rates = 12;

	public MyLink() {
		this.id = linkCount++;
		this.pLoss = new double[rates];
	}

	public String toString() {
		return String.valueOf(id);
	}

	public String prettyPrint() {
		return "Link: "+ id + " with Ploss " + pLoss;
	}

//	public void setPloss(double[] pLoss) {
//		this.pLoss = pLoss;
//	}

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
