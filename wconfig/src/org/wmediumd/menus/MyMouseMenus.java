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

package org.wmediumd.menus;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Point2D;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import org.wmediumd.dialogs.EdgePropertyDialog;
import org.wmediumd.entities.MyLink;
import org.wmediumd.entities.MyNode;
import org.wmediumd.graphs.CustomSparseGraph;
import org.wmediumd.listeners.EdgeMenuListener;
import org.wmediumd.listeners.MenuPointListener;

import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.visualization.VisualizationViewer;

public class MyMouseMenus {
    
    public static class EdgeMenu extends JPopupMenu {        

		private static final long serialVersionUID = 7606592592742848841L;

        public EdgeMenu(final JFrame frame) {
            super("Edge Menu");
            this.add(new DeleteEdgeMenuItem<MyLink>());
            this.addSeparator();
            this.add(new EdgeCheckBoxItem());
            this.add(new EdgePropItem(frame));           
        }
    }

    @SuppressWarnings("unchecked")
	public static class EdgeCheckBoxItem extends JCheckBoxMenuItem implements EdgeMenuListener<MyLink>,
            MenuPointListener {
    	/**
		 * 
		 */
		private static final long serialVersionUID = 3333189256173310513L;

		MyLink edge;
		
		CustomSparseGraph graph;
		VisualizationViewer visComp;
        Point2D point;
        
        public void setEdgeAndView(MyLink edge, VisualizationViewer visComp) {
            this.edge = edge;
            this.visComp = visComp;
            this.graph = (CustomSparseGraph) visComp.getGraphLayout().getGraph();
            if (graph.getEdgeType(edge).equals(EdgeType.DIRECTED))
            	this.setSelected(true);
            else 
            	this.setSelected(false);
        }

        public void setPoint(Point2D point) {
            this.point = point;
        }
        
        public EdgeCheckBoxItem() {            
            super("Asymmetric");
            this.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    visComp.getPickedEdgeState().pick(edge, false);
                    
                    graph.swapEdgeType(edge);
                    visComp.repaint();
                }
                
            });
        }
        
    }
    
    @SuppressWarnings("unchecked")
	public static class EdgePropItem extends JMenuItem implements EdgeMenuListener<MyLink>,
            MenuPointListener {
    	/**
		 * 
		 */
		private static final long serialVersionUID = 3333189256173310513L;

		MyLink edge;

		VisualizationViewer visComp;
        Point2D point;
        
        public void setEdgeAndView(MyLink edge, VisualizationViewer visComp) {
            this.edge = edge;
            this.visComp = visComp;
        }

        public void setPoint(Point2D point) {
            this.point = point;
        }
        
        public EdgePropItem(final JFrame frame) {            
            super("Edit link Properties...");
            this.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    EdgePropertyDialog dialog = new EdgePropertyDialog(frame, edge, visComp);
                    dialog.setLocation((int)point.getX()+ frame.getX(), (int)point.getY()+ frame.getY());
                    dialog.setVisible(true);
                }
            });
        }
        
    }
    public static class VertexMenu extends JPopupMenu {

		private static final long serialVersionUID = 7893682221815396648L;

		public VertexMenu() {
            super("Vertex Menu");
            this.add(new DeleteVertexMenuItem<MyNode>());
//            this.addSeparator();
//            this.add(new pscCheckBox());
//            this.add(new tdmCheckBox());
        }
		
    }    
}
