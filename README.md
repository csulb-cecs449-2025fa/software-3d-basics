# Basics of software 3D rendering

A series of C++ projects using CMake, vcpkg, SFML, and Assimp to demonstrate the basics of 
3D rendering in software. Each project builds on the previous, following lecture notes
[posted here](https://docs.google.com/presentation/d/1cRCjv20EMTsZz1DQWvrGNWEyLhBNDEoOg1Kymv-V5i0/edit?usp=sharing).

## Static2D

Introduces the Vertex structure, and representing a "mesh" as a list
of vertices and a list of indexes used to form triangles in screen space.

## ClipCoordinates

Introduces normalized device coordinates (aka clip space coordinates), 
and transforms meshes from clip space to screen space.

## Vertex3D 

Introduces a third dimension for vertices, but without understanding what exactly
we expect to happen with this information.

## ViewCoordinates

Mesh vertices are now expressed in view space, relative to a camera at 
(0, 0, 0) looking down the negative Z axis. Transforms mesh vertices from view
space to clip space to screen space.

## WorldSpace

Assigns a position, orientation, and scale in "world space" to each "object" in the scene; objects
can share the same mesh structure. Transforms meshes from local space to world space (view space for now)
to clip space to screen space.

## Assimp

Uses the Assimp library to load the Stanford Bunny, plugging its vertices
and faces into the rest of the rendering engine.