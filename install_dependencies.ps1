$glmUrl = "http://kent.dl.sourceforge.net/project/ogl-math/glm-0.9.6.1/glm-0.9.6.1.zip";
$sdl2Url = "https://www.libsdl.org/release/SDL2-devel-2.0.3-VC.zip";

# Download glm
Write-Output "Downloading glm...";
Invoke-WebRequest -Uri $glmUrl -OutFile glm.zip;
7z x -o".\glm" .\glm.zip;
Copy-Item -Recurse .\glm\glm .\lib\glm;
Remove-Item -Recurse .\glm;
Remove-Item .\glm.zip;

# Download SDL2
Write-Output "Downloading SDL2...";
Invoke-WebRequest -Uri $sdl2Url -OutFile sdl2.zip;
7z x -o".\sdl2" .\sdl2.zip;
Copy-Item -Recurse ".\sdl2\SDL2-2.0.3" .\lib\sdl;
Remove-Item -Recurse .\sdl2;
Remove-Item .\sdl2.zip;