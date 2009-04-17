NV38Box is a "realtime" raytracer for NV38 or NV40.

Features of RForce method:
- dynamic GPU raytracer running in realtime for simple scenes and low resolutions
- scene = 16 triangular objects per 64 triangles (total up to 1024 triangles)
- using AABB as an acceleration structure
- per pixel shading for both primary and secondary rays
- no dynamic control flow as opposite to Purcell's method
- hybrid algorithm (primary rays = classic rasterization)
- nVidia only, NV38 path, NV40 path, brute force for comparison
- implementation performs only secondary reflection rays, but method is more flexible in general
- tested on WinXP, GeForce 5950 Ultra, ForceWare 62.20

You can execute:
nv38box064.exe - raytracing in resolution  64x64
nv38box128.exe - raytracing in resolution 128x128
nv38box256.exe - raytracing in resolution 256x256
nv38box512.exe - raytracing in resolution 512x512

See source package for more details.

(c) 2004 Antonin Hildebrand
http://inferno.hildebrand.cz