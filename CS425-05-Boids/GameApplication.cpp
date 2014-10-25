#include "GameApplication.h"
#include "Grid.h" // Lecture 5
#include <fstream>
#include <sstream>
#include <map> 

//-------------------------------------------------------------------------------------
GameApplication::GameApplication(void)
{
	agent = NULL; // Init member data
	grid = NULL;
	demoMode = false;
}
//-------------------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
	if (agent != NULL)  // clean up memory
		delete agent; 
	if (!agentList.empty())
		agentList.clear();
	if (grid != NULL)
		delete grid;
	if (!demoGoals.empty())
		demoGoals.clear();
}

//-------------------------------------------------------------------------------------
void GameApplication::createScene(void)
{
    loadEnv();
	setupEnv();
	loadObjects();
	loadCharacters();
}
//////////////////////////////////////////////////////////////////
// Lecture 5: Returns a unique name for loaded objects and agents
std::string getNewName() // return a unique name 
{
	static int count = 0;	// keep counting the number of objects

	std::string s;
	std::stringstream out;	// a stream for outputing to a string
	out << count++;			// make the current count into a string
	s = out.str();

	return "object_" + s;	// append the current count onto the string
}

// Lecture 5: Load level from file!
void // Load the buildings or ground plane, etc
GameApplication::loadEnv()
{
	using namespace Ogre;	// use both namespaces
	using namespace std;

	class readEntity // need a structure for holding entities
	{
	public:
		string filename;
		float y;
		float scale;
		float orient;
		bool agent;
	};

	ifstream inputfile;		// Holds a pointer into the file
	const string fileName = "levelBoids_big.txt";
	string path = __FILE__; //gets the current cpp file's path with the cpp file
	path = path.substr(0,1+path.find_last_of('\\')); //removes filename to leave path
	path+= fileName;	//if txt file is in the same directory as cpp file
	inputfile.open(path);

	if (!inputfile.is_open()) // oops. there was a problem opening the file
	{
		cout << "ERROR, FILE COULD NOT BE OPENED" << std::endl;	// Hmm. No output?
		return;
	}

	// the file is open
	int x,z;
	inputfile >> x >> z;	// read in the dimensions of the grid
	string matName;
	inputfile >> matName;	// read in the material name

	// create floor mesh using the dimension read
	MeshManager::getSingleton().createPlane("floor", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
		Plane(Vector3::UNIT_Y, 0), x*NODESIZE, z*NODESIZE, x, z, true, 1, x, z, Vector3::UNIT_Z);
	
	//create a floor entity, give it material, and place it at the origin
	Entity* floor = mSceneMgr->createEntity("Floor", "floor");
	floor->setMaterialName(matName);
	floor->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->attachObject(floor);

	this->grid = new Grid(mSceneMgr, z, x); // Set up the grid. z is rows, x is columns
	this->grid->setName(fileName);

	string buf;
	inputfile >> buf;	// Start looking for the Objects section
	while  (buf != "Objects")
		inputfile >> buf;
	if (buf != "Objects")	// Oops, the file must not be formated correctly
	{
		cout << "ERROR: Level file error" << endl;
		return;
	}

	// read in the objects
	readEntity *rent = new readEntity();	// hold info for one object
	std::map<string,readEntity*> objs;		// hold all object and agent types;
	while (!inputfile.eof() && buf != "Characters") // read through until you find the Characters section
	{ 
		inputfile >> buf;			// read in the char
		if (buf != "Characters")
		{
			inputfile >> rent->filename >> rent->y >> rent->orient >> rent->scale;  // read the rest of the line
			rent->agent = false;		// these are objects
			objs[buf] = rent;			// store this object in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}

	while  (buf != "Characters")	// get through any junk
		inputfile >> buf;
	
	// Read in the characters
	while (!inputfile.eof() && buf != "World") // Read through until the world section
	{
		inputfile >> buf;		// read in the char
		if (buf != "World")
		{
			inputfile >> rent->filename >> rent->y >> rent->scale; // read the rest of the line
			rent->agent = true;			// this is an agent
			objs[buf] = rent;			// store the agent in the map
			rent = new readEntity();	// create a new instance to store the next object
		}
	}
	delete rent; // we didn't need the last one

	// read through the placement map
	char c;
	for (int i = 0; i < z; i++)			// down (row)
		for (int j = 0; j < x; j++)		// across (column)
		{
			inputfile >> c;			// read one char at a time
			buf = c + '\0';			// convert char to string
			rent = objs[buf];		// find cooresponding object or agent
			if (rent != NULL)		// it might not be an agent or object
				if (rent->agent)	// if it is an agent...
				{
					// Use subclasses instead!
					agent = new Agent(this, this->mSceneMgr, getNewName(), rent->filename, rent->y, rent->scale);
					agentList.push_back(agent);
					agent->setPosition(grid->getPosition(i,j).x, rent->y, grid->getPosition(i,j).z);
					agent->setGrid(grid);						// pass pointer for grid to agent
					agent->claimNode(grid->getNode(i,j));		// pass pointer for the gridNode agent is in

					// If we were using different characters, we'd have to deal with 
					// different animation clips. 
				}
				else	// Load objects
				{
					grid->loadObject(getNewName(), rent->filename, i, rent->y, j, rent->scale);
					grid->getNode(i,j)->setOccupied();	// set node with an object as occupied
				}
			else // not an object or agent
			{
				if (c == 'w') // create a wall
				{
					Entity* ent = mSceneMgr->createEntity(getNewName(), Ogre::SceneManager::PT_CUBE);
					ent->setMaterialName("Examples/RustySteel");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ent);
					mNode->scale(0.1f,0.2f,0.1f);		// cube is 100 x 100
					grid->getNode(i,j)->setOccupied();  // indicate that agents can't pass through
					mNode->setPosition(grid->getPosition(i,j).x, 10.0f, grid->getPosition(i,j).z);
				}
				else if (c == 'e')
				{
					ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(grid->getPosition(i,j).x, 60.0f, grid->getPosition(i,j).z);
				}
				else if (c == 'g') //set a goal point to run to with markers
				{
					//set markers for goals in game
					ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);  // set nonvisible timeout
					ParticleSystem* ps = mSceneMgr->createParticleSystem(getNewName(), "Examples/PurpleFountain");
					Ogre::SceneNode* mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
					mNode->attachObject(ps);
					mNode->setPosition(grid->getPosition(i,j).x, 0.0f, grid->getPosition(i,j).z);

					demoGoals.push_back(grid->getNode(i,j));	//append node to demo walk list
				}
			}
		}
	
	// delete all of the readEntities in the objs map
	rent = objs["s"]; // just so we can see what is going on in memory (delete this later)
	
	std::map<string,readEntity*>::iterator it;
	for (it = objs.begin(); it != objs.end(); it++) // iterate through the map
	{
		delete (*it).second; // delete each readEntity
	}
	objs.clear(); // calls their destructors if there are any. (not good enough)
	
	inputfile.close();
	grid->printToFile(); // see what the initial grid looks like.
	if (!demoGoals.empty()) { demoMode = true; } //toggle demo mode 
}

