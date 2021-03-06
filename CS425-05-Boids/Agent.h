#include "BaseApplication.h"
#include <deque>

#pragma once
#include "Grid.h"
#include "GameApplication.h"

#define CSEPERATE 1.0
#define CALIGN 1.0
#define CCOHESION 1.0
#define KSEPERATE 0.5
#define KALIGN 0.5
#define KCOHESION 0.01

//forward declarations -----
class GridNode;
class Grid;
class GameApplication;
//--------------------------

class Agent
{
private:
	Ogre::SceneManager* mSceneMgr;		// pointer to scene graph
	Ogre::SceneNode* mBodyNode;			
	Ogre::Entity* mBodyEntity;
	float height;						// height the character should be moved up
	float scale;						// scale of character from original model

	// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together
	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

	Ogre::AnimationState* mAnims[13];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[13];						// which animations are fading in
	bool mFadingOut[13];					// which animations are fading out
	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	void setupAnimations();							// load this character's animations
	void fadeAnimations(Ogre::Real deltaTime);		// blend from one animation to another
	void updateAnimations(Ogre::Real deltaTime);	// update the animation frame

	// for A*
	Grid* mGrid;							// pointer to the current grid the agent is in
	GridNode* mGridNode;					// node the agent currently occupies 
	GridNode* mNextNode;					// destination node

	// for flocking
	GameApplication* mGame;					// a pointer to the gameapplication, will use for agent list
	Ogre::Vector3 vFlock();					// calculate the flocking velocity
	//Ogre::Vector3 vSeparate();				// calculate the separation velocity
	//Ogre::Vector3 vAlign();					// calculate the alignment velocity
	//Ogre::Vector3 vCohesion();				// calculate the cohesion velocity
	void assimilate();						// bring neighbors into the flock
	bool mFlocking;							// is the agent flocking with other agents?

	// for locomotion
	bool mWalking;							// is the agent walking presently?
	Ogre::Real mDistance;					// The distance the agent has left to travel
	Ogre::Vector3 mDirection;				// The direction the object is moving
	Ogre::Vector3 mDestination;				// The destination the object is moving towards
	std::deque<Ogre::Vector3> mWalkList;	// The list of points we are walking to
	Ogre::Real mWalkSpeed;					// The speed at which the object is moving
	bool nextLocation();					// Is there another destination?
	void updateLocomote(Ogre::Real deltaTime);		// update the character's walking
	bool procedural;						// Is this character performing a procedural animation
	void rotate(Ogre::Vector3 towards);		// rotate agent towards goal

public:
	Agent(GameApplication* game, Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale);
	~Agent();
	void setPosition(float x, float y, float z);	//set position by coordinates

	void claimNode(GridNode* n);		//set pointer to current grid node agent is occupying
	void setGrid(Grid* g);				//set pointer to grid level agent is in

	void update(Ogre::Real deltaTime);		// update the agent
	
	void setBaseAnimation(AnimID id, bool reset = false);	// choose animation to display
	void setTopAnimation(AnimID id, bool reset = false);

	void genWalkList();				// generate a random walk list
	void walkTo(GridNode* n);		// walk character from current location to destination node
	void walkTo(Ogre::Vector3 dest);// walk character from current location to destination position
	//void addToWalkList(GridNode* n);	// add destinations to walk list
	void moveTo(GridNode* n);		// calculate path to destination 
	bool isFlocking() { return mFlocking; }	//return if agent is flocking
	void toggleFlocking() { mFlocking = !mFlocking; } //toggle flocking on/off
};