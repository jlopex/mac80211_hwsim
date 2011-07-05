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

package org.wmediumd;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Paint;
import java.awt.Stroke;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.lang.reflect.Method;

import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTextArea;
import javax.swing.UIManager;

import org.apache.commons.collections15.Transformer;
import org.wmediumd.dialogs.FileChooserLoad;
import org.wmediumd.dialogs.FileChooserSave;
import org.wmediumd.entities.MyLink;
import org.wmediumd.entities.MyNode;
import org.wmediumd.entities.ProbMatrix;
import org.wmediumd.factories.EdgeFactory;
import org.wmediumd.factories.VertexFactory;
import org.wmediumd.graphs.CustomSparseGraph;
import org.wmediumd.menus.MyMouseMenus;
import org.wmediumd.plugin.PopupVertexEdgeMenuMousePlugin;

import edu.uci.ics.jung.algorithms.layout.FRLayout;
import edu.uci.ics.jung.algorithms.layout.Layout;
import edu.uci.ics.jung.visualization.VisualizationViewer;
import edu.uci.ics.jung.visualization.control.EditingModalGraphMouse;
import edu.uci.ics.jung.visualization.control.ModalGraphMouse;
import edu.uci.ics.jung.visualization.decorators.ToStringLabeller;
import edu.uci.ics.jung.visualization.renderers.Renderer.VertexLabel.Position;


public class WmediumdGraphView {

	public CustomSparseGraph<MyNode, MyLink> graph;
	private JFrame frame;
	public JTextArea log;

	private VisualizationViewer <MyNode,MyLink> vViewer;
	private EditingModalGraphMouse<MyNode, MyLink> graphMouse;

	public WmediumdGraphView () {

		JMenuBar menuBar;
		JMenu menu;
		JMenuItem menuItem;

		setup();	
		setupVisualization();
		setupMouse();

		// Let's add a menu for changing mouse modes
		menuBar = new JMenuBar();

		menu = new JMenu();
		menu.setText("File");
		menu.setIcon(null); 

		menuItem = new JMenuItem("New file");
		menuItem.setIcon(UIManager.getIcon("FileView.fileIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				// TODO Use the Factory
				MyNode.nodeCount = 0;
				MyLink.linkCount = 0;
				graph.clear();
				frame.repaint();
			}
		});
		menu.add(menuItem);

