# COMP3015-CW2 - Deep Space Nova
An OpenGL project, with the focus on GLSL implementation of at least 3 basic
shaders techniques or an indepth technical implementation of one shader technique. <br> 

In this project, there are several different shading views to select, each changing 
the scene to a new shader or set of shaders. Each view uses the same objects, a UFO spaceship, 
a few meteors, and a teapot. The 'Normal' view uses the blinn-phong model to shade the objects, 
using a point and spot light, and a normal map for the UFO. The 'Gaussian' view applies the 
gaussian function to a render texture of the scene, making the final image blurry. The 
'Silhouette' view uses a geometry shader and makes the objects 'toon' shaded whilst 
also hightlighting the object edges and applying lines to them. Finally, the 'Night Vision' 
view generates a noise texture which is applied over a render texture of the scene, making 
it seem like you are seeing the scene through night vision goggles.

### How to navigate the code.
The bulk of the logic pertaining to the shading techniques is done within the 
scenebasic_uniform.cpp source file. There are instances the 'Gauss' and 'NightVision' 
classes which are created and used in the game loop, these were used to further abstract 
the codebase, and make it more manageable. The shading techniques are handled in the 
render function, which is executed every frame. Several boolean values, which change 
on a button press, decide which technique is used. The init function initiallises 
a lot of the various buffer objects and such that need to be used.

### Running the program.
To view the scene, download the .zip file from the released tab, extract the foler, 
then run the .exe file name 'COMP3015-CW2' to start the program. Or you can download 
the source code to inspect the project further. You will need all the OpenGL 
dependancies installed and linked to the project. These include: glfw, glad, and glm.

![image](https://user-images.githubusercontent.com/55700734/168471860-9219d64f-abf5-4345-8633-5162d0f75487.png)
![image](https://user-images.githubusercontent.com/55700734/168471914-777f4e62-449c-438c-8b01-2cb4de4a3537.png)
![image](https://user-images.githubusercontent.com/55700734/168471946-6ec9d5e2-db32-4d74-a534-9eeb834bb86c.png)
![image](https://user-images.githubusercontent.com/55700734/168471958-8503104e-51b9-4b43-993d-10111ef0aee9.png)
![image](https://user-images.githubusercontent.com/55700734/168471976-63e7a8ec-28e3-4107-aa6c-4b93f77384b3.png)
