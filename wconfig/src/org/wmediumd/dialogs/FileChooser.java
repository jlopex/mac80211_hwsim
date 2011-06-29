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

public class FileChooser extends JPanel implements ActionListener {

	private static final long serialVersionUID = 5793073038314518180L;
	JFileChooser fc;

	public FileChooser(String fileString) {

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

		int returnVal = fc.showSaveDialog(FileChooser.this);
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
