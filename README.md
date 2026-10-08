\# OpenGL Game Engine



A 3D model viewer built with OpenGL, GLFW, ImGui, and Assimp.



\## Features



\- Load 3D models (FBX, OBJ, etc.) with Assimp

\- PBR materials

\- Bloom post-processing

\- ImGui-based UI

\- Camera controls



\## Build



\### Windows (MinGW)



build.bat



\### Run



run.bat



\## Structure



game engine/

&#x20; code/

&#x20;   src/                # Engine source

&#x20; include/              # Headers (glm, glad, assimp, KHR)

&#x20; glfw-3.4.bin.WIN64/   # GLFW library

&#x20; imgui/                # ImGui library

&#x20; tinyfiledialogs/      # File dialogs

&#x20; assets/               # 3D models

&#x20; build.bat

&#x20; run.bat



\## Assets



Only `Valkyrie/` is included in this repository. Other models (dragon, Gwyndolin, marika) are excluded due to size.



\## License



MIT

