# Galaxy

A 2D game inspired by Gem Swap and Space Invaders, Galaxy features an UFO avatar navigating through a grid of asteroids. The UFO avatar is trailed by a small goldfish (non-aggro) and can shoot other objects, create mini black holes, etc.

**Preview**

The following shows some of the implementations in action

Black hole | Flamethrower |
------------ | ------------- | 
<img src="https://github.com/eutopi/galaxy/blob/master/Galaxy/galaxy1.png" alt="drawing" width="400"/> | <img src="https://github.com/eutopi/galaxy/blob/master/Galaxy/galaxy3.png" alt="drawing" width="400"/> 
<img src="https://github.com/eutopi/galaxy/blob/master/Galaxy/galaxy2.png" alt="drawing" width="400"/> | <img src="https://github.com/eutopi/galaxy/blob/master/Galaxy/galaxy4.png" alt="drawing" width="400"/>

## Controls
- [W][A][S][D] keys to move
- [SPACEBAR] to shoot a projectile directly upwards
- [B] to spawn a black hole
- [Q] to instigate a violent quake
- [HOLD MOUSE] to shoot constant stream of fireballs


## Features 
1. **Grid** - displays a n*n grid of asteroids (semi-transparent textured quads with different images).
2. **Quake** - the visible window shakes violently accordingly to a high-freq sin function of time. During the quake, in every frame, all asteroids stand a 0.1% chance of disappearing.
3. **Dramatic exit** - when asteroids disappearing due to Quake, they do so by quickly, but gradually shrinking to zero size, while rotating.
4. **Rocket science** - movement is according to acceleration instead of constant speed.
5. **Shoot 'em up** - when projectiles collide with other objects or fly a certain distance away from the avatar, they disappear. There is a cooldown time of ~2 seconds per shot. 
6. **Path animation** - some enemies move along parametric curves.
7. **Seeker** - some enemies are constantly accelerated towards the avatar.
8. **BOOM!** - when collision occurs, animated explosion is displayed using a semi-transparent quad, its texture including sprites of all movement phases, and the vertex shader adjusting texture coordinates to show the proper phase, received as a uniform parameter.
9. **Flamethrower** - fired towards the location of the mouse.
10. **Black holes** - attract other objects according to the Law of Gravitation.

## Note
To render each texture properly, must hardcode the paths of each image file.

## Frameworks
- OpenGL
- GLUT
