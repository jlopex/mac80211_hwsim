package org.wmediumd.dialogs;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.LinkedList;
import java.util.List;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

import org.wmediumd.entities.MyLink;

public class EdgePropertyDialog extends javax.swing.JDialog {

	private static final long serialVersionUID = -6544888894593651971L;
	List <JTextField> textFields = new LinkedList<JTextField>();
	MyLink edge;

	/** Creates new form EdgePropertyDialog */
	public EdgePropertyDialog(final java.awt.Frame parent, final MyLink edge) {
		super(parent, true);
		this.edge = edge;
		setTitle("Edge: " + edge.toString());

		this.setSize(340, 290);
		final Container cp = this.getContentPane();

		Container cp2 = new Container();

		cp2.setLayout(new GridLayout(12, 2));
		for (int i = 0; i < 12; i++) {
			cp2.add(new JLabel("Loss prob at rate: " + i));

			JTextField textField = new JTextField(String.valueOf(edge.getPloss(i)));
			textFields.add(textField);
			cp2.add(textField);
		}
		cp2.setSize(320, 250);
		JButton ok=new JButton("OK");

		ok.addActionListener(new ActionListener() {

			public void actionPerformed(ActionEvent e) {
				for (int i = 0 ; i < 12; i++) {

					double val = Double.parseDouble(textFields.get(i).getText());
					if (val > 1.0 || val < 0.0) {
						JOptionPane.showMessageDialog(parent,
								"Check given data, loss probabilities defined from\n" +
								"0.0 to 1.0, where 0.0 means perfect channel\n" + 
								"and 1.0 means a link with 100% frame loss.",
								"Value error",
								JOptionPane.ERROR_MESSAGE);
						return;
					}
					edge.setPloss(i,val);
				}
				setVisible(false);
				parent.repaint();
			}
		});
		cp.add(cp2, BorderLayout.NORTH);
		cp.add(ok, BorderLayout.SOUTH);
	}
}
