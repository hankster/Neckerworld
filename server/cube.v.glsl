//layout(location = 0) in vec3 coord3d;
//layout(location = 1) in vec2 coordtexture;

attribute vec3 coord3d;
attribute vec3 normal;
attribute vec2 coordtexture;
varying vec4 f_position;
varying vec3 f_normal;
varying vec2 f_coordtexture;
uniform mat4 mvp;
uniform mat4 model;

void main(void) {
  gl_Position = mvp * vec4(coord3d, 1.0);
  f_position = model * vec4(coord3d, 1.0);
  f_normal = normalize(mat3(model) * normal);
  //f_normal = mat3(transpose(inverse(model))) * normal;
  f_coordtexture = coordtexture;
}
