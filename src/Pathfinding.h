#pragma once

#include "PopSS.h"

namespace IntelOrca { namespace PopSS {

class PathNode {
public:
	int x, z;
	int g, f;
	PathNode *parent;

	PathNode();
	PathNode(int x, int z);

	class Comparator {
	public:
		Comparator(const bool &rev = false) { this->reverse = rev; }

		bool operator() (const PathNode &lhs, const PathNode &rhs) const {
			if (this->reverse) return lhs.f > rhs.f;
			else return lhs.f < rhs.f;
		}

	private:
		bool reverse;
	};
};

class Path {
public:
	int length;
	struct { int x, z; } *positions;
};

class Pathfinding {
public:
	static int GetDistance(int x0, int z0, int x1, int z1);
	static Path *GetPath(int startX, int startZ, int targetX, int targetZ);
	static std::vector<PathNode> IdentifySuccessors(const PathNode &node, int targetX, int targetZ);
	static bool Jump(int x, int z, int dx, int dz, int targetX, int targetZ, PathNode *outNode);
};

} }