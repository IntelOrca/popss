Write-Output "Building debug:x86"
msbuild .\projects\popss.vcxproj /p:Configuration=Debug /p:Platform=x86 /verbosity:quiet /nologo
Write-Output "Building debug:x64"
msbuild .\projects\popss.vcxproj /p:Configuration=Debug /p:Platform=x64 /verbosity:quiet /nologo
Write-Output "Building release:x86"
msbuild .\projects\popss.vcxproj /p:Configuration=Release /p:Platform=x86 /verbosity:quiet /nologo
Write-Output "Building release:x64"
msbuild .\projects\popss.vcxproj /p:Configuration=Release /p:Platform=x64 /verbosity:quiet /nologo
