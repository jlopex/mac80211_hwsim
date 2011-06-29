package org.wmediumd.listeners;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public interface EdgeMenuListener<E> {

     void setEdgeAndView(E e, VisualizationViewer<?, E> visView); 
    
}
