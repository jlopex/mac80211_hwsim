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

import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Locale;

public class ProbMatrix {
	private int M;             // number of rows
	private int N;             // number of columns
	private double[][] data;   // M-by-N array

	// create M-by-N matrix of 0's
	public ProbMatrix(int size) {
		this.M = size;
		this.N = size;
		data = new double[M][N];
		
		for (int i = 0; i < M ; i++) {
			for (int j = 0; j < N ; j++) {
				if (i ==j)
					data[i][j] = -1;
				else 
					data[i][j] = 1;
			}
		}
	}
	
	public double getValue(int i, int j) {
		return data[i][j];
	}
	
	public void setValue(int i, int j, double value) {
		if (i == j)
			return;
		
		data[i][j] = value;
	}
	
    // print matrix to standard output
    public void show() {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) 
                System.out.printf("%9.4f ", data[i][j]);
            System.out.println();
        }
    }
    
    public String toLinearString() {
    	
    	NumberFormat nf = NumberFormat.getNumberInstance(Locale.US);
    	DecimalFormat df = (DecimalFormat)nf;
    	df.applyPattern("0.000");
    	StringBuffer sb = new StringBuffer("[ ");
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {	
                sb.append(df.format(data[i][j]));
            	sb.append(", ");
            }
        }
        sb.deleteCharAt(sb.length()-2);
        sb.append("]");
        return sb.toString();
    }

	public boolean fromLinearString(String in) {
		
		String[] probs = in.split(",");
		
		if (probs.length > M*N) {
		System.err.println("Something wrong happened parsing prob_matrix, " +
					"check for values and not correct format.");
			return false;
			
		}
	
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N ; j++) {
				data[i][j] = Double.parseDouble(probs[i*M+j]);
			}
		}
		return true;
	}
	
	public boolean isSymmetric() {
		
	    for(int i = 0; i < M; i++)
	    {
	        for(int j = 0; j < N; j++)
	        {
	            if(!isLinkSymmetric(i,j))
	                return false;
	        }
	    }
	    return true;
	}

	public int size() {
		return M;
	}

	public boolean isLinkSymmetric(int i, int j) {
        if(data[i][j] == data[j][i])
            return true;
        return false;
	}

}
