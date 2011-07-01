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

package org.wmediumd.graphs;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Vector;

import org.apache.commons.collections15.Factory;
import org.wmediumd.entities.MyLink;
import org.wmediumd.entities.MyNode;
import org.wmediumd.entities.ProbMatrix;
import org.wmediumd.entities.ProbMatrixList;
import org.wmediumd.factories.EdgeFactory;
import org.wmediumd.factories.VertexFactory;

import edu.uci.ics.jung.graph.AbstractGraph;
import edu.uci.ics.jung.graph.Graph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Pair;

/**
 * An implementation of <code>Graph</code> that is suitable for wmediumd graphs and
 * permits both directed and undirected edges and swapping between EdgeTypes
 */
@SuppressWarnings("serial")
public class CustomSparseGraph<V,E> 
    extends AbstractGraph<V,E> 
    implements Graph<V,E>
{
    /**
     * Returns a {@code Factory} that creates an instance of this graph type.
     * @param <V> the vertex type for the graph factory
     * @param <E> the edge type for the graph factory
     */
    public static <V,E> Factory<Graph<V,E>> getFactory() 
    { 
        return new Factory<Graph<V,E>> () 
        {
            public Graph<V,E> create() 
            {
                return new CustomSparseGraph<V,E>();
            }
        };
    }

    protected static final int INCOMING = 0;
    protected static final int OUTGOING = 1;
    protected static final int INCIDENT = 2;
    
    protected Map<V, Map<V,E>[]> vertex_maps; // Map of vertices to adjacency maps of vertices to {incoming, outgoing, incident} edges
    protected Map<E, Pair<V>> directed_edges;    // Map of directed edges to incident vertex sets
    protected Map<E, Pair<V>> undirected_edges;    // Map of undirected edges to incident vertex sets
    
    /**
     * Creates an instance.
     */
    public CustomSparseGraph()
    {
        vertex_maps = new HashMap<V, Map<V,E>[]>();
        directed_edges = new HashMap<E, Pair<V>>();
        undirected_edges = new HashMap<E, Pair<V>>();
    }
    
    @Override
    public E findEdge(V v1, V v2)
    {
        if (!containsVertex(v1) || !containsVertex(v2))
            return null;
        E edge = vertex_maps.get(v1)[OUTGOING].get(v2);
        if (edge == null)
            edge = vertex_maps.get(v1)[INCIDENT].get(v2);
        return edge;
    }

    @Override
    public Collection<E> findEdgeSet(V v1, V v2)
    {
        if (!containsVertex(v1) || !containsVertex(v2))
            return null;
        Collection<E> edges = new ArrayList<E>(2);
        E e1 = vertex_maps.get(v1)[OUTGOING].get(v2);
        if (e1 != null)
            edges.add(e1);
        E e2 = vertex_maps.get(v1)[INCIDENT].get(v2);
        if (e1 != null)
            edges.add(e2);
        return edges;
    }
    
    @Override
    public boolean addEdge(E edge, Pair<? extends V> endpoints, EdgeType edgeType)
    {
    	// If Ploss is 100% No loop possible
    	MyLink l = (MyLink)edge;
    	
    	if (l.getPlossSum() == 12) {
    		return false;
    	}
    	
    	// No internal loops possible
		if (endpoints.getFirst().equals(endpoints.getSecond()))
			return false;
    	
        Pair<V> new_endpoints = getValidatedEndpoints(edge, endpoints);
        if (new_endpoints == null)
            return false;
        
        V v1 = new_endpoints.getFirst();
        V v2 = new_endpoints.getSecond();
        
        // undirected edges and directed edges are considered to be parallel to each other,

        E connection = findEdge(v1, v2);
        if (connection != null )// && getEdgeType(connection) == edgeType)
            return false;
        
        // If there's a directional in opposite direction
        // modify the new edgeType to directional
        if (edgeType.equals(EdgeType.UNDIRECTED)) {
        	if (findEdge(v2, v1) != null)
        		edgeType = EdgeType.DIRECTED;
        }
        

        if (!containsVertex(v1))
            this.addVertex(v1);
        
        if (!containsVertex(v2))
            this.addVertex(v2);
        
        // map v1 to <v2, edge> and vice versa
        if (edgeType == EdgeType.DIRECTED)
        {
            vertex_maps.get(v1)[OUTGOING].put(v2, edge);
            vertex_maps.get(v2)[INCOMING].put(v1, edge);
            directed_edges.put(edge, new_endpoints);
        }
        else
        {
            vertex_maps.get(v1)[INCIDENT].put(v2, edge);
            vertex_maps.get(v2)[INCIDENT].put(v1, edge);
            undirected_edges.put(edge, new_endpoints);
        }
        
        return true;
    }

    
    
    public Collection<E> getInEdges(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        
        // combine directed inedges and undirected
        Collection<E> in = new HashSet<E>(vertex_maps.get(vertex)[INCOMING].values());
        in.addAll(vertex_maps.get(vertex)[INCIDENT].values());
        return Collections.unmodifiableCollection(in);
    }

    public Collection<E> getOutEdges(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        
        // combine directed outedges and undirected
        Collection<E> out = new HashSet<E>(vertex_maps.get(vertex)[OUTGOING].values());
        out.addAll(vertex_maps.get(vertex)[INCIDENT].values());
        return Collections.unmodifiableCollection(out);
    }

    public Collection<V> getPredecessors(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        
        // consider directed inedges and undirected
        Collection<V> preds = new HashSet<V>(vertex_maps.get(vertex)[INCOMING].keySet());
        preds.addAll(vertex_maps.get(vertex)[INCIDENT].keySet());
        return Collections.unmodifiableCollection(preds);
    }

    public Collection<V> getSuccessors(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        
        // consider directed outedges and undirected
        Collection<V> succs = new HashSet<V>(vertex_maps.get(vertex)[OUTGOING].keySet());
        succs.addAll(vertex_maps.get(vertex)[INCIDENT].keySet());
        return Collections.unmodifiableCollection(succs);
    }

    public Collection<E> getEdges(EdgeType edgeType)
    {
        if (edgeType == EdgeType.DIRECTED)
            return Collections.unmodifiableCollection(directed_edges.keySet());
        else if (edgeType == EdgeType.UNDIRECTED)
            return Collections.unmodifiableCollection(undirected_edges.keySet());
        else
            return null;
    }

    public Pair<V> getEndpoints(E edge)
    {
        Pair<V> endpoints;
        endpoints = directed_edges.get(edge);
        if (endpoints == null)
            return undirected_edges.get(edge);
        else
            return endpoints;
    }

    public EdgeType getEdgeType(E edge)
    {
        if (directed_edges.containsKey(edge))
            return EdgeType.DIRECTED;
        else if (undirected_edges.containsKey(edge))
            return EdgeType.UNDIRECTED;
        else
            return null;
    }

    public V getSource(E directed_edge)
    {
        if (getEdgeType(directed_edge) == EdgeType.DIRECTED)
            return directed_edges.get(directed_edge).getFirst();
        else
            return null;
    }

    public V getDest(E directed_edge)
    {
        if (getEdgeType(directed_edge) == EdgeType.DIRECTED)
            return directed_edges.get(directed_edge).getSecond();
        else
            return null;
    }

    public boolean isSource(V vertex, E edge)
    {
        if (!containsVertex(vertex) || !containsEdge(edge))
            return false;
        
        V source = getSource(edge);
        if (source != null)
            return source.equals(vertex);
        else
            return false;
    }

    public boolean isDest(V vertex, E edge)
    {
        if (!containsVertex(vertex) || !containsEdge(edge))
            return false;
        
        V dest = getDest(edge);
        if (dest != null)
            return dest.equals(vertex);
        else
            return false;
    }

    public Collection<E> getEdges()
    {
        Collection<E> edges = new ArrayList<E>(directed_edges.keySet());
        edges.addAll(undirected_edges.keySet());
        return Collections.unmodifiableCollection(edges);
    }

    public Collection<V> getVertices()
    {
        return Collections.unmodifiableCollection(vertex_maps.keySet());
    }

    public boolean containsVertex(V vertex)
    {
        return vertex_maps.containsKey(vertex);
    }

    public boolean containsEdge(E edge)
    {
        return directed_edges.containsKey(edge) || undirected_edges.containsKey(edge);
    }

    public int getEdgeCount()
    {
        return directed_edges.size() + undirected_edges.size();
    }

    public int getVertexCount()
    {
        return vertex_maps.size();
    }

    public Collection<V> getNeighbors(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        // consider directed edges and undirected edges
        Collection<V> neighbors = new HashSet<V>(vertex_maps.get(vertex)[INCOMING].keySet());
        neighbors.addAll(vertex_maps.get(vertex)[OUTGOING].keySet());
        neighbors.addAll(vertex_maps.get(vertex)[INCIDENT].keySet());
        return Collections.unmodifiableCollection(neighbors);
    }

    public Collection<E> getIncidentEdges(V vertex)
    {
        if (!containsVertex(vertex))
            return null;
        Collection<E> incident = new HashSet<E>(vertex_maps.get(vertex)[INCOMING].values());
        incident.addAll(vertex_maps.get(vertex)[OUTGOING].values());
        incident.addAll(vertex_maps.get(vertex)[INCIDENT].values());
        return Collections.unmodifiableCollection(incident);
    }

    @SuppressWarnings("unchecked")
    public boolean addVertex(V vertex)
    {
        if(vertex == null) {
            throw new IllegalArgumentException("vertex may not be null");
        }
        if (!containsVertex(vertex)) {
            vertex_maps.put(vertex, new HashMap[]{new HashMap<V,E>(), new HashMap<V,E>(), new HashMap<V,E>()});
            return true;
        } else {
            return false;
        }
    }

    public boolean removeVertex(V vertex)
    {
        if (!containsVertex(vertex))
            return false;
        
        // copy to avoid concurrent modification in removeEdge
        Collection<E> incident = new ArrayList<E>(getIncidentEdges(vertex));
        
        for (E edge : incident)
            removeEdge(edge);
        
        vertex_maps.remove(vertex);
        
        return true;
    }

    public boolean removeEdge(E edge)
    {
        if (!containsEdge(edge)) 
            return false;
        
        Pair<V> endpoints = getEndpoints(edge);
        V v1 = endpoints.getFirst();
        V v2 = endpoints.getSecond();
        
        // remove edge from incident vertices' adjacency maps
        if (getEdgeType(edge) == EdgeType.DIRECTED)
        {
            vertex_maps.get(v1)[OUTGOING].remove(v2);
            vertex_maps.get(v2)[INCOMING].remove(v1);
            directed_edges.remove(edge);
        }
        else
        {
            vertex_maps.get(v1)[INCIDENT].remove(v2);
            vertex_maps.get(v2)[INCIDENT].remove(v1);
            undirected_edges.remove(edge);
        }

        return true;
    }
    
    @SuppressWarnings("unchecked")
	public boolean swapEdgeType(E edge)
    {
        if (!containsEdge(edge)) 
            return false;
        
        Pair<V> endpoints = getEndpoints(edge);
        V v1 = endpoints.getFirst();
        V v2 = endpoints.getSecond();
        

        if (getEdgeType(edge) == EdgeType.DIRECTED)
        {
            // remove edge from incident vertices' adjacency maps
            vertex_maps.get(v1)[OUTGOING].remove(v2);
            vertex_maps.get(v2)[INCOMING].remove(v1);
            directed_edges.remove(edge);

            E connection = findEdge(v2, v1);
            vertex_maps.get(v1)[INCOMING].remove(v2);
            vertex_maps.get(v2)[OUTGOING].remove(v1);
            directed_edges.remove(connection);

            // Add edge on undirected map
            vertex_maps.get(v1)[INCIDENT].put(v2, edge);
            vertex_maps.get(v2)[INCIDENT].put(v1, edge);
            undirected_edges.put(edge, endpoints);
        }
        else
        {
            // remove edge from incident vertices' adjacency maps
            vertex_maps.get(v1)[INCIDENT].remove(v2);
            vertex_maps.get(v2)[INCIDENT].remove(v1);
            undirected_edges.remove(edge);
            
            // Add edge on directed map
            vertex_maps.get(v1)[OUTGOING].put(v2, edge);
            vertex_maps.get(v2)[INCOMING].put(v1, edge);
            directed_edges.put(edge, endpoints);
            
            MyLink myLink = (MyLink) edge;
            MyLink myLink2 =  EdgeFactory.getInstance().create();
            
            // Clone the values to the new link
            for (int i = 0; i < MyLink.rates; i++) {
				myLink2.setPloss(i, myLink.getPloss(i));
			}
            
            E edge2 = (E) myLink2;
            vertex_maps.get(v1)[INCOMING].put(v2, edge2);
            vertex_maps.get(v2)[OUTGOING].put(v1, edge2);
            
            Pair<V> new_endpoints = new Pair<V>(endpoints.getSecond(), endpoints.getFirst());
            directed_edges.put(edge2, new_endpoints);
        }

        return true;
    }
    
    public int getEdgeCount(EdgeType edge_type)
    {
        if (edge_type == EdgeType.DIRECTED)
            return directed_edges.size();
        if (edge_type == EdgeType.UNDIRECTED)
            return undirected_edges.size();
        return 0;
    }

	public EdgeType getDefaultEdgeType() 
	{
		return EdgeType.UNDIRECTED;
	}
	
	public String toList() {
		StringBuffer sb = new StringBuffer("Edges:");
		sb.append("\n");
		for (E e : getEdges()) {
			Pair<V> ep = getEndpoints(e);
			sb.append(e + "[" + ep.getFirst() + "," + ep.getSecond()
					+ "] ");
		}
		return sb.toString();
	}

	public ProbMatrix toMatrix(int rate) {
		ProbMatrix p = new ProbMatrix(MyNode.nodeCount); 

		for (E e : getEdges()) {

			MyLink ml = (MyLink)e;
			Pair<V> ep = getEndpoints(e);
			
			p.setValue(Integer.parseInt(ep.getFirst().toString()),
					Integer.parseInt(ep.getSecond().toString()),
					ml.getPloss(rate));
			
			if(getEdgeType(e).equals(EdgeType.UNDIRECTED)) {
				p.setValue(Integer.parseInt(ep.getSecond().toString()),
						Integer.parseInt(ep.getFirst().toString()),
						ml.getPloss(rate));
			}
		}
		return p;
	}
	
	public void setDataFromMatrixList(ProbMatrixList matrixList) {
		
		Vector <MyNode>tmpList = new Vector<MyNode>();
		
		for (int i = 0; i < matrixList.nodes(); i++) {
			MyNode node = VertexFactory.getInstance().create();
			tmpList.add(node);
			addVertex((V)node);
		}
		
		for (int i = 0; i < matrixList.nodes(); i++) {
			for (int j = i+1; j < matrixList.nodes(); j++) {
				Pair <MyLink> p = matrixList.getEdge(i, j);
				
				if (p.getFirst().equals(p.getSecond())) {
					// Es bidireccional
					Pair <V> endpoints = new Pair<V> ((V)tmpList.get(i),(V)tmpList.get(j));
					addEdge((E) p.getFirst(), endpoints, EdgeType.UNDIRECTED);
				} else {
					// Se han de hacer dos unidireccionales
					Pair <V> endpoints = new Pair<V> ((V)tmpList.get(i),(V)tmpList.get(j));
					addEdge((E) p.getFirst(), endpoints, EdgeType.DIRECTED);
					
					endpoints = new Pair<V> ((V)tmpList.get(j),(V)tmpList.get(i));
					addEdge((E) p.getSecond(), endpoints, EdgeType.DIRECTED);
				}
			}
		}
	}
	
	public void clear() {
		vertex_maps.clear();
		directed_edges.clear();
		undirected_edges.clear();
	}
}
