#include "Grid.h"
#include <iostream>
#include <fstream>
#include <cmath>

////////////////////////////////////////////////////////////////
// create a node
GridNode::GridNode(int nID, int row, int column, bool isC)
{
	this->clear = isC;

	this->rCoord = row;
	this->cCoord = column;

	this->entity = NULL;

	if (isC)
		this->contains = '.';
	else
		this->contains = 'B';
}

// default constructor
GridNode::GridNode()
{
	nodeID = -999;			// mark these as currently invalid
	this->clear = true;
	this->contains = '.';
} 

////////////////////////////////////////////////////////////////
// destroy a node
GridNode::~GridNode()
{}  // doesn't contain any pointers, so it is just empty

////////////////////////////////////////////////////////////////
// set the node id
void 
GridNode::setID(int id)
{
	this->nodeID = id;
}

////////////////////////////////////////////////////////////////
// set the x coordinate
void 
GridNode::setRow(int r)
{
	this->rCoord = r;
}

////////////////////////////////////////////////////////////////
// set the y coordinate
void 
GridNode::setColumn(int c)
{
	this->cCoord = c;
}

////////////////////////////////////////////////////////////////
// get the x and y coordinate of the node
int 
GridNode::getRow()
{
	return rCoord;
}

int 
GridNode::getColumn()
{
	return cCoord;
}

// return the position of this node
Ogre::Vector3 
GridNode::getPosition(int rows, int cols)
{
	Ogre::Vector3 t;
	t.z = (rCoord * NODESIZE) - (rows * NODESIZE)/2.0 + (NODESIZE/2.0); 
	t.y = 0; 
	t.x = (cCoord * NODESIZE) - (cols * NODESIZE)/2.0 + (NODESIZE/2.0); 
	return t;
}

////////////////////////////////////////////////////////////////
// set the node as walkable
void 
GridNode::setClear()
{
	this->clear = true;
	this->contains = '.';
}

////////////////////////////////////////////////////////////////
// set the node as occupied
void 
GridNode::setOccupied()
{
	this->clear = false;
	this->contains = 'B';
}

