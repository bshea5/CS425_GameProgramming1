Author: Brandon Shea
Assignment 5 'Boids'
CS425 Game Programming 1

Overview:
This program is an implementation of flocking using boids.  Each agent will come together and move to each goal point.  Space bar will trigger the demo event, moving the flock from one demo goal to the next; in total 12. 

WARNING: 
If you press space bar during the demo, it will exit demo mode and move the group to random nodes with each press, but still flocking.

Issues: (lots)
Right now, the group stops whenever one of the boids gets to the goal.  This could cause problems if the distance check is too small. They could miss the target and never stop walking.

Doesn't allow multiple flocks at the moment

distance check for joining a flock is large right now, so they all are in the same flock rather immedietly.

vFlock seems to be affecting the agents while standing.

rotation code is commented out for vFlock, causes break dancing. Still updates for nextLocation though.

Notes:
the demo goals are the sparklers.
