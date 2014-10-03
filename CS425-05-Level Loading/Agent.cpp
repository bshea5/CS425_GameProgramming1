#include "Agent.h"

Agent::Agent(Ogre::SceneManager* SceneManager, std::string name, std::string filename, float height, float scale)
{
	using namespace Ogre;

	mSceneMgr = SceneManager; // keep a pointer to where this agent will be

	if (mSceneMgr == NULL)
	{
		std::cout << "ERROR: No valid scene manager in Agent constructor" << std::endl;
		return;
	}

	this->height = height;
	this->scale = scale;

	mBodyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(); // create a new scene node
	mBodyEntity = mSceneMgr->createEntity(name, filename); // load the model
	mBodyNode->attachObject(mBodyEntity);	// attach the model to the scene node

	mBodyNode->translate(0,height,0); // make the Ogre stand on the plane (almost)
	mBodyNode->scale(scale,scale,scale); // Scale the figure

	setupAnimations();  // load the animation for this character

	//genWalkList();

	// configure walking parameters
	mWalking = false;
	mWalkSpeed = 35.0f;	
	mDirection = Ogre::Vector3::ZERO;

	mGrid = NULL;
	mGridNode = NULL;
}

Agent::~Agent(){
	// mSceneMgr->destroySceneNode(mBodyNode); // Note that OGRE does not recommend doing this. It prefers to use clear scene
	// mSceneMgr->destroyEntity(mBodyEntity);
}

//set the position of the agent via coordinates
void 
Agent::setPosition(float x, float y, float z)
{
	this->mBodyNode->setPosition(x, y + height, z);
}

void
Agent::claimNode(GridNode* n)
{
	this->mGridNode = n;
	//also needs to tell node this, so grid knows whats up
}

void
Agent::setGrid(Grid* g)
{
	this->mGrid = g;
}

// update is called at every frame from GameApplication::addTime
void
Agent::update(Ogre::Real deltaTime) 
{
	this->updateAnimations(deltaTime);	// Update animation playback
	this->updateLocomote(deltaTime);	// Update Locomotion
}


void 
Agent::setupAnimations()
{
	this->mTimer = 0;	// Start from the beginning
	this->mVerticalVelocity = 0;	// Not jumping

	// this is very important due to the nature of the exported animations
	mBodyEntity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);

	// Name of the animations for this character
	Ogre::String animNames[] =
		{"IdleBase", "IdleTop", "RunBase", "RunTop", "HandsClosed", "HandsRelaxed", "DrawSwords",
		"SliceVertical", "SliceHorizontal", "Dance", "JumpStart", "JumpLoop", "JumpEnd"};

	// populate our animation list
	for (int i = 0; i < 13; i++)
	{
		mAnims[i] = mBodyEntity->getAnimationState(animNames[i]);
		mAnims[i]->setLoop(true);
		mFadingIn[i] = false;
		mFadingOut[i] = false;
	}

	// start off in the idle state (top and bottom together)
	setBaseAnimation(ANIM_IDLE_BASE);
	setTopAnimation(ANIM_IDLE_TOP);

	// relax the hands since we're not holding anything
	mAnims[ANIM_HANDS_RELAXED]->setEnabled(true);
}

void 
Agent::setBaseAnimation(AnimID id, bool reset)
{
	if (mBaseAnimID >= 0 && mBaseAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mBaseAnimID] = false;
		mFadingOut[mBaseAnimID] = true;
	}

	mBaseAnimID = id; 

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}
	
void Agent::setTopAnimation(AnimID id, bool reset)
{
	if (mTopAnimID >= 0 && mTopAnimID < 13)
	{
		// if we have an old animation, fade it out
		mFadingIn[mTopAnimID] = false;
		mFadingOut[mTopAnimID] = true;
	}

	mTopAnimID = id;

	if (id != ANIM_NONE)
	{
		// if we have a new animation, enable it and fade it in
		mAnims[id]->setEnabled(true);
		mAnims[id]->setWeight(0);
		mFadingOut[id] = false;
		mFadingIn[id] = true;
		if (reset) mAnims[id]->setTimePosition(0);
	}
}

void 
Agent::updateAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	Real baseAnimSpeed = 1;
	Real topAnimSpeed = 1;

	mTimer += deltaTime; // how much time has passed since the last update

	// Commented out to fix run animations :)
	//if (mTopAnimID != ANIM_IDLE_TOP)
	//if (mTopAnimID != ANIM_NONE)
	//if (mTimer >= mAnims[mTopAnimID]->getLength())
	//	{
	//		setTopAnimation(ANIM_IDLE_TOP, true);
	//		setBaseAnimation(ANIM_IDLE_BASE, true);
	//		mTimer = 0;
	//	}
	
	// increment the current base and top animation times
	if (mBaseAnimID != ANIM_NONE) mAnims[mBaseAnimID]->addTime(deltaTime * baseAnimSpeed);
	if (mTopAnimID != ANIM_NONE) mAnims[mTopAnimID]->addTime(deltaTime * topAnimSpeed);

	// apply smooth transitioning between our animations
	fadeAnimations(deltaTime);
}

