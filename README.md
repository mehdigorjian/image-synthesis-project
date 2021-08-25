# image_synthesis
Coputer graphics with C++ and openGL


## Project 01: 2D Diffuse Shading & Specular Highlight with Normal Maps
 
Goal: To learn basics of Diffuse Shading.
Synopsis: The first project is intended for you to start with a simple 2D diffuse shader. You will use simple objects in 2D using normal map to shade such as the ones provided in class Google drive.

Simple Rendering with diffuse reflection by using 2D shapes defined by normal maps. There will be two types of diffuse rendering: (1) mixing two dark and light colors; and (2) mixing dark and light images; (2 tasks)
Simple Rendering with diffuse reflection and specular highlight by using 2D shapes defined by normal maps. There will be three types of shaders: (1) mixing one dark and one light color with specular color; and (2) mixing dark and light images with specular color (Reflective region should be defined by a ks parameter that can be given as either as alpha or a separate image) (2 tasks)

## Project 02: 2D Mirror Surfaces
 
Synopsis: This project is intended for you to extend your shader to include mirror effects. The main goal of this project to make you write a shader with complete control with mirror effects. You will still use normal maps to shade. 

Mirror reflections of environment images in front of a background image using a normal map with alpha.
Combining appropriately diffuse reflection with mirror reflections of environment images along with ks using a normal map with alpha.

## Project 03: 2D Transparent Surfaces
 
Synopsis: This project is intended for you to start with a simple 2D refraction shader. You will still use normal maps as refective objects. They will refract a background image using transmission vector and reflect an environment image. The result will appear to be a transparent or translucent surface. 

Perfect Refraction of background image.
Reflection of environmenmet map and Refraction of background image mixed with Fresnel.
Complex shapes with transparent, mirror and diffuse regions appropriately combined.

## Project 04: 3D Camera & Ray Casting
 
Goal: To construct a controllable camera with simple shading. 
Synopsis: This project is intended for you to create a camera you can easily control and to get familiar with the fundamental concepts behind ray casting. 

Inclusion of objects into your ray caster rendered with constant color (2 tasks and 1 bonus)
Spheres
Planes
Bonus: Quadrics
Controlling your camera parameters (2 tasks)
Demonstration of changing view direction and rotating the camera along its axis.
Demonstration of changing field of view.
Shading objects (you should have more than one type of objects such as spheres and planes) with different shaders (3 tasks and two bonus)
Diffuse shading (dark and bright diffuse colors will come from desired style)
Sharp and soft Specular highlights (specular colors will come from desired style)
Control illumination using directional, point spot lights
Bonus: Rectangular (or other shaped) Specular highlights.
Bonus: Outlines (outline/border colors will come from desired style)

## Project 05: Shadow Casting
 
Goal: To learn to shadow casting. 
Synopsis: This project is intended for you to handle simple shadows in your ray caster. 
You will continue to use a goal image (preferably a color painting) that provide a desired style. Your goal is to obtain desired look-and-feel with your shading and shadow. In this case, choosing shadow colors will be an important task.

Casting Shadows (4 tasks)
Directional and Point lights
Spot light (If you are outside of spot light cone, there will be shadow)
Soft Shadows with area Lights
Bonus: Shadow Map
Bonus: Qualitatvely Consistent Soft Shadows and Subsurface Scatterring with d/r equation that provides cos theta

## Project 06: Texture mapping & Solid Textures
 
Goal: To learn handling textures. 
Synopsis: This project is intended for you to deal with textures. You can use a basic scene with a single sphere, single infinite plane and single environment (infinite) sphere.

Texture mapping images (3 tasks)
Mapping one image to a sphere.
Create a wallpaper on an inifinite plane.
Create an Environment map by mapping an image to the infinite sphere.
Solid Texturing texturing (Choose 1)
Create a solid texture by parallel projecting an image to 3D space.
Create a solid texture by perspective projecting an image to 3D space.
Create a light shader by using parallel projection from directional light.
Create a light shader by using perspective projection from either spot of point light.
Procedural textures (One Bonus)
Solid texturing 3D Julia sets
Solid texturing 3D Perlin Noise
Solid texturing with 3D functions
Mapping 2D Julia Sets
Mapping 2D Perlin Noise
Mapping using 2D functions
Texture effects (One Bonus)
Normal map
Bump mapping

## Project 07: Casting Triangles
 
Goal: Ray casting triangles 
Synopsis: This project is intended for you to deal with more complicated objects. 

Inclusion of Triangular Meshes (3 tasks)
Tetrahedron
Cube
General triangular mesh given by .obj file
Texture mapping triangular mesh (1 task)
Textured triangular mesh whose texture coordinates are given by .obj file

## Project 08: Reflection & Refraction
 
Goal: Specular Reflection & Refraction. 
Synopsis: This project is intended for you to deal with effects such as reflection, refraction and Fresnel. 

Reflection & Refraction (4 tasks)
Specularly reflected implicit shapes
Transparent with Fresnel implicit shapes
Specularly reflected polygonal meshes
Transparent with Fresnel implicit polygonal meshes
Specular reflection with texture applications (2 task - choose 2 out of 4 )
Environment map
Specular reflection and refraction defined by a normal (or bump) map
Iridescence effect
Index of refraction defined by a texture map

## Project 09: Distributed Ray Tracing
 
Goal: Distributed ray tracing. 
Synopsis: This project is intended for you to deal with effects such as caustics, ambient occlusion, color bleeding, subsurface scattering. 

Distributed Ray Tracing (4 tasks)
Motion blur
Out of focus
Translucent effect
Glossy

## Project 10: Final Animation
Submissions	Due: Apr. 26
 
Goal: Semester End Show. 
Synopsis: This project is intended for you to complete semester end show animations. 
Semester-End Show Work (4 tasks)
Animation with moving objects: This is a standard ray tracing animation. You can use any of your resulting scenes to turn them into animation. Feel free submit more than on if you have several interesting pieces. You can get one bonus creadit if you submit several high quality animations. 

Animation of parameters such as Index of refraction animation. 

Animation with Camera Painting: In this case, you will use an image (in fact a series of images) to alter 3D positions of points on viewport. This will deform your viewport to result in a "cubist" looking animation. If you do not remember instructions given in the class, see this paper to fin out how to obtain camera painted animations. 

Stereo Graphics Ray Tracing. You can simply put two images in your class website to get the credit. If you want your stereo graphics is shown in VizaGoGo, ask VizaGoGo committee for specifications.