void // Set up lights, shadows, etc
GameApplication::setupEnv()
{
	using namespace Ogre;

	// set shadow properties
	mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
	mSceneMgr->setShadowColour(ColourValue(0.5, 0.5, 0.5));
	mSceneMgr->setShadowTextureSize(1024);
	mSceneMgr->setShadowTextureCount(1);

	// disable default camera control so the character can do its own 
	mCameraMan->setStyle(OgreBites::CS_FREELOOK); // CS_FREELOOK, CS_ORBIT, CS_MANUAL

	// use small amount of ambient lighting
	mSceneMgr->setAmbientLight(ColourValue(0.3f, 0.3f, 0.3f));

	// add a bright light above the scene
	Light* light = mSceneMgr->createLight();
	light->setType(Light::LT_POINT);
	light->setPosition(-10, 40, 20);
	light->setSpecularColour(ColourValue::White);

	//mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8); // Lecture 4
}

void // Load other props or objects
GameApplication::loadObjects()
{

	// Lecture 5: comment out all because loading from file
	//using namespace Ogre;
	//
	//Ogre::Entity *ent;
	//Ogre::SceneNode *node;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//// Lecture 4-hold
	//ent = mSceneMgr->createEntity("Gun", "38pistol.mesh");
	//node = mSceneMgr->getRootSceneNode()->createChildSceneNode("GunNode", Ogre::Vector3(5.0f, 1.4f,  5.0f));
	//node->attachObject(ent);
	//node->pitch(Degree(90));
	//node->setScale(0.1f, 0.1f, 0.1f);

	//ent = mSceneMgr->createEntity("Gun2", "9mm.mesh");
	//node = mSceneMgr->getRootSceneNode()->createChildSceneNode("GunNode2", Ogre::Vector3(5.0f, 1.4f,  5.0f));
	//node->attachObject(ent);
	//node->pitch(Degree(90));
	//node->setScale(0.1f, 0.1f, 0.1f);
	/////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	//// Draw a line: Lecture 4
	//Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1"); 
	//Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node"); 
 //
	//// NOTE: The second parameter to the create method is the resource group the material will be added to.
	//// If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
	//Ogre::MaterialPtr myManualObjectMaterial = Ogre::MaterialManager::getSingleton().create("manual1Material","General"); 
	//myManualObjectMaterial->setReceiveShadows(false); 
	//myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true); 
	//myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0); 
	//myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(1,0,0); 
	//myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0); 
	////myManualObjectMaterial->dispose();  // dispose pointer, not the material
 // 
	//myManualObject->begin("manual1Material", Ogre::RenderOperation::OT_LINE_LIST); 
	//myManualObject->position(3, 2, 1); 
	//myManualObject->position(8, 1, 0); 
	//// etc 
	//myManualObject->end(); 
 //
	//myManualObjectNode->attachObject(myManualObject);
	/////////////////////////////////////////////////////////////////////
}

