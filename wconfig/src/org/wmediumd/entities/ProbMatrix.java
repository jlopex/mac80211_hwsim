package org.wmediumd.entities;

import java.text.DecimalFormat;

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
    	
    	DecimalFormat df = new DecimalFormat("0.000");
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

}
