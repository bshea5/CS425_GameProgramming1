Author: Brandon Shea
Assignment 4 'Avoidin'
CS425 Game Programming 1

Overview:
This program utilizes the A* algorithm to find the optimal path from a starting point to an end point. By hitting the spacebar, you will generate for each agent, a random node to run to.  Each agent will then find the optimal path to get there, and move to each node along the path.  Be aware that hitting the spacebar will sometimes pass a blocked or unreachable location, which will cause that particular agent to not move.  If it was because of an unreachable destination, a printfile will still be called so that you can see where the agent checked for paths.  Speaking of the printfile, after an agent has found its optimal path to a node, it will generate a text file showing its path taken, and also the nodes on the open or closed lists.

PrintFile INFO:
Numbers '0'-'9' = path from start to end
'S' = Start
'E' = End
'B' = Blocked
'X' = No path available from this point
'~' = Closed
'-' = Open 

Each level can generate up to 10 text files, after which, previous files will start to be overwritten.
You will find 5 files for testing A* out. level0B1-level0B5
Their path files will include the level names.

Note:
+ spacebar will not always generate a valid node, and might require additional presses for maps with lots of blocked nodes and invalid paths

+ spacebar will only generate a movement if the agent is not already moving
+ if one agent is moving but another isn't, spacebar will generate a path for the agent not moving.

Known Bugs:
-Still using the height + height hack to keep the ogre above the ground.
-There is a pause while generating paths on large grids with multiple agents
