#include <algorithm>
#include "graph_algorithms.hpp"

namespace graph
{
	extern int gridDisplaySize_WithLine; // Defined in graph.cpp

	size_t nodesSelected[MAX_NODES] = {};
	size_t numNodesSelected = 0;

	size_t wiresSelected[MAX_WIRES] = {};
	size_t numWiresSelected = 0;

	void AddNode(NodeType type, int screenx, int screeny)
	{
		int x = screenx / gridDisplaySize_WithLine;
		int y = screeny / gridDisplaySize_WithLine;
		Node createdNode =
		{
			.type = type,
			.x = x,
			.y = y,
		};
		nodes[numNodes++] = createdNode;
	}

	void AddWire(WireElbow elbow, Node* startNode, Node* endNode)
	{
		wires[numWires++] = Wire
		{
			.elbow = elbow,
			.startNode = startNode,
			.endNode = endNode,
		};
	}

	// Removes the nodes in nodeIndicesToRemove and numNodesToRemove.
	void RemoveSelectedNodes()
	{
		{
			size_t index = 0;
			auto pred = [&index](const Node& node)
			{
				if (index == numNodesSelected)
				{
					return false;
				}

				size_t nodeIndex = &node - nodes;
				if (nodeIndex == nodesSelected[index])
				{
					++index;
					return true;
				}
				return false;
			};
			std::stable_partition(nodes, nodes + numNodes, pred);
			size_t numRemoved = index + 1;
			numNodes -= numRemoved;
		}
	}

	// Deposits results in nodesSelected and numNodesSelected.
	void SelectNodesInRanges(panel::Bounds screenRanges[], size_t numRanges)
	{
		numNodesSelected = 0;

		// Convert from screenspace to gridspace
		for (size_t i = 0; i < numRanges; ++i)
		{
			panel::Bounds& range = screenRanges[i];

			range.xmin = range.xmin / gridDisplaySize_WithLine;
			range.ymin = range.ymin / gridDisplaySize_WithLine;
			range.xmax = range.xmax / gridDisplaySize_WithLine;
			range.ymax = range.ymax / gridDisplaySize_WithLine;
		}

		for (size_t i = 0; i < numNodes; ++i)
		{
			const Node& node = nodes[i];

			for (size_t i = 0; i < numRanges; ++i)
			{
				const panel::Bounds& range = screenRanges[i];

				bool isInRange =
					range.xmin <= node.x && node.x <= range.xmax &&
					range.ymin <= node.y && node.y <= range.ymax;

				if (isInRange)
				{
					nodesSelected[numNodesSelected++] = i;
					break; // Only include once
				}
			}
		}
	}

	void RemoveNode(int screenx, int screeny)
	{
		panel::Bounds ranges[1] =
		{
			{
				.xmin = screenx,
				.ymin = screeny,
				.xmax = screenx,
				.ymax = screeny,
			}
		};
		SelectNodesInRanges(ranges, 1);
		RemoveSelectedNodes();
	}
}
