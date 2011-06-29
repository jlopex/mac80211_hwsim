package org.wmediumd.menus;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Point2D;

import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import org.wmediumd.dialogs.EdgePropertyDialog;
import org.wmediumd.entities.MyLink;
import org.wmediumd.entities.MyNode;
import org.wmediumd.listeners.EdgeMenuListener;
import org.wmediumd.listeners.MenuPointListener;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public class MyMouseMenus {
    
    public static class EdgeMenu extends JPopupMenu {        

		private static final long serialVersionUID = 7606592592742848841L;

        public EdgeMenu(final JFrame frame) {
            super("Edge Menu");
            this.add(new DeleteEdgeMenuItem<MyLink>());
            this.addSeparator();
            this.add(new EdgePropItem(frame));           
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
        
        public  EdgePropItem(final JFrame frame) {            
            super("Edit Edge Properties...");
            this.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    EdgePropertyDialog dialog = new EdgePropertyDialog(frame, edge);
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
