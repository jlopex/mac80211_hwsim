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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import org.wmediumd.entities.MyLink;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public class EdgePropertyDialog extends javax.swing.JDialog {

	private static final long serialVersionUID = -6544888894593651971L;

	public enum Rates {
		 _1_Mbps, 
		 _2_Mbps, 
		 _5d5Mbps, 
		 _11_Mbps, 
		 _6_Mbps,
		 _9_Mbps,
		 _12_Mbps,
		 _18_Mbps,
		 _24_Mbps,
		 _36_Mbps,
		 _48_Mbps,
		 _54_Mbps;  //; is required here.

		 @Override public String toString() {
		   //only capitalize the first letter
		   String s = super.toString();
		   s = s.replace('d', '.');
		   s = s.substring(1);
		   s = s.replace('_', ' ');
		   return s;
		 }
	}
	
	List <JTextField> textFields = new LinkedList<JTextField>();
	MyLink edge;

	
	
	/** Creates new form EdgePropertyDialog */
	@SuppressWarnings("unchecked")
	public EdgePropertyDialog(final java.awt.Frame parent, 
			final MyLink edge, final VisualizationViewer visComp) {
		super(parent, true);
		this.edge = edge;
		setTitle("Link: " + edge.toString());

//		this.setSize(340, 290);
		final Container cp = this.getContentPane();

		NumberFormat nf = NumberFormat.getNumberInstance(Locale.US);
    	DecimalFormat df = (DecimalFormat)nf;
    	df.applyPattern("0.000");
		
		Container container = new Container();

		container.setLayout(new GridLayout(12, 2));
		for (int i = 0; i < 12; i++) {
			container.add(new JLabel("Loss prob at " + Rates.values()[i].toString()));
			JTextField textField = new JTextField();
			textField.setText(df.format(edge.getPloss(i)));
			textField.setSelectionStart(0);
			textField.setSelectionEnd(textField.getText().length());
			textFields.add(textField);
			container.add(textField);
		}
		JButton ok=new JButton("OK");
		ok.addActionListener(new ActionListener() {
			
			

			public void actionPerformed(ActionEvent e) {
				
				boolean error = false;
				
				for (int i = 0 ; i < 12; i++) {
					
					String textfield = textFields.get(i).getText();
					
					/* Check if textfield contains a comma 
					   character instead a point character */
					if (textfield.contains(","))
						textfield = textfield.replace(',', '.');
					
					double parsedValue;
					
					try {
						parsedValue = Double.parseDouble(textfield);
					} catch (Exception ex) {
						error = true;
						textFields.get(i).setBackground(Color.ORANGE);
						continue;
					}
					
					if (parsedValue > 1.0 || parsedValue < 0.0) {
						error = true;
						textFields.get(i).setBackground(Color.ORANGE);
					} else {
						edge.setPloss(i,parsedValue);
					}
					
				}
				
				if (error) {
					JOptionPane.showMessageDialog(parent,
							"Check given data, loss probabilities defined from\n" +
							"0.0 to 1.0, where 0.0 means perfect channel\n" + 
							"and 1.0 means a link with 100% frame loss.",
							"Value error",
							JOptionPane.ERROR_MESSAGE);
					parent.repaint();
					return;
				}
				
				
				if (edge.getPlossSum() >= 12)
					visComp.getGraphLayout().getGraph().removeEdge(edge);
				setVisible(false);				
				parent.repaint();
			}
		});
		
		cp.add(container, BorderLayout.NORTH);
		cp.add(ok, BorderLayout.SOUTH);
	
		this.pack();
	}
}
