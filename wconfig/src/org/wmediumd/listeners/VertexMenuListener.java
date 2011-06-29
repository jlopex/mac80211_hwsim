package org.wmediumd.listeners;

import edu.uci.ics.jung.visualization.VisualizationViewer;

public interface VertexMenuListener<V> {
    void setVertexAndView(V v, VisualizationViewer<V, ?> visView);    
}
