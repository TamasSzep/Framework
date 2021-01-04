// Core/GraphViz.cpp

#include <Core/GraphViz.h>

#include <Core/Platform.h>

#include <fstream>
#include <stdexcept>

using namespace Core;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GraphVizElement::s_EmptyString;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GraphVizGraph::s_DefaultGraphName("Graph1");

GraphVizGraph::GraphVizGraph(bool isDirected, const std::string& name)
	: m_IsDirected(isDirected)
	, m_Name(name)
{
}

bool GraphVizGraph::IsDirected() const
{
	return m_IsDirected;
}

const std::string& GraphVizGraph::GetName() const
{
	return m_Name;
}

unsigned GraphVizGraph::AddNode(const std::string& label)
{
	auto nodeIndex = static_cast<unsigned>(m_Nodes.size());

	char nodeName[16];
	{
#ifdef IS_WINDOWS
		sprintf_s(nodeName, 16, "node%d", nodeIndex);
#else
		sprintf(nodeName, "node%d", nodeIndex);
#endif
	}

	m_Nodes.emplace_back();
	auto& node = m_Nodes.back();
	node.Label = label;
	node.Name = nodeName;
	return nodeIndex;
}

unsigned GraphVizGraph::AddEdge(unsigned node1Index, unsigned node2Index, const std::string& label)
{
	auto edgeIndex = static_cast<unsigned>(m_Edges.size());
	m_Edges.emplace_back();
	auto& edge = m_Edges.back();
	edge.m_Node1Index = node1Index;
	edge.m_Node2Index = node2Index;
	edge.Label = label;
	return edgeIndex;
}

void GraphVizGraph::Save(const std::string& fileName)
{
	// Creating output stream.
	std::ofstream os(fileName);
	if (!os)
	{
		throw std::runtime_error(("Cannot open file for writing: " + fileName).c_str());
	}

	// Starting graph.
	if (m_IsDirected)
	{
		os << "digraph";
	}
	else
	{
		os << "graph";
	}
	os << " {\n";

	// Writing nodes.
	for (size_t i = 0; i < m_Nodes.size(); i++)
	{
		auto& node = m_Nodes[i];

		os << "\"" << node.Name << "\" [label=\"" << node.Label << "\"]\n";
	}

	// Writing edges.
	for (size_t i = 0; i < m_Edges.size(); i++)
	{
		auto& edge = m_Edges[i];
		auto& node1 = m_Nodes[edge.m_Node1Index];
		auto& node2 = m_Nodes[edge.m_Node2Index];

		os << "\"" << node1.Name << "\"";
		
		if (m_IsDirected)
		{
			os << "->";
		}
		else
		{
			os << "--";
		}

		os << "\"" << node2.Name << "\"[label=\"" << edge.Label << "\"]\n";
	}

	// Closing graph.
	os << "}\n";

	// Closing output stream.
	os.close();
}