////////////////////////////////////////////////////////////////
// is the node walkable
bool 
GridNode::isClear()
{
	return this->clear;
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
// create a grid
Grid::Grid(Ogre::SceneManager* mSceneMgr, int numRows, int numCols)
{
	this->mSceneMgr = mSceneMgr; 

	assert(numRows > 0 && numCols > 0);
	this->nRows = numRows;
	this->nCols = numCols;

	data.resize(numCols, GridRow(numRows));
		
	// put the coordinates in each node
	int count = 0;
	for (int i = 0; i < numRows; i++) 
	{
		for (int j = 0; j < numCols; j++)
		{
			GridNode *n = this->getNode(i,j);
			n->setRow(i);
			n->setColumn(j);
			n->setID(count);
			count++;
		}
	}

	//vectors to store info for A* 
	//initialize with zeros and resize for given level
	fCosts.resize(this->nRows, std::vector<int>(this->nCols, 0));
	gCosts.resize(this->nRows, std::vector<int>(this->nCols, 0));
	hCosts.resize(this->nRows, std::vector<int>(this->nCols, 0));
	whichList.resize(this->nRows, std::vector<int>(this->nCols, 0));
	parents.resize(this->nRows, std::vector<GridNode*>(this->nCols, 0));
}

/////////////////////////////////////////
// destroy a grid
Grid::~Grid(){};  														

////////////////////////////////////////////////////////////////
// get the node specified 
GridNode* 
Grid::getNode(int r, int c)
{
	if (r >= nRows || c >= nCols || r < 0 || c < 0)	//check if out of bounds
		return NULL;

	return &this->data[c].data[r];
}

////////////////////////////////////////////////////////////////
// get adjacent nodes;
// utilizing the getNode method to check for neighbors
GridNode* 
Grid::getNorthNode(GridNode* n)
{
	if (n == NULL) { return NULL; }
	GridNode* neighbor = getNode(n->getRow()-1, n->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getSouthNode(GridNode* n)
{
	if (n == NULL) { return NULL; }
	GridNode* neighbor = getNode(n->getRow()+1, n->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getEastNode(GridNode* n)
{
	if (n == NULL) { return NULL; }
	GridNode* neighbor = getNode(n->getRow(), n->getColumn()+1);
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getWestNode(GridNode* n)
{
	if (n == NULL) { return NULL; }
	GridNode* neighbor = getNode(n->getRow(), n->getColumn()-1);
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getNENode(GridNode* n)  
{
	if (getNorthNode(n) == NULL || getEastNode(n) == NULL) { return NULL; }
	GridNode* neighbor = getNode(getNorthNode(n)->getRow(), getEastNode(n)->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getNWNode(GridNode* n) 
{
	if (getNorthNode(n) == NULL || getWestNode(n) == NULL) { return NULL; }
	GridNode* neighbor = getNode(getNorthNode(n)->getRow(), getWestNode(n)->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getSENode(GridNode* n) 
{
	if (getSouthNode(n) == NULL || getEastNode(n) == NULL) { return NULL; }
	GridNode* neighbor = getNode(getSouthNode(n)->getRow(), getEastNode(n)->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

GridNode* 
Grid::getSWNode(GridNode* n) 
{
	if (getSouthNode(n) == NULL || getWestNode(n) == NULL) { return NULL; }
	GridNode* neighbor = getNode(getSouthNode(n)->getRow(), getWestNode(n)->getColumn());
	if (neighbor == NULL || !neighbor->isClear()) { return NULL; }
	return neighbor;
}

////////////////////////////////////////////////////////////
//returns a vector of neighbors
//vector will contain a NULL for any out of bounds neighbors
//{North,South,East,West,NE,NW,SE,SW}
std::vector<GridNode*>
Grid::getAllNeighbors(GridNode* n)
{
	std::vector<GridNode*> neighbors;
	neighbors.push_back(getNorthNode(n));
	neighbors.push_back(getSouthNode(n));
	neighbors.push_back(getEastNode(n));
	neighbors.push_back(getWestNode(n));
	neighbors.push_back(getNENode(n));
	neighbors.push_back(getNWNode(n));
	neighbors.push_back(getSENode(n));
	neighbors.push_back(getSWNode(n));
	return neighbors;
}
////////////////////////////////////////////////////////////////
//get distance between between two nodes, by the number of nodes
//(vertically/horizontally)away times 10.
//return the Manhattan distance
int 
Grid::getDistance(GridNode* node1, GridNode* node2)
{
	if (node1 == NULL || node2 == NULL) { return NULL; }
	int distance;
	distance = std::abs(node2->getRow() - node1->getRow());				//  number of row nodes away
	distance += std::abs((node2->getColumn() - node1->getColumn()));	//+ number of col nodes away
	return distance * NODESIZE;											//= total # of nodes away * 10
}

void
Grid::setName(std::string name)
{
	this->levelName = name;
}

///////////////////////////////////////////////////////////////////////////////
// Print out the grid in ASCII
void 
Grid::printToFile()
{
	static int count = 0;
	if (count == 9) { count = 0; }
	count++;
	std::string str_count = static_cast<std::ostringstream*>( &(std::ostringstream() << count) )->str();
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path += levelName + str_count;	//include level name in agent's path file
	path += "Grid.txt";				//if txt file is in the same directory as cpp file
	std::ofstream outFile;
	outFile.open(path);

	if (!outFile.is_open()) // oops. there was a problem opening the file
	{
		std::cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	
		return;
	}

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			outFile << this->getNode(i, j)->contains << " ";
			if (this->getNode(i, j)->contains != '.' 
				&& this->getNode(i, j)->contains != 'B')
			{
				this->getNode(i, j)->contains = '.';
			}
		}
		outFile << std::endl;
	}
	outFile.close();
}

void // load and place a model in a certain location.
Grid::loadObject(std::string name, std::string filename, int row, int height, int col, float scale)
{
	using namespace Ogre;

	if (row >= nRows || col >= nCols || row < 0 || col < 0)
		return;

	Entity *ent = mSceneMgr->createEntity(name, filename);
    SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name,
        Ogre::Vector3(0.0f, 0.0f,  0.0f));
    node->attachObject(ent);
    node->setScale(scale, scale, scale);

	GridNode* gn = this->getNode(row, col);
	node->setPosition(getPosition(row, col)); 
	node->setPosition(getPosition(row, col).x, height, getPosition(row, col).z);
	gn->setOccupied();
	gn->entity = ent;
}

////////////////////////////////////////////////////////////////////////////
// Added this method and changed GridNode version to account for varying floor 
// plane dimensions. Assumes each grid is centered at the origin.
// It returns the center of each square. 
Ogre::Vector3 
Grid::getPosition(int r, int c)	
{
	Ogre::Vector3 t;
	t.z = (r * NODESIZE) - (this->nRows * NODESIZE)/2.0 + NODESIZE/2.0; 
	t.y = 0; 
	t.x = (c * NODESIZE) - (this->nCols * NODESIZE)/2.0 + NODESIZE/2.0; 
	return t;
}

////////////////////////////////////////////////////////////////////////////
// Added the next two methods to get the number of rows/cols
// to use when calling GridNode->getPosition(rows, cols)
int
Grid::getNumRows()
{
	return this->nRows;
}

int
Grid::getNumCols()
{
	return this->nCols;
}

////////////////////////////////////////////////////////////
//calculate a path from start to end avoiding all obstacles
std::deque<GridNode*> 
Grid::aStar(GridNode* start, GridNode* end)
{
	std::deque<GridNode*> path;				//optimal path to return

	GridNode* current_node = start;			//node we are currently checking
	GridNode* next_node = NULL;				//node to check next

	static const int diagonal_cost = sqrt(NODESIZE * NODESIZE + NODESIZE * NODESIZE);
	int lowest_fCost = NULL;
	char count = '0';						//for printing path with printFile()
	
	static int onOpenList = -1;				//values to use for if on open/closed list
	static int onClosedList = 0;			//	
	onOpenList += 5;						//these values will increment with each call to aStar, so old vals are obsolete
	onClosedList += 5;						//

	whichList[current_node->getRow()][current_node->getColumn()] = onClosedList;
	gCosts[current_node->getRow()][current_node->getColumn()] = 0;
	current_node->contains = 'S';

	while (whichList[end->getRow()][end->getColumn()] != onClosedList) //run until target node is on the closed list
	{
		//look at adjacent nodes and mark walkable nodes as onOpenList
		//and assign F, G, H values
		std::vector<GridNode*> neighbors = getAllNeighbors(current_node);
		for (unsigned int i = 0; i < neighbors.size(); i++)
		{
			// if neighbor is a valid adjacent node, assign costs and mark onOpenList
			if (neighbors[i] != NULL 
				&& whichList[neighbors[i]->getRow()][neighbors[i]->getColumn()] != onClosedList) 
			{
				//if not already marked onOpen
				if (whichList[neighbors[i]->getRow()][neighbors[i]->getColumn()] != onOpenList)
				{
					whichList[neighbors[i]->getRow()][neighbors[i]->getColumn()] = onOpenList;
					//assign Costs -------------------------------------------------------------------------------
					if (neighbors[i]->getRow() != current_node->getRow() 
						&& neighbors[i]->getColumn() != current_node->getColumn())
					{  //diagonal move (NE,NW,SE,SW)
						gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] = 
							diagonal_cost + gCosts[current_node->getRow()][current_node->getColumn()];
					}
					else
					{  //horizontal/vertical move (N,S,E,W)
						gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] = 
							NODESIZE + gCosts[current_node->getRow()][current_node->getColumn()];
					}
					parents[neighbors[i]->getRow()][neighbors[i]->getColumn()] = current_node;
					hCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] = getDistance(neighbors[i], end);
					fCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] =
													  gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()]
													+ hCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()];
					//Costs assigned ------------------------------------------------------------------------------
					neighbors[i]->contains = '-'; //displays open list nodes in print to file
				}
				else //node is already marked onOpenList
				{
					//calculate possible new gCost ----------------------------------------------------------------
					int new_gCost;	
					if (neighbors[i]->getRow() != current_node->getRow() 
						&& neighbors[i]->getColumn() != current_node->getColumn())
					{  //diagonal move (NE,NW,SE,SW)
						new_gCost = diagonal_cost + gCosts[current_node->getRow()][current_node->getColumn()];
					}
					else
					{  //horizontal/vertical move (N,S,E,W)
						new_gCost = NODESIZE + gCosts[current_node->getRow()][current_node->getColumn()];
					}

					//if new cost is lower, change the parent and recalculate F ////////
					if (new_gCost < gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()])
					{
						gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] = new_gCost;
						parents[neighbors[i]->getRow()][neighbors[i]->getColumn()] = current_node;
						hCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] = getDistance(neighbors[i], end);
						fCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()] =
													  gCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()]
													+ hCosts[neighbors[i]->getRow()][neighbors[i]->getColumn()];
					}
					//else do nothing
					//Costs re-assigned ----------------------------------------------------------------------------
				}
			}
		}//end for

		//at this point, nodes on the open list will have re-assigned F values
		//Pick the node with the lowest F value ------------------------------------------------
		for (int i = 0; i < getNumRows(); i++)
		{
			for (int j = 0; j < getNumCols(); j++)
			{
				if (whichList[i][j] == onOpenList)	//check only nodes marked onOpenList
				{
					if (lowest_fCost == NULL || fCosts[i][j] <= lowest_fCost)
					{
						lowest_fCost = fCosts[i][j];
						next_node = getNode(i,j);
					}
				}
			}
		}
		if (lowest_fCost == NULL) // No path available
		{
			//std::cout << "No path!!" << std::endl;
			start->contains = 'X';	//X for no path found
			end->contains = 'E';
			printToFile();
			return path; //return empty path
		} 
		//node is picked, assign to current node and mark onClosedList
		lowest_fCost = NULL;
		current_node = next_node;
		whichList[current_node->getRow()][current_node->getColumn()] = onClosedList;
		current_node->contains = '~';	//displays closed list nodes in the print to file
		// --------------------------------------------------------------------------------------

	}//end while

	//set up deque for path to return
	while (current_node != start)
	{
		path.push_front(current_node);
		current_node = parents[current_node->getRow()][current_node->getColumn()];
	}
	//and assign path numbers
	for (unsigned int i = 0; i < path.size(); i++)
	{
		path[i]->contains = count;
		if (count == '9') { count = '0'; }
		else count++;
	}
	end->contains = 'E';
	printToFile(); //move this to while their running instead of before
	return path;
}