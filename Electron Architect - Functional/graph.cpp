#include <fstream>
#include <string>
#include "console.hpp"
#include "properties.hpp"
#include "tools.hpp"
#include "graph.hpp"

using panel::Panel;
using panel::PanelID;
using panel::Bounds;
using panel::Rect;
using tools::toolsPanel;
using properties::propertiesPanel;
using console::consolePanel;

namespace graph
{
	Panel graphPanel = {
		.title = "Graph",
		.id = PanelID::Graph,
		.bounds = {
			&toolsPanel.bounds.xmax,
			0,
			&propertiesPanel.bounds.xmin,
			&consolePanel.bounds.ymin,
		},
		.draggable = (panel::DraggableEdges)((int)panel::DraggableEdges::EdgeB | (int)panel::DraggableEdges::EdgeR)
	};

	size_t numNodes = 0;
	Node nodes[MAX_NODES] = {};

	size_t numWires = 0;
	Wire wires[MAX_WIRES] = {};

	void Save(const char* filename)
	{
		std::ofstream file(filename);

		int majorVersion = 2;
		int minorVersion = 0;
		int patchVersion = 0;

		file << "v " << majorVersion << ' ' << minorVersion << ' ' << patchVersion << std::endl;

		file << "n " << numNodes;
		for (size_t i = 0; i < numNodes; ++i)
		{
			const Node& node = nodes[i];
			file << '\n' << (char)node.type << ' ' << node.x << ' ' << node.y << " ```" << node.name << "```";
		}
		file << std::endl;

		file << "w " << numWires;
		for (size_t i = 0; i < numWires; ++i)
		{
			const Wire& wire = wires[i];
			file << '\n' << (int)wire.elbow << ' ' << wire.startNode << ' ' << wire.endNode;
		}
		file << std::endl;

		file.close();
	}

	void Load(const char* filename)
	{
		std::ifstream file(filename);

		char regionCode = '\0';

		file >> regionCode;
		if (regionCode != 'v')
		{
			console::Error("graph: File is malformed or incompatible: Expected VERSION (v) region. Cancelling.");
			return;
		}
		int majorVersion, minorVersion, patchVersion;
		file >> majorVersion >> minorVersion >> patchVersion;

		file >> regionCode;
		if (regionCode != 'n')
		{
			console::Error("graph: File is malformed or incompatible: Expected NODE (n) region. Cancelling.");
			return;
		}
		file >> numNodes;

		for (size_t i = 0; i < numNodes; ++i)
		{
			Node& node = nodes[i];
			char _type;
			file >> _type;
			node.type = (NodeType)_type;
			file >> node.x >> node.y;

			// I have concerns about this.
			if (file.peek() != '\n' && !file.eof())
			{
				int ch;
				constexpr size_t BUFF_MAX_SIZE = 512;
				char buff[BUFF_MAX_SIZE] = {};
				size_t strBuffSize = 0;
				while ((ch = file.get()) != '\n')
				{
					buff[strBuffSize++] = ch;
					if (strBuffSize == BUFF_MAX_SIZE - 1)
					{
						console::Warn("Node name exceeds maximum length of 511 characters. Some information will be lost.");
					}
				}
				buff[strBuffSize] = '\0';
				node.name.reserve(strBuffSize);
				node.name = buff;
			}
		}

		file >> regionCode;
		if (regionCode != 'w')
		{
			console::Error("graph: File is malformed or incompatible: Expected WIRE (w) region. Cancelling.");
			return;
		}
		file >> numWires;

		for (size_t i = 0; i < numWires; ++i)
		{
			Wire& wire = wires[i];
			int _elbow;
			file >> _elbow;
			wire.elbow = (WireElbow)_elbow;
			size_t startNodeIndex, endNodeIndex;
			file >> startNodeIndex >> endNodeIndex;
			wire.startNode = &nodes[startNodeIndex];
			wire.endNode = &nodes[endNodeIndex];
		}

		file.close();
	}

	int gridMagnitude = 0;

	int gridOffsetX = 0;
	int gridOffsetY = 0;

	int gridDisplaySize;
	int gridDisplaySize_WithLine;

	void UpdateGridDisplaySize()
	{
		if (gridMagnitude > 0)
		{
			gridDisplaySize = gridSize / (gridMagnitude + 1);
		}
		else if (gridMagnitude < 0)
		{
			gridDisplaySize = gridSize * (-gridMagnitude + 1);
		}
		else // gridMagnitude == 0
		{
			gridDisplaySize = gridSize;
		}

		gridDisplaySize_WithLine = gridDisplaySize + gridlineWidth;
	}

	int ScreenXToGridX(int x)
	{
		return 0;
	}

	int ScreenYToGridY(int y)
	{
		return 0;
	}

	void Zoom(int amount)
	{
		gridMagnitude -= amount;
		if (gridMagnitude > gridSize)
		{
			gridMagnitude = gridSize;
		}
		if (gridMagnitude < -gridSize)
		{
			gridMagnitude = -gridSize;
		}
	}
}
