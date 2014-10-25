#ifndef __GameApplication_h_
#define __GameApplication_h_
#pragma once
#include "BaseApplication.h"
#include "Agent.h"

// forward declarations ----------------
class Agent;
class Grid;
class GridNode;
//--------------------------------------

class GameApplication : public BaseApplication
{
private:
	Agent* agent; // store a pointer to the character
	std::list<Agent*> agentList; // Lecture 5: now a list of agents
	Grid* grid;	// store a pointer to the grid
	std::deque<GridNode*> demoGoals; //list of locations to walk to for flocking demo
	bool demoMode;		//game is running demo mode
public:
    GameApplication(void);
    virtual ~GameApplication(void);

	void loadEnv();			// Load the buildings or ground plane, etc.
	void setupEnv();		// Set up the lights, shadows, etc
	void loadObjects();		// Load other props or objects (e.g. furniture)
	void loadCharacters();	// Load actors, agents, characters

	void addTime(Ogre::Real deltaTime);		// update the game state

	//////////////////////////////////////////////////////////////////////////
	// keyboard interaction
    bool keyPressed( const OIS::KeyEvent &arg );
    bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    bool mouseMoved( const OIS::MouseEvent &arg );
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	////////////////////////////////////////////////////////////////////////////
	std::list<Agent*> getAgentList();	//return the current agent list
	bool inDemoMode() { return demoMode; }	//check if in demo mode

protected:
    virtual void createScene(void);
};

#endif // #ifndef __TutorialApplication_h_
