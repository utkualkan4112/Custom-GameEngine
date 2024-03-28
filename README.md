# Custom-GameEngine
 This is an experimental OpenGL graphics engine for testing some of the current technology implementation
# Collusion Detection
 Octat trees have been used for collision detection. Programs allow custom collision mesh also it assigns a sphere collusion that fits the model automatically.
# Collusion Response
 A rigid body is assigned to each of the objects to decide how objects behave in the world. Right now it only has a bounce response but the class is easily modifiable and all kinds of rigid body responses can be added.
# Ray Collusion
 There is a ray collusion. Right now response is just to delete the hit object but can be easily modified. Users can emit rays by clicking a left mouse button.
# Input System
 There is a classic OpenGL callback input system for the keyboard, mouse, and also joystick.
# Shaders and Shadows
 Right now engine only accepts base color and normal textures. Tangent space is calculated for normal maps and the normal texture effect can be closed by pressing 'N'.
 There is global illumination (ambient, specular, diffuse). Blinn-Phong lighting is implemented and can be closed when pressing 'B'.
 Skybox is implemented using Cubemaps.
 3 Types of light implemented these lights are: Point Light, Spot Light, Directional light
 All types of light have shadow maps. (Spotlight shadow is not working properly right now)

# Scene class
 Scene class is to create unique scenes with different parameters and frame buffers. This is the main function to create a new scene and environment. There is no save functionality right now but planning to add one. Also movement of the objects with gismo UI.

# Rigs and Animtation
 TO_DO
 

This tutorial has been followed for most of the code: https://github.com/michaelg29/yt-tutorials/tree/master/CPP/OpenGL
