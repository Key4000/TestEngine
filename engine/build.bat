REM Build scripts for engine

@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cppFilenames=
FOR /R %%f IN (*.cpp) DO (
   SET cppFilenames=!cppFilenames! %%f
)

echo "Files:" %cppFilenames%

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror

REM -Wall -Werror

SET includeFlags=-Isrc -I%VULKAN_SDK%/Include
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib
SET defines=-D_DEBUG -D_TE_EXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%%..."
clang++ %cppFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%
