package org.wmediumd.menus;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenuItem;

import org.wmediumd.entities.MyNode;
import org.wmediumd.listeners.VertexMenuListener;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public class DeleteVertexMenuItem<V> extends JMenuItem implements VertexMenuListener<V> {

	private static final long serialVersionUID = -7808205250743340654L;
	private V vertex;
    @SuppressWarnings("unchecked")
	private VisualizationViewer visComp;
    @SuppressWarnings("unchecked")
    /** Creates a new instance of DeleteVertexMenuItem */
    public DeleteVertexMenuItem() {
        super("Delete Vertex");
        this.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e) {
                visComp.getPickedVertexState().pick(vertex, false);
                MyNode node = (MyNode)vertex;
                node.releaseId();
                visComp.getGraphLayout().getGraph().removeVertex(vertex);
                visComp.repaint();
            }
        });
    }

    public void setVertexAndView(V v, VisualizationViewer<V, ?> visComp) {
        this.vertex = v;
        this.visComp = visComp;
        this.setText("Delete Vertex " + v.toString());
    }
    
}