void 
Agent::fadeAnimations(Ogre::Real deltaTime)
{
	using namespace Ogre;

	for (int i = 0; i < 13; i++)
	{
		if (mFadingIn[i])
		{
			// slowly fade this animation in until it has full weight
			Real newWeight = mAnims[i]->getWeight() + deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight >= 1) mFadingIn[i] = false;
		}
		else if (mFadingOut[i])
		{
			// slowly fade this animation out until it has no weight, and then disable it
			Real newWeight = mAnims[i]->getWeight() - deltaTime * 7.5f; //ANIM_FADE_SPEED;
			mAnims[i]->setWeight(Math::Clamp<Real>(newWeight, 0, 1));
			if (newWeight <= 0)
			{
				mAnims[i]->setEnabled(false);
				mFadingOut[i] = false;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
//checks to see if there is a destination available in the mWalkList
//if so, it sets up the variables for the agent's movement to the
//next destination.
bool 
Agent::nextLocation()
{
	if ( mWalkList.empty() ) //any destinations to walk to?
	{
		mWalking = false;
		return false;
	}
	mWalking = true;

	mDestination = mWalkList.front();	// get next destination
	mWalkList.pop_front();				// remove from queue

	mDirection = mDestination - mBodyNode->getPosition();	//set direction

	mDistance = mDirection.normalise();

	// Rotation code will go here, moved from updateLocomote
 	Ogre::Vector3 src = mBodyNode->getOrientation() * Ogre::Vector3::UNIT_Z;
	if ( (1.0f + src.dotProduct(mDirection)) < 0.0001f) 
	{
		mBodyNode->yaw(Ogre::Degree(180));
	}
	else 
	{
		Ogre::Quaternion quat = src.getRotationTo(mDirection);
		mBodyNode->rotate(quat);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////
//update animations and move the agent according to deltaTime
void 
Agent::updateLocomote(Ogre::Real deltaTime)
{
	if ( !mWalking ) //faster to use a bool than compare vectors
	{ 
		if ( nextLocation() ) 
		{
			//set running animation
			setBaseAnimation(ANIM_RUN_BASE);
			setTopAnimation(ANIM_RUN_TOP);
		}
	}
	else {
		Ogre::Real move = (mWalkSpeed * deltaTime);
		mDistance -= move; 

		if ( mDistance <= 0.0f )	// destination reached!
		{
			mBodyNode->setPosition(mDestination);
			mDirection = Ogre::Vector3::ZERO;

			if ( !nextLocation() )	//no other point to walk to, Idle ogre is idle
			{
				// set Idle animation
				setBaseAnimation(ANIM_IDLE_BASE);
				setTopAnimation(ANIM_IDLE_TOP);
				return;
			}
		}
		else 
		{
			mBodyNode->translate(mDirection * move);
		}
	}
}

///////////////////////////////////////////////
//generates a list of random points to walk to
//Not Grid based. Code from last assignment
void
Agent::genWalkList() 
{  
	float x, y;
	for (int i = 0; i < 15; i++)
	{
		x = rand() % 60 - 30;
		y = rand() % 60 - 30;
		mWalkList.push_back(Ogre::Vector3(x, this->height, y));
	}
}

///////////////////////////////////////////////////////////////////////
//walk to gridnode n by passing the position of that gridnode to the mWalklist
//when mWalklist is checked, the ogre will walk to this point.
//presently overrids old destination whenever called.
//adding aStar method call inside to get a path to destination, going around obstacles
void
Agent::walkTo(GridNode* n) 
{   
	Ogre::Vector3 destination = n->getPosition( mGrid->getNumRows(), mGrid->getNumCols() );
	destination[1] = this->height + height;	//this keeps the orge above the grid
	if ( mWalking )	// overrides old destination
	{
		mWalking = false;
		mWalkList.clear();
	}
	mWalkList.push_back( destination );	//pass destination to walklist
	mGridNode = n; // TODO: move this somewhere else
}

///////////////////////////////////////////////////////////////////////
//calculate a path to the given node destination avoiding all obstacles
//utilizes A* algorithm to find the optimal path
void
Agent::moveTo(GridNode* n)
{
	if (n== NULL || !n->isClear()) { return; }

	std::deque<GridNode*> path = mGrid->aStar(mGridNode, n);
	while (!path.empty())
	{
		walkTo(path.front());
		path.pop_front();
	}	
}
