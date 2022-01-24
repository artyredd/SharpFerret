vertexShader: assets/shaders/outlineShader.vertexshader

# The path to the fragment shader that should be used for this shader
fragmentShader: assets/shaders/outlineShader.fragmentshader

# whether or not backface culling should be enabled for this shader
enableBackfaceCulling: true

# whether or not this shader should use camera perspective, (GUI elements for example shouldnt)
useCameraPerspective: true

# enable the depth buffer so we can see the outline ontop of the terrain and other geometry
enableDepthTesting: true

useStencilBuffer: true

# dont write to the stencil buffer, this is the cube that is drawn AFTER the original is drawn
writeToStencilBuffer: false

# change how we render fragments
useCustomStencilAttributes: true

# only render fragments if the stencil buffer value is NOT 1(so we only draw a border around the first object)
customStencilFunction: notEqual

# set the value to test if it loads properly
customStencilValue: 1

# set the mask to test if it loads properly
customStencilMask: 0xFF

