rem utf-8
chcp 65001
chcp

%VK_SDK_PATH%\Bin32\glslc.exe shader.vert -o vert.spv
%VK_SDK_PATH%\Bin32\glslc.exe shader.frag -o frag.spv
pause