		menuItem = new JMenuItem("Load file");
		menuItem.setIcon(UIManager.getIcon("FileChooser.upFolderIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				FileChooserLoad fl = new FileChooserLoad();
				if (fl.getMatrixList().rates() == MyLink.rates) {
					MyNode.nodeCount = 0;
					MyLink.linkCount = 0;
					graph.clear();
					graph.setDataFromMatrixList(fl.getMatrixList());
					frame.repaint();
				}

			}
		});
		menu.add(menuItem);


		menuItem = new JMenuItem("Save as...");
		menuItem.setIcon(UIManager.getIcon("FileView.floppyDriveIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				if (MyNode.nodeCount > 1) {
					System.out.println(generateConfigString());
					new FileChooserSave(generateConfigString());
				}
			}
		});
		menu.add(menuItem);

		menu.addSeparator();
		menuItem = new JMenuItem("Exit");
		menuItem.setIcon(UIManager.getIcon("InternalFrame.closeIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				System.exit(0);
			}
		});
		menu.add(menuItem);
		menuBar.add(menu);

		menu = graphMouse.getModeMenu(); // Obtain mode menu from the mouse
		menu.setText("Edit");
		menu.setIcon(null); // I'm using this in a main menu
		menu.setPreferredSize(new Dimension(40,15)); // Change the size
		menuBar.add(menu);

		menu = new JMenu("Help");

		menuItem = new JMenuItem("Help");
		menuItem.setIcon(UIManager.getIcon("FileChooser.detailsViewIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				openURL("http://o11s.org/trac/wiki/MeshTestingWmediumd#a4.1.Usingwconfig");
			}
		});
		menu.add(menuItem);
		menu.addSeparator();

		menuItem = new JMenuItem("About");
		menuItem.setIcon(UIManager.getIcon("FileChooser.homeFolderIcon"));
		menuItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {

				JOptionPane.showMessageDialog(frame,
						"Wireless medium daemon\n" +
						"configuration tool <v0.2b>\n\n" +
						"(C) 2011 - Javier Lopez\n" +
						"<jlopex@gmail.com>\n",
						"About",
						JOptionPane.INFORMATION_MESSAGE);

			}
		});
		menu.add(menuItem);

		menuBar.add(menu);

		graphMouse.setMode(ModalGraphMouse.Mode.EDITING); // Start off in editing mode
		vViewer.setGraphMouse(graphMouse);

		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.setJMenuBar(menuBar);
		frame.getContentPane().add(vViewer);
		frame.pack();
		frame.setVisible(true);
	}

	private void setup () {
		// Create our graph to save all info
		graph = new CustomSparseGraph<MyNode, MyLink>();
		// Set the Frame
		frame = new JFrame("Wmediumd Configuration Tool");
	}

	private void setupVisualization() {
		// Create a Layout to display our graph data
		Layout<MyNode, MyLink> layout = new FRLayout<MyNode, MyLink>(graph);
		layout.setSize(new Dimension(640,480)); // Sets the initial size of the space

		Transformer<MyNode,Paint> vertexPaint = new Transformer<MyNode,Paint>() {

			@Override
			public Paint transform(MyNode arg0) {
				// TODO Auto-generated method stub
				return Color.ORANGE;
			}
		};
		// Transformer to draw links with dark blue color
		//		Transformer<MyLink,Paint> edgePaint = new Transformer<MyLink, Paint>() {
		//
		//			@Override
		//			public Paint transform(MyLink arg0) {
		//				// TODO Auto-generated method stub
		//				return new Color(68, 68, 68);
		//			}
		//		};

		// Set up a new stroke Transformer for the edges
		Transformer<MyLink, Stroke> edgeStrokeTransformer = 
			new Transformer<MyLink, Stroke>() {

			@Override
			public Stroke transform(MyLink s) {

				float plossSum = s.getPlossSumFloat();

				if (plossSum > 0.0f) {
					// For each Link estimate the quality and create a custom stroke
					float dash[] = {12.5f - s.getPlossSumFloat(), s.getPlossSumFloat()};
					return new BasicStroke(1.5f, BasicStroke.CAP_BUTT,
							BasicStroke.JOIN_MITER, 1.0f, dash, 0.0f);
				} else {
					return new BasicStroke(1.5f);
				}
			}
		};


		// Create a Visualization viewer to draw our Layout
		vViewer = new VisualizationViewer<MyNode, MyLink> (layout);
		vViewer.setPreferredSize(new Dimension(640,480)); //Sets the viewing area size
		vViewer.getRenderContext().setVertexFillPaintTransformer(vertexPaint);
		//vViewer.getRenderContext().setEdgeDrawPaintTransformer(edgePaint);
		vViewer.getRenderContext().setEdgeStrokeTransformer(edgeStrokeTransformer);
		vViewer.getRenderContext().setVertexLabelTransformer(new ToStringLabeller<MyNode>());
		vViewer.getRenderer().getVertexLabelRenderer().setPosition(Position.CNTR);


	}

	@SuppressWarnings("unchecked")
	private void setupMouse () {
		// Create a graph mouse and add it to the visualization viewer
		graphMouse =
			new EditingModalGraphMouse<MyNode, MyLink>(vViewer.getRenderContext(),
					VertexFactory.getInstance(), EdgeFactory.getInstance());

		PopupVertexEdgeMenuMousePlugin myPlugin = new PopupVertexEdgeMenuMousePlugin();
		JPopupMenu edgeMenu = new MyMouseMenus.EdgeMenu(frame);
		JPopupMenu vertexMenu = new MyMouseMenus.VertexMenu();
		myPlugin.setEdgePopup(edgeMenu);
		myPlugin.setVertexPopup(vertexMenu);

		graphMouse.remove(graphMouse.getPopupEditingPlugin());  // Removes the existing popup editing plugin
		graphMouse.remove(graphMouse.getEditingPlugin());

		graphMouse.add(myPlugin);   // Add our new plugin to the mouse

		vViewer.addKeyListener(graphMouse.getModeKeyListener());
	}

	private String generateConfigString() {

		StringBuffer sb = new StringBuffer();
		sb.append("ifaces :\n{\n\tcount = ");
		sb.append(MyNode.nodeCount);
		sb.append(";\n\tids = [");

		for (int i = 0; i < MyNode.nodeCount; i++) {
			sb.append("\"42:00:00:00:"+ String.format("%02d", i) + ":00\", ");
		}
		sb.deleteCharAt(sb.length()-2);

		sb.append("];\n};\nprob :\n{\n\trates = ");
		sb.append(MyLink.rates);
		sb.append(";\n\tmatrix_list = (\n");

		for (int i = 0; i < MyLink.rates; i++) {
			ProbMatrix p = graph.toMatrix(i);
			sb.append("\t\t" + p.toLinearString() + ",\n");
		}
		sb.deleteCharAt(sb.length()-2);
		sb.append("\t);\n};");
		return sb.toString();	
	}

	@SuppressWarnings("unchecked")
	private void openURL(String url) {
		String osName = System.getProperty("os.name");
		try {
			if (osName.startsWith("Windows")) {
				Runtime.getRuntime().exec(
						"rundll32 url.dll,FileProtocolHandler " + url);
			} else if (osName.startsWith("Mac OS")) {
				Class fileMgr = Class.forName("com.apple.eio.FileManager");
				Method openURL = fileMgr.getDeclaredMethod("openURL",
						new Class[] {String.class});
				openURL.invoke(null, new Object[] {url});
			} else {
				String[] browsers = { "firefox", "opera", "konqueror",
						"epiphany", "mozilla", "netscape" };
				String browser = null;
				for (int count = 0; count < browsers.length && browser == null; count++)
					if (Runtime.getRuntime().exec(
							new String[] { "which", browsers[count] })
							.waitFor() == 0)
						browser = browsers[count];
				Runtime.getRuntime().exec(new String[] { browser, url });
			}
		} catch (Exception e) {
			JOptionPane.showMessageDialog(null, "Error in opening browser"
					+ ":\n" + e.getLocalizedMessage());
		}
	}

}