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
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import javax.swing.JFileChooser;
import javax.swing.JPanel;

public class FileChooserSave extends JPanel implements ActionListener {

	private static final long serialVersionUID = 5793073038314518180L;
	JFileChooser fc;

	public FileChooserSave(String fileString) {

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

		int returnVal = fc.showSaveDialog(FileChooserSave.this);
		if (returnVal == JFileChooser.APPROVE_OPTION) {
			File file = fc.getSelectedFile();
			Writer output;
			try {
				output = new BufferedWriter(new FileWriter(file));
				output.write(fileString);
				output.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			System.out.println("Saved!");
		} else {
			System.err.println("Save command cancelled by user");
		}

	}

	@Override
	public void actionPerformed(ActionEvent arg0) {
		System.out.println("UALA!");
	}
}
