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
	/////////////////////
	// TESTING STUFF ////

	std::cout << getSouthNode(getNode(1,1))->getID() << std::endl;

	//test manhattan distance accross the grid ///////////////////////
	int manDist = getDistance(getNode(0,0), getNode(nRows-1, nCols-1));
	std::cout << "Manhattan distance across the board is " << manDist << " nodes." << std::endl;
	//////////////////////////////////////////////////////////////////

	//test get neighbors ////////////////////////////////////////////
	std::vector<GridNode*> neighbors = getAllNeighbors(getNode(0,0));
	std::cout << "Neighbors of node " << getNode(0,0)->getID() << " are..." << std::endl;
	for (int i = 0; i < neighbors.size(); i++)
	{
		if (neighbors[i] != NULL)
			std::cout << neighbors[i]->getID() << std::endl;
	}
	/////////////////////////////////////////////////////////////////

	// END TESTING /////
	////////////////////
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
	return getNode(n->getRow()-1, n->getColumn());
}

GridNode* 
Grid::getSouthNode(GridNode* n)
{
	return getNode(n->getRow()+1, n->getColumn());
}

GridNode* 
Grid::getEastNode(GridNode* n)
{
	return getNode(n->getRow(), n->getColumn()+1);
}

GridNode* 
Grid::getWestNode(GridNode* n)
{
	return getNode(n->getRow(), n->getColumn()-1);
}

GridNode* 
Grid::getNENode(GridNode* n)  
{
	return getNode(n->getRow()-1, n->getColumn()+1);
}

GridNode* 
Grid::getNWNode(GridNode* n) 
{
	return getNode(n->getRow()-1, n->getColumn()-1);
}

GridNode* 
Grid::getSENode(GridNode* n) 
{
	return getNode(n->getRow()+1, n->getColumn()+1);
}

GridNode* 
Grid::getSWNode(GridNode* n) 
{
	return getNode(n->getRow()+1, n->getColumn()-1);
}

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
//get distance between between two nodes
//return the Manhattan distance
int 
Grid::getDistance(GridNode* node1, GridNode* node2)
{
	int distance;
	distance = std::abs(node2->getRow() - node1->getRow());				//  number of row nodes away
	distance += std::abs((node2->getColumn() - node1->getColumn()));	//+ number of col nodes away
	return distance;													//= total # of nodes away
}

///////////////////////////////////////////////////////////////////////////////
// Print out the grid in ASCII
void 
Grid::printToFile()
{
	std::string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= "Grid.txt"; //if txt file is in the same directory as cpp file
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