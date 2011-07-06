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
        this.setText("Delete node " + v.toString());
    }
    
}
