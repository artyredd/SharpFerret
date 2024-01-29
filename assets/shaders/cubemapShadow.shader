# The path to the vertex shader that should be used for this shader
vertexShader: assets/shaders/stub_model.vertexshader

# The path to the fragment shader that should be used for this shader
fragmentShader: assets/shaders/linearCubemap.fragmentShader

geometryShader: assets/shaders/cubemap.geometryshader

# whether or not backface culling should be enabled for this shader
culling: front

# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)
useCameraPerspective: true

depthTest: less