void // Load actors, agents, characters
GameApplication::loadCharacters()
{
	// Lecture 5: now loading from file
	// agent = new Agent(this->mSceneMgr, "Sinbad", "Sinbad.mesh");
}

void
GameApplication::addTime(Ogre::Real deltaTime)
{
	// Lecture 5: Iterate over the list of agents
	std::list<Agent*>::iterator iter;
	for (iter = agentList.begin(); iter != agentList.end(); iter++)
		if (*iter != NULL)
			(*iter)->update(deltaTime);
}

bool 
GameApplication::keyPressed( const OIS::KeyEvent &arg ) // Moved from BaseApplication
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::PolygonMode pm;

        switch (mCamera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            newVal = "Wireframe";
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            newVal = "Points";
            pm = Ogre::PM_POINTS;
            break;
        default:
            newVal = "Solid";
            pm = Ogre::PM_SOLID;
        }

        mCamera->setPolygonMode(pm);
        mDetailsPanel->setParamValue(10, newVal);
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)		// exit
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_SPACE)		//run'dem ogres with spacebar w/ flocking
	{
		if (demoMode)	//running demo 
		{
			std::cout << "Demo mode: running to set goals\n"
				<< "press spacebar again to toggle off and run to random points" << std::endl;
			while (!demoGoals.empty())		//run to demo goals
			{
				(*agentList.begin())->toggleFlocking();
				std::list<Agent*>::iterator iter;
				for (iter = agentList.begin(); iter != agentList.end(); iter++)
				{
					(*iter)->walkTo(demoGoals.front());
				}
				demoGoals.pop_front();
			}
			demoMode = false; //toggle off demo mode. I would like this to trigger elsewhere
		}
		else {		//else run to random points
			int x = rand() % grid->getNumRows();	//get a random row
			int y = rand() % grid->getNumCols();	//get a random column
													//moved to give all agents same dest.
			std::cout << "row: " << x << "col: " << y << std::endl;

			std::list<Agent*>::iterator iter;
			for (iter = agentList.begin(); iter != agentList.end(); iter++)
			{			
				if (!(*iter)->isFlocking()) { (*iter)->toggleFlocking(); }
				(*iter)->walkTo(grid->getNode(x, y));		//run to node
			}
		}
	}
	else if (arg.key == OIS::KC_LCONTROL)	//run A* movement (TODO: fix it )
	{
		if (!demoMode) 
		{
			std::list<Agent*>::iterator iter;
			for (iter = agentList.begin(); iter != agentList.end(); iter++)
			{		
				int x = rand() % grid->getNumRows();	//get a random row
				int y = rand() % grid->getNumCols();	//get a random column

				(*iter)->moveTo(grid->getNode(x, y));		//run to node, avoid obstacles
			}
		}
	}
   
    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GameApplication::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GameApplication::mouseMoved( const OIS::MouseEvent &arg )
{
    if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
    return true;
}

bool GameApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GameApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

std::list<Agent*> 
GameApplication::getAgentList()
{
	return this->agentList;
}