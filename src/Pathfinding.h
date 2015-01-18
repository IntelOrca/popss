#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

struct PathPosition {
	int x, z;
};

class PathNode {
public:
	union {
		PathPosition position;
		struct { int x, z; };
	};
	int g, f;
	const PathNode *parent;

	PathNode();
	PathNode(int x, int z);

	class Comparator {
	public:
		Comparator(const bool &rev = false) { this->reverse = rev; }

		bool operator() (const PathNode *lhs, const PathNode *rhs) const {
			if (this->reverse) return lhs->f < rhs->f;
			else return lhs->f > rhs->f;
		}

	private:
		bool reverse;
	};
};

typedef std::priority_queue<PathNode*, std::vector<PathNode*>, PathNode::Comparator> PathNodePriorityQueue;

class Path {
public:
	int length;
	PathPosition *positions;

	Path() { length = 0; positions = NULL; }
};

class PathFinder {
public:
	PathFinder();
	~PathFinder();

	Path GetPath(int startX, int startZ, int goalX, int goalZ);

	static void RunPathfinderLoop();

private:
	uint8 *tileFlags;
	PathNode **nodeMap;
	PathNodePriorityQueue openset;
	std::vector<PathNode> nodePool;

	PathNode *GetNewNode();

	bool IsPositionInOpenSet(int x, int z);
	bool IsPositionInClosedSet(int x, int z);

	int EstimateHeuristicCost(int startX, int startZ, int goalX, int goalZ);
	int GetDistance(int x0, int z0, int x1, int z1);

	Path GetPathToNode(const PathNode *node) const;
};

class Pathfinding {
public:
	static int GetDistance(int x0, int z0, int x1, int z1);
	static Path *GetPath(int startX, int startZ, int targetX, int targetZ);
	static std::vector<PathNode> IdentifySuccessors(const PathNode &node, int targetX, int targetZ);
	static bool Jump(int x, int z, int dx, int dz, int targetX, int targetZ, PathNode *outNode);
};

} }