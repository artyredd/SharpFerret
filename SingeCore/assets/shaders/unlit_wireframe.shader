# The path to the vertex shader that should be used for this shader
vertexShader: assets/shaders/default.vertexshader

# The path to the fragment shader that should be used for this shader
fragmentShader: assets/shaders/unlit.fragmentshader

# whether or not backface culling should be enabled for this shader
culling: none

# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)
useCameraPerspective: true

depthTest: less

useStencilBuffer: true
writeToStencilBuffer: true
fillMode: line