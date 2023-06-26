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

	void AddWire(WireElbow elbow, size_t startNode, size_t endNode)
	{
		{
			Wire createdWire =
			{
				.elbow = elbow,
				.startNode = startNode,
				.endNode = endNode,
			};
			wires[numWires++] = createdWire;
		}
	}

	// Removes the nodes in nodeIndicesToRemove and numNodesToRemove.
	void RemoveSelectedNodes()
	{
		for (size_t offset = 1; offset <= numNodesSelected; ++offset)
		{
			for (size_t i = nodesSelected[offset - 1] + 1; i < nodesSelected[offset]; ++i)
			{
				nodes[i - offset] = nodes[i];
			}

			for (size_t i = 0; i < numWires; ++i)
			{
				if (nodesSelected[offset - 1] < wires[i].startNode && wires[i].startNode < nodesSelected[offset])
				{

				}
			}
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
