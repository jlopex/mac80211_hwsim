package org.wmediumd.menus;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenuItem;

import org.wmediumd.listeners.EdgeMenuListener;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public class DeleteEdgeMenuItem<E> extends JMenuItem implements EdgeMenuListener<E> {

	private static final long serialVersionUID = -4598105903956637248L;
	private E edge;
    private VisualizationViewer<?, E> visComp;
    
    /* Creates a new instance of DeleteEdgeMenuItem */
    public DeleteEdgeMenuItem() {
        super("Delete Edge");
        this.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e) {
                visComp.getPickedEdgeState().pick(edge, false);
                visComp.getGraphLayout().getGraph().removeEdge(edge);
                visComp.repaint();
            }
        });
    }

    public void setEdgeAndView(E edge, VisualizationViewer<?, E> visComp) {
        this.edge = edge;
        this.visComp = visComp;
        this.setText("Delete Edge " + edge.toString());
    } 
}
