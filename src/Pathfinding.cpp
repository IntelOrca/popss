#include "Pathfinding.h"
#include "World.h"

using namespace IntelOrca::PopSS;

enum {
	TILE_FLAG_OPEN = (1 << 0),
	TILE_FLAG_CLOSED = (1 << 1)
};

PathNode::PathNode() { }
PathNode::PathNode(int x, int z)
{
	this->x = x;
	this->z = x;
	this->g = 0;
	this->f = 0;
}


PathFinder::PathFinder()
{
	this->tileFlags = new uint8[gWorld->sizeSquared];
	this->nodeMap = new PathNode*[gWorld->sizeSquared];
}

PathFinder::~PathFinder()
{
	SafeDelete(this->tileFlags);
	SafeDelete(this->nodeMap);
}

Path PathFinder::GetPath(int startX, int startZ, int goalX, int goalZ)
{
	// Initialise state
	this->openset = PathNodePriorityQueue();
	memset(this->tileFlags, 0, gWorld->sizeSquared * sizeof(uint8));
	memset(this->nodeMap, 0, gWorld->sizeSquared * sizeof(PathNode*));
	this->nodePool.clear();

	// Add start node
	PathNode *startNode = this->GetNewNode();
	startNode->x = startX;
	startNode->z = startZ;
	startNode->g = 0;
	startNode->f = startNode->g + this->EstimateHeuristicCost(startX, startZ, goalX, goalZ);
	startNode->parent = NULL;
	this->nodeMap[startX + startZ * gWorld->size] = startNode;
	this->openset.push(startNode);

	while (!openset.empty()) {
		const PathNode *current = openset.top();

		// Ignore out of date nodes
		if (current->g == -1) {
			openset.pop();
			continue;
		}

		if (current->x == goalX && current->z == goalZ)
			GetPathToNode(current);

		openset.pop();
		this->tileFlags[current->x + current->z * gWorld->size] &= ~TILE_FLAG_OPEN;
		this->tileFlags[current->x + current->z * gWorld->size] |= TILE_FLAG_CLOSED;

		for (int dz = -1; dz <= 1; dz++) {
			for (int dx = -1; dx <= 1; dx++) {
				int neighbourX = current->x + dx;
				int neighbourZ = current->z + dz;

				if (gWorld->GetTile(neighbourX, neighbourZ) == 0)
					continue;

				if (this->IsPositionInClosedSet(neighbourX, neighbourZ))
					continue;

				bool isopen = this->IsPositionInOpenSet(neighbourX, neighbourZ);
				int tentativeG = current->g + this->GetDistance(current->x, current->z, neighbourX, neighbourZ);
				PathNode *existing = this->nodeMap[neighbourX + neighbourZ * gWorld->size];

				if (!isopen || tentativeG < existing->g) {
					// Set existing node to be out of date
					if (existing != NULL)
						existing->g = -1;
					
					PathNode *node = this->GetNewNode();
					node->x = neighbourX;
					node->z = neighbourZ;
					node->g = tentativeG;
					node->f = tentativeG + this->EstimateHeuristicCost(neighbourX, neighbourZ, goalX, goalZ);
					node->parent = current;
					this->openset.push(node);
				}
			}
		}
	}

	return Path();
}

PathNode *PathFinder::GetNewNode()
{
	this->nodePool.push_back(PathNode());
	return &this->nodePool.back();
}

bool PathFinder::IsPositionInOpenSet(int x, int z)
{
	return this->tileFlags[x + z * gWorld->size] & TILE_FLAG_OPEN;
}

bool PathFinder::IsPositionInClosedSet(int x, int z)
{
	return this->tileFlags[x + z * gWorld->size] & TILE_FLAG_CLOSED;
}

int PathFinder::EstimateHeuristicCost(int startX, int startZ, int goalX, int goalZ)
{
	return abs(goalX - startX) + abs(goalZ - startZ);
}

int PathFinder::GetDistance(int x0, int z0, int x1, int z1)
{
	return 1;
}

Path PathFinder::GetPathToNode(const PathNode *node) const
{
	Path path;
	std::vector<PathPosition> positions;
	do {
		positions.push_back(node->position);
		node = node->parent;
	} while (node != NULL);

	// Add them to path in reverse
	path.length = positions.size();
	path.positions = new PathPosition[path.length];
	for (int i = 0; i < path.length; i++)
		path.positions[i] = positions[path.length - i - 1];

	return path;
}







int Pathfinding::GetDistance(int x0, int z0, int x1, int z1)
{
	return abs(x1 - x0) + abs(z1 - z0);
}

