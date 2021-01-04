// Core/GraphViz.h

#ifndef _CORE_GRAPHVIZ_H_
#define _CORE_GRAPHVIZ_H_

#include <vector>
#include <string>

namespace Core
{
	struct GraphVizElement
	{
		std::string Label;

		static std::string s_EmptyString;
	};

	struct GraphVizNode : public GraphVizElement
	{
		std::string Name;
	};

	struct GraphVizEdge : public GraphVizElement
	{
		unsigned m_Node1Index;
		unsigned m_Node2Index;
	};

	class GraphVizGraph
	{
		std::vector<GraphVizNode> m_Nodes;
		std::vector<GraphVizEdge> m_Edges;

		bool m_IsDirected;
		std::string m_Name;

		static std::string s_DefaultGraphName;

	public:

		GraphVizGraph(bool isDirected, const std::string& name = s_DefaultGraphName);

		bool IsDirected() const;
		const std::string& GetName() const;

		unsigned AddNode(const std::string& label = GraphVizElement::s_EmptyString);
		unsigned AddEdge(unsigned node1Index, unsigned node2Index,
			const std::string& label = GraphVizElement::s_EmptyString);

		void Save(const std::string& fileName);
	};
}

#endif
