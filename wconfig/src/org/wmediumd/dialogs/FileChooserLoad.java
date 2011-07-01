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

package org.wmediumd.dialogs;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

import javax.swing.JFileChooser;
import javax.swing.JPanel;

import org.wmediumd.entities.ProbMatrix;
import org.wmediumd.entities.ProbMatrixList;

public class FileChooserLoad extends JPanel implements ActionListener {

	private static final long serialVersionUID = 5793073038314518180L;
	private JFileChooser fc;
	private ProbMatrixList matrixList = new ProbMatrixList();
	

	public ProbMatrixList getMatrixList() {
		return matrixList;
	}

	public FileChooserLoad() {

		//Create a file chooser
		fc = new JFileChooser();

		//Uncomment one of the following lines to try a different
		//file selection mode.  The first allows just directories
		//to be selected (and, at least in the Java look and feel,
		//shown).  The second allows both files and directories
		//to be selected.  If you leave these lines commented out,
		//then the default mode (FILES_ONLY) will be used.
		//
		//fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		//fc.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);

		int returnVal = fc.showOpenDialog(FileChooserLoad.this);
		
		if (returnVal == JFileChooser.APPROVE_OPTION) {
			System.out.println("Opened!");
			
			if (fc.getSelectedFile() != null) {
				File fileToLoad = fc.getSelectedFile();
				try {
					FileReader fr = new FileReader(fileToLoad);
					BufferedReader br = new BufferedReader(fr);
					
					
					String out;
					int matrixSize = 0;
					boolean inMatrices = false;
					
					while((out = br.readLine()) != null) {

						if (out.contains("count")) {
							String out2[] = out.split(" ");
							String out3 = out2[2].replace(";","");
							matrixSize = Integer.parseInt(out3);
						} else if (out.contains("matrix_list")) {
							inMatrices = true;
						} else if (inMatrices) {
							if (out.length()>3) {
								String out2 = out.substring(4, out.length()-2);
								ProbMatrix p = new ProbMatrix(matrixSize);
								p.fromLinearString(out2);
								matrixList.add(p);
							}
						}
					}
					fr.close();
					
				} catch (Exception e) {
					e.printStackTrace();
					return;
				} 
			} 
			
			
		} 

	}

	@Override
	public void actionPerformed(ActionEvent arg0) {
		System.out.println("UALA!");
	}
}
