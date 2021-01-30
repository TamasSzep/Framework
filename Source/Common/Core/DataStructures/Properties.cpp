// Core/Properties.cpp

#include <Core/DataStructures/Properties.h>

#include <Core/System/SimpleIO.h>
#include <Core/Constants.h>
#include <Core/String.hpp>

#include <External/rapidxml/rapidxml.hpp>
#include <External/rapidxml/rapidxml_print.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>

using namespace Core;

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned Properties::AddString(const char* str, unsigned size)
{
	return m_Strings.Add(str, size);
}

unsigned Properties::AddString(const char* str)
{
	return m_Strings.Add(str);
}

void Properties::ReleaseString(unsigned index)
{
	m_Strings.Remove(index);
}

void Properties::ModifyString(unsigned index, const char* str)
{
	m_Strings[index] = str;
}

const char* Properties::GetString(unsigned index) const
{
	return m_Strings[index].c_str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned Properties::CreateNode(const char* name, unsigned parent)
{
	Node node;
	node.Name = AddString(name);
	node.Parent = parent;
	return m_Nodes.Add(std::move(node));
}

unsigned Properties::CreateNode(const char* name, unsigned nameSize, unsigned parent)
{
	Node node;
	node.Name = AddString(name, nameSize);
	node.Parent = parent;
	return m_Nodes.Add(std::move(node));
}

unsigned Properties::CreateProperty(const char* name, const char* value)
{
	Property property;
	property.Name = AddString(name);
	property.Value = AddString(value);
	return m_Properties.Add(std::move(property));
}

inline bool Properties::IsMatchingNodeName(unsigned nameIndex, const char* path, const char* dotSearchRes) const
{
	return (strncmp(GetString(nameIndex), path, static_cast<unsigned>(dotSearchRes - path)) == 0);
}

void Properties::Traverse(const char* path, IndexVectorU& propertyIndices,
	unsigned& longestMatchSize, unsigned& longestMatchIndex) const
{
	struct TraverseData
	{
		unsigned NodeIndex;
		unsigned PathStart;
		unsigned MatchSize;
	};

	auto pRootDot = strchr(path, '.');
	assert(pRootDot != nullptr); // No direct properties are supported.

	auto roots = const_cast<decltype(m_Roots)&>(m_Roots);
	auto it = roots.GetBeginIterator();
	auto end = roots.GetEndIterator();
	std::queue<TraverseData> indices;
	for (; it != end; ++it)
	{
		if (IsMatchingNodeName(m_Nodes[*it].Name, path, pRootDot))
		{
			indices.push({ *it, static_cast<unsigned>(pRootDot - path) + 1, 1 });
		}
	}

	longestMatchSize = 0;
	longestMatchIndex = c_InvalidIndexU;

	while (indices.size() > 0)
	{
		auto& traverseData = indices.front();
		unsigned nodeIndex = traverseData.NodeIndex;
		auto currentPath = path + traverseData.PathStart;
		unsigned matchSize = traverseData.MatchSize;
		unsigned nextMatchSize = matchSize + 1;
		indices.pop();

		if (matchSize > longestMatchSize)
		{
			longestMatchSize = matchSize;
			longestMatchIndex = nodeIndex;
		}

		auto& nodeData = m_Nodes[nodeIndex];

		auto pDot = strchr(currentPath, '.');
		if (pDot == nullptr)
		{
			auto& properties = nodeData.Properties;
			for (unsigned i = 0; i < properties.GetSize(); i++)
			{
				unsigned propertyIndex = properties[i];
				auto& property = m_Properties[propertyIndex];

				if (strcmp(GetString(property.Name), currentPath) == 0)
				{
					propertyIndices.PushBack(propertyIndex);
				}
			}
		}
		else
		{
			auto& children = nodeData.Children;
			for (unsigned i = 0; i < children.GetSize(); i++)
			{
				unsigned childNodeIndex = children[i];
				auto& childNode = m_Nodes[childNodeIndex];
				if (IsMatchingNodeName(childNode.Name, currentPath, pDot))
				{
					indices.push({ childNodeIndex, static_cast<unsigned>(pDot - path) + 1, nextMatchSize });
				}
			}
		}
	}
}

inline const char* SkipNChar(const char* str, char ch, unsigned n)
{
	const char* result = str;
	for (; n > 0; --n, result = strchr(result, ch) + 1);
	return result;
}

void Properties::AddPropertyOnNonExistingPath(const char* path, unsigned matchSize, unsigned nodeIndex,
	const char* value)
{
	if (nodeIndex == c_InvalidIndexU)
	{
		assert(matchSize == 0);
		auto pRootDot = strchr(path, '.');
		assert(pRootDot != nullptr); // No direct property is supported.
		nodeIndex = CreateNode(path, static_cast<unsigned>(pRootDot - path), c_InvalidIndexU);
		path = pRootDot + 1;
		m_Roots.Add(nodeIndex);
	}

	auto currentPath = SkipNChar(path, '.', matchSize);
	while (true)
	{
		auto pDot = strchr(currentPath, '.');
		if (pDot == nullptr)
		{
			m_Nodes[nodeIndex].Properties.Add(CreateProperty(currentPath, value));
			break;
		}
		else
		{
			unsigned childNodeIndex = CreateNode(currentPath, static_cast<unsigned>(pDot - currentPath), nodeIndex);
			m_Nodes[nodeIndex].Children.Add(childNodeIndex);
			nodeIndex = childNodeIndex;
			currentPath = pDot + 1;
		}
	}
}

const char* Properties::GetPropertyValueStr(const char* path) const
{
	IndexVectorU propertyIndices;
	unsigned longestMatchSize, longestMatchIndex;
	Traverse(path, propertyIndices, longestMatchSize, longestMatchIndex);

	assert(propertyIndices.GetSize() <= 1);

	if (propertyIndices.GetSize() == 0) return nullptr;
	return GetString(m_Properties[propertyIndices[0]].Value);
}

bool Properties::HasProperty(const char* path) const
{
	IndexVectorU propertyIndices;
	unsigned longestMatchSize, longestMatchIndex;
	Traverse(path, propertyIndices, longestMatchSize, longestMatchIndex);

	return (propertyIndices.GetSize() > 0);
}

unsigned Properties::GetPropertyValuesStr(const char* path, const char** pPath, unsigned maxResultCount)
{
	IndexVectorU propertyIndices;
	unsigned longestMatchSize, longestMatchIndex;
	Traverse(path, propertyIndices, longestMatchSize, longestMatchIndex);

	unsigned countResults = propertyIndices.GetSize();
	unsigned countStoredResults = std::min(countResults, maxResultCount);
	for (unsigned i = 0; i < countStoredResults; i++)
	{
		pPath[i] = GetString(m_Properties[propertyIndices[i]].Value);
	}

	return countResults;
}

void Properties::AddProperty(const char* path, const char* value)
{
	IndexVectorU propertyIndices;
	unsigned longestMatchSize, longestMatchIndex;
	Traverse(path, propertyIndices, longestMatchSize, longestMatchIndex);

	// Adding a new property, even if one or more old properties with the same path exists.
	AddPropertyOnNonExistingPath(path, longestMatchSize, longestMatchIndex, value);
}

void Properties::SetProperty(const char* path, const char* value)
{
	IndexVectorU propertyIndices;
	unsigned longestMatchSize, longestMatchIndex;
	Traverse(path, propertyIndices, longestMatchSize, longestMatchIndex);

	if (propertyIndices.GetSize() > 0)
	{
		assert(propertyIndices.GetSize() == 1);
		ModifyString(m_Properties[propertyIndices[0]].Value, value);
	}
	else
	{
		AddPropertyOnNonExistingPath(path, longestMatchSize, longestMatchIndex, value);
	}
}

void Properties::RaiseExceptionOnPropertyNotFound(const char* path) const
{
	throw std::runtime_error((std::string("Property was not found: ") + path).c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void Properties::LoadFromXml(const char* path)
{
	std::string text;
	ReadAllText(path, text);
	LoadFromXmlString(text.c_str());
}

inline std::string GetNodeName(rapidxml::xml_node<>* node)
{
	return std::string(node->name(), node->name_size());
}

inline std::string GetAttributeValue(rapidxml::xml_attribute<>* attribute)
{
	return std::string(attribute->value(), attribute->value_size());
}

void Properties::LoadFromXmlString(const char* str)
{
	char* strCopy;
	Copy(str, &strCopy);
	auto xmlDocument = std::make_unique<rapidxml::xml_document<>>();
	xmlDocument->parse<0>(strCopy);

	struct XmlLoadingData
	{
		rapidxml::xml_node<>* XmlNode;
		unsigned NodeIndex;
	};

	std::queue<XmlLoadingData> queue;
	for (auto rootNode = xmlDocument->first_node(); rootNode != nullptr; rootNode = rootNode->next_sibling())
	{
		unsigned rootNodeIndex = CreateNode(rootNode->name(), c_InvalidIndexU);
		m_Roots.Add(rootNodeIndex);
		queue.push({ rootNode, rootNodeIndex });
	}

	while(!queue.empty())
	{
		const auto& nodeData = queue.front();
		unsigned nodeIndex = nodeData.NodeIndex;
		auto xmlNode = nodeData.XmlNode;
		queue.pop();

		for (auto childXmlNode = xmlNode->first_node(); childXmlNode != nullptr;
			childXmlNode = childXmlNode->next_sibling())
		{
			auto cName = GetNodeName(childXmlNode);
			if (cName == "Property")
			{
				auto nameAttribute = childXmlNode->first_attribute("name");
				auto valueAttribute = childXmlNode->first_attribute("value");
				if (nameAttribute == nullptr || valueAttribute == nullptr) throw std::runtime_error("Invalid property.");
				auto name = GetAttributeValue(nameAttribute);
				auto value = GetAttributeValue(valueAttribute);
				m_Nodes[nodeIndex].Properties.Add(CreateProperty(name.c_str(), value.c_str()));
			}
			else
			{
				unsigned childNodeIndex = CreateNode(cName.c_str(), nodeIndex);
				m_Nodes[nodeIndex].Children.Add(childNodeIndex);
				queue.push({ childXmlNode, childNodeIndex });
			}
		}
	}
}

void Properties::SaveToXml(const char* path) const
{
	auto text = ToXmlString();
	WriteAllText(path, text);
}

std::string Properties::ToXmlString() const
{
	auto xmlDocument = std::make_unique<rapidxml::xml_document<>>();

	struct XmlSavingData
	{
		rapidxml::xml_node<>* XmlNode;
		uint32_t NodeIndex;
	};

	std::queue<XmlSavingData> queue;
	for (uint32_t i = 0; i < m_Roots.GetSize(); i++)
	{
		uint32_t nodeIndex = m_Roots[i];
		const auto node = xmlDocument->allocate_node(rapidxml::node_element,
			m_Strings[m_Nodes[nodeIndex].Name].c_str());
		xmlDocument->append_node(node);
		queue.push({ node , nodeIndex });
	}

	while (!queue.empty())
	{
		const auto& nodeData = queue.front();
		unsigned nodeIndex = nodeData.NodeIndex;
		auto xmlNode = nodeData.XmlNode;
		queue.pop();

		const auto& node = m_Nodes[nodeIndex];

		uint32_t countProperties = node.Properties.GetSize();
		for (uint32_t i = 0; i < countProperties; i++)
		{
			const auto& property = m_Properties[node.Properties[i]];

			const auto propertyNode = xmlDocument->allocate_node(rapidxml::node_element, "Property");
			const auto propertyNameAttribute = xmlDocument->allocate_attribute("name", m_Strings[property.Name].c_str());
			const auto propertyValueAttribute = xmlDocument->allocate_attribute("value", m_Strings[property.Value].c_str());
			propertyNode->append_attribute(propertyNameAttribute);
			propertyNode->append_attribute(propertyValueAttribute);

			xmlNode->append_node(propertyNode);
		}

		uint32_t countChildren = node.Children.GetSize();
		for (uint32_t i = 0; i < countChildren; i++)
		{
			uint32_t childNodeIndex = node.Children[i];
			const auto childNode = xmlDocument->allocate_node(rapidxml::node_element,
				m_Strings[m_Nodes[childNodeIndex].Name].c_str());
			xmlNode->append_node(childNode);
			queue.push({ childNode , childNodeIndex });
		}
	}

	std::stringstream ss;
	for (auto rootNode = xmlDocument->first_node(); rootNode != nullptr; rootNode = rootNode->next_sibling())
	{
		ss << *rootNode;
	}
	return ss.str();
}