Path *Pathfinding::GetPath(int startX, int startZ, int targetX, int targetZ)
{
	// unsigned char *openclosed = new unsigned char[gWorld->size * gWorld->size];
	// memset(openclosed, 0, gWorld->size * gWorld->size * sizeof(unsigned char));
	// 
	// std::priority_queue<PathNode, std::vector<PathNode>, PathNode::Comparator> openset;
	// std::vector<PathNode> nodePool;
	// 
	// PathNode startNode = PathNode(startX, startZ);
	// startNode.g = 0;
	// startNode.f = heuristic(startNode, targetX, targetZ);
	// openset.push(startNode);
	// 
	// while (!openset.empty()) {
	// 	PathNode current = openset.top();
	// 	openset.pop();
	// 	openclosed[current.x + current.z * gWorld->size] &= ~1;
	// 
	// 	if (current.x == targetX && current.z == targetZ) {
	// 		delete[] openclosed;
	// 
	// 		Path *path = new Path();
	// 		
	// 		return path;
	// 	}
	// 
	// 	// Close position
	// 	openclosed[current.x + current.z * gWorld->size] |= 2;
	// 
	// 	for (int z = -1; z <= 1; z++) {
	// 		for (int x = -1; x <= 1; x++) {
	// 			nodePool.push_back({});
	// 			PathNode *neighbour = &nodePool[nodePool.size() - 1];
	// 			neighbour->x = current.x + x;
	// 			neighbour->z = current.z + z;
	// 			neighbour->f = 0;
	// 			neighbour->g = 0;
	// 			neighbour->parent = NULL;
	// 
	// 			if (openclosed[neighbour->x + neighbour->z * gWorld->size] & 2)
	// 				continue;
	// 
	// 			int tentativeGScore = current.g + GetDistance(current.x, current.z, neighbour->x, neighbour->z);
	// 
	// 			bool neighbourOpen = openclosed[neighbour->x + neighbour->z * gWorld->size] & 1;
	// 			if (!neighbourOpen || tentativeGScore < neighbour->g) {
	// 				neighbour->parent = &current;
	// 			}
	// 		}
	// 	}
	// 
	// 	// std::vector<PathNode> successors = IdentifySuccessors(current, targetX, targetZ);
	// 	// 
	// 	// for (const PathNode &node : successors) {
	// 	// 	if (openclosed[node.x + node.z * gWorld->size] & 2)
	// 	// 		continue;
	// 	// 
	// 	// 	int g = current.g + GetDistance(current.x, current.z, node.x, node.z);
	// 	// 	bool openSetNeighbour = openclosed[node.x + node.z * gWorld->size] & 1;
	// 	// 
	// 	// 	if (!openSetNeighbour || g < node.g) {
	// 	// 		node.parent = current;
	// 	// 		node.g = g;
	// 	// 		node.f = node.g + heuristic(node, targetX, targetZ);
	// 	// 
	// 	// 		if (!openSetNeighbour)
	// 	// 			openset.push(node);
	// 	// 	}
	// 	// }
	// }
	// 
	// delete[] openclosed;
	return NULL;
}

std::vector<PathNode> Pathfinding::IdentifySuccessors(const PathNode &node, int targetX, int targetZ)
{
	std::vector<PathNode> successors;
	std::vector<PathNode> neighbours;

	neighbours.push_back(node);
	for (int z = -1; z <= 1; z++) {
		for (int x = -1; x <= 1; x++) {
			PathNode jumpNode;
			if (Jump(node.x, node.z, x, z, targetX, targetZ, &jumpNode))
				successors.push_back(jumpNode);
		}
	}

	return successors;
}

bool Pathfinding::Jump(int x, int z, int dx, int dz, int targetX, int targetZ, PathNode *outNode)
{
	int nextX = x + dx;
	int nextZ = z + dz;
	int nextHeight = gWorld->GetTile(nextX, nextZ)->height;

	// If it's blocked we can't jump here
	if (nextHeight == 0)
		return false;

	// If the node is the goal return it
	if (nextX == targetX && nextZ == targetZ) {
		if (outNode != NULL) {
			outNode->x = nextX;
			outNode->z = nextZ;
		}
		return true;
	}

	if (dx != 0 && dz != 0) {
		// Diagonal case
		if (
			(gWorld->GetTile(nextX - dx, nextZ + dz)->height != 0 && gWorld->GetTile(nextX - dx, nextZ)->height == 0) ||
			(gWorld->GetTile(nextX + dx, nextZ - dz)->height != 0 && gWorld->GetTile(nextX, nextZ - dz)->height == 0)
		) {
			if (outNode != NULL) {
				outNode->x = nextX;
				outNode->z = nextZ;
			}
			return true;
		}

		// Check in horizontal and vertical directions for forced neighbors
		// This is a special case for diagonal direction
		if (
			Jump(nextX, nextZ, dx, 0, targetX, targetZ, NULL) ||
			Jump(nextX, nextZ, 0, dz, targetX, targetZ, NULL)
		) {
			outNode->x = nextX;
			outNode->z = nextZ;
			return true;
		}
	} else {
		if (dx != 0) {
			// Horizontal case
			if (
				(gWorld->GetTile(nextX + dx, nextZ + 1)->height != 0 && gWorld->GetTile(nextX, nextZ + 1)->height == 0) ||
				(gWorld->GetTile(nextX + dx, nextZ - 1)->height != 0 && gWorld->GetTile(nextX, nextZ - 1)->height == 0)
			) {
				if (outNode != NULL) {
					outNode->x = nextX;
					outNode->z = nextZ;
				}
				return true;
			}
		} else {
			// Vertical case
			if (
				(gWorld->GetTile(nextX + 1, nextZ + dz)->height != 0 && gWorld->GetTile(nextX + 1, nextZ)->height == 0) ||
				(gWorld->GetTile(nextX - 1, nextZ + dz)->height != 0 && gWorld->GetTile(nextX - 1, nextZ)->height == 0)
			) {
				if (outNode != NULL) {
					outNode->x = nextX;
					outNode->z = nextZ;
				}
				return true;
			}
		}
	}

	// If forced neighbor was not found try next jump point
	return Jump(nextX, nextZ, dx, dz, targetX, targetZ, outNode);
}