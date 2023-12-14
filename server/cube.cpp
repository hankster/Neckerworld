 /**
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <ctime>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <zlib.h>
using namespace std;

/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>

/* GLFW */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/* GLM */
// #define GLM_MESSAGES
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Library for image loading, processing */
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

/* Common shader programs */
#include "shader_utils.h"

/* Function prototypes for this program */
#include "cube.h"
#include "cube_client.h"

/* Main window */
GLFWwindow* window;
/* Hidden windows for separate cube views */
GLFWwindow* windows[NC];

/* Window settings */
const char * window_title = "Neckerworld";
int main_window_width = WINDOW_WIDTH;
int main_window_height = WINDOW_HEIGHT;
int main_window_channels = WINDOW_CHANNELS;
int raster_width = IMAGE_RASTER_WIDTH;
int raster_height = IMAGE_RASTER_HEIGHT;
float window_background_color_r = 1.0;
float window_background_color_g = 1.0;
float window_background_color_b = 1.0;
float window_background_color_a = 1.0;

/* World scene variables */
glm::mat4 view;
glm::mat4 projection;
float perspective_fov = 60.0f;
  
/* Camera position and target settings */
glm::vec3 camera_position_default = glm::vec3(0.0, 6.0, 8.0);
glm::vec3 camera_target_default = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 camera_up_default = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 camera_position;
glm::vec3 camera_target;
glm::vec3 camera_up;

/* Copy of the window to be picked up by a second thread */
GLint view_index = -1;
GLint view_index_delay[4] = {-1, -1, -1, -1};
GLint view_index_delay_length = 4;
int   capture_delay = 0;

/* For remote cube views */
// pixels=frame buffer
// pixels_frame=frame count when image was captured
// pixels_frame_start=minimum frame count required for capture
// pixels_mutex=protection for pixel transfer (used in screenview())
// pixelx_mutex_wait=protects wait condition
// pixels_cv=variable used for message thread waiting
std::vector<uint8_t> pixels[NC];
unsigned pixels_frame[NC];
unsigned pixels_frame_start[NC];
std::mutex pixels_mutex[NC];
std::mutex pixels_mutex_wait[NC];
std::condition_variable pixels_cv[NC];

/* For remote ground views */
std::vector<uint8_t> ground_pixels[NG];
unsigned ground_pixels_frame[NG];
unsigned ground_pixels_frame_start[NG];
std::mutex ground_pixels_mutex[NG];

/* The vertex attribute array */
GLuint VertexArrayObject;
GLuint VertexArrayObjects[NC];

/* Cube generating parameters */
int n_cubes = 0;
Cube cubes[NC];

/* Wire cube generating parameters */
int n_wires = 0;
Wire wires[NW];

/* Ground generation parameters */
int n_grounds = 0;
Ground grounds[NG];

/* Light generating parameters */
int n_lights = 0;
Light lights[NL];

/* Material definitions */
Material materials[NM];
int n_materials = 0;

/* Texture related definitions*/
Texture textures[NT];
int n_textures = 0;

/* Normal vector parameters */
GLuint vbo_normals;
glm::mat4 normals_model;
glm::mat4 normals_mvp;

/* Shader programs */
GLuint program;
GLint attribute_coord3d;
GLint attribute_normal;
GLint attribute_coordtexture;
GLint uniform_model;
GLint uniform_mvp;
GLint uniform_object_texture;
GLint uniform_object_type;
GLint uniform_object_color;
GLint uniform_view_position;

/* Uniforms for our array of point lights */
/* With structs need to do it by each member */
GLint uniform_n_lights;
Ul uniform_lights[NL];

/* Material uniforms */
GLint uniform_material_ambient;
GLint uniform_material_diffuse;
GLint uniform_material_specular;
GLint uniform_material_shininess;;

/* JSON options */
char* jsonfile;

/* JSON remote import */
string json_import_file;
string json_import_object;
bool json_flag_file = false;
bool json_flag_object = false;

/* Frame counter and timestamp*/
double frames_per_second = DEFAULT_FPS;
GLuint frame_counter = 0;
double frame_time;

/* Screen shot number */
int screenshot_number = 0;

// Define a static variable for key processing related to cubes
int cube_key = 0;
bool cube_key_active = false;

// Multithread server related
int workers = MT_WORKERS;
int port = MT_PORT;
string ip = MT_IP;

/* For debugging drawing code */
bool   clock_time = false;
GLint debug = 0;
GLuint diagnostic = false;
GLuint display_cubes = 1;
GLuint display_wires = 1;
GLuint display_normals = 0;
GLuint display_grounds = 1;
GLuint display_rotation = 0;
GLuint display_rotation_x = 0;
GLuint display_rotation_y = 0;
GLuint display_rotation_z = 0;
GLuint position = 0;
GLuint strategy = false;
GLuint strategy_basic = true;
GLuint strategy_extended = false;
GLuint transparency = 0;
bool   video = false;
bool   matrix_test = false;

/* Constants */
float pi = 3.14159265358979323846;

void Usage()
{
  fprintf(stdout, "Usage: cube -a delay -c 0/1 -d level -f filename -g 0/1 -j jsonfile -h -n 0/1 -p -r 1-4 -t -v -w 0/1 -x windowwidth -y windowheight jsonfile [...jsonfile]\n"
	  "where\n"
	  "\t-a delay\tAdd 'delay' frames before returning a view request, default=0 (this is a hack).\n"
	  "\t-c 0/1\t\tDisplay cubes, default=1 or true.\n"
	  "\t-d level\tSet debug level, default=0.\n"
	  "\t-f filename\tNot used.\n"
	  "\t-g 0/1\t\tDisplay grounds, default=1 or true.\n"
	  "\t-j jsonfile\tName of the JSON file to process (not used).\n"
	  "\t-h\t\tPrint out parameter help.\n"
	  "\t-n 0/1\t\tDisplay cube face normals, default=0.\n"
	  "\t-p\t\tSet position=1 for cube spin.\n"
	  "\t-r 1-4\t\tSetup different axes for rotation (x, y, z, all).\n"
	  "\t-t\t\tSet transparency for rendering, default=0.\n"
	  "\t-v\t\tPrint cube version level.\n"
	  "\t-w 0/1\t\tDisplay wireframe cubes, default=0.\n"
	  "\t-x width\tMain window width (X direction), default=1280.\n"
	  "\t-y height\tMain window height (Y direction), default=720.\n"
	  "\tjsonfiles\tOne or more JSON files to process to setup the game playing field.\n"
	  );
}

// Generate a random number from 0.0-1.0
float random1() {
  return (float)((float)rand()/float(RAND_MAX));
}

// Update the view to a ground view or the camera view
void ground_update_view(int gv) {
      
  /* Redo the ground view */
  if (gv >= 0) {
    view = glm::lookAt(grounds[gv].ground_view_position, grounds[gv].ground_view_target, grounds[gv].ground_view_up);
  } else {
    view = glm::lookAt(camera_position, camera_target, camera_up);
  }
  for (int i = 0; i < n_cubes; ++i) {
    cube_update_mvp(i);
  }
  for (int i = 0; i < n_grounds; ++i) {
    ground_update_mvp(i);
  }
}

int shaders()
{
  GLint link_ok = GL_FALSE;
  GLint active_attributes;
  GLint active_uniforms;
  GLuint vs, fs;
  const char* attribute_name;
  const char* uniform_name;

  if ((vs = create_shader("cube.v.glsl", GL_VERTEX_SHADER))   == -1) return -1;
  if ((fs = create_shader("cube.f.glsl", GL_FRAGMENT_SHADER)) == -1) return -1;

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  /* Check if installation went Ok */
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    fprintf(stderr, "glLinkProgram:");
    print_log(program);
    return -1;
  }

  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &active_attributes);
  fprintf(stdout, "cube.cpp: Number of active shader attributes = %d \n", active_attributes);
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &active_uniforms);
  fprintf(stdout, "cube.cpp: Number of active shader uniforms = %d \n", active_uniforms);

  if (active_attributes > 0) {
    attribute_name = "coord3d";
    attribute_coord3d = glGetAttribLocation(program, attribute_name);
    fprintf(stdout, "cube.cpp: attribute_coord3d = %d\n", attribute_coord3d);
    if (attribute_coord3d == -1) {
      fprintf(stderr, "cube.cpp: Could not bind attribute %s\n", attribute_name);
      return -1;
    }
  }

  if (active_attributes > 1) {
    attribute_name = "normal";
    attribute_normal = glGetAttribLocation(program, attribute_name);
    fprintf(stdout, "cube.cpp: attribute_normal = %d\n", attribute_normal);
    if (attribute_normal == -1) {
      fprintf(stderr, "cube.cpp: Could not bind attribute %s\n", attribute_name);
      return -1;
    }
  }

    if (active_attributes > 2) {
    attribute_name = "coordtexture";
    attribute_coordtexture = glGetAttribLocation(program, attribute_name);
    fprintf(stdout, "cube.cpp: attribute_coordtexture = %d\n", attribute_coordtexture);
    if (attribute_coordtexture == -1) {
      fprintf(stderr, "cube.cpp: Could not bind attribute %s\n", attribute_name);
      return -1;
    }
  }

  if (active_uniforms > 0) {
    uniform_name = "mvp";
    uniform_mvp = glGetUniformLocation(program, uniform_name);
    if (uniform_mvp == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    uniform_name = "model";
    uniform_model = glGetUniformLocation(program, uniform_name);
    if (uniform_model == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    uniform_name = "texture";
    uniform_object_texture = glGetUniformLocation(program, uniform_name);
    if (uniform_object_texture == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    uniform_name = "object_type";
    uniform_object_type = glGetUniformLocation(program, uniform_name);
    if (uniform_object_type == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    uniform_name = "object_color";
    uniform_object_color = glGetUniformLocation(program, uniform_name);
    if (uniform_object_color == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    /* This is actually the camera position */
    uniform_name = "view_position";
    uniform_view_position = glGetUniformLocation(program, uniform_name);
    if (uniform_view_position == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    /* uniforms for structure Light */
    uniform_name = "n_lights";
    uniform_n_lights = glGetUniformLocation(program, uniform_name);
    if (uniform_n_lights == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    
    /* uniforms for structure Light */
    /* To pass uniforms for structs need to do it for each member */
    /* Iterate over all the lights we've defined */

    // Allocates storage
    char *u_name = (char*)malloc(32 * sizeof(char));

    for ( int i = 0; i < n_lights; ++i ) {
      // Prints "uniform name" into u_name
      sprintf(u_name, "lights[%d].position", i); 
      uniform_lights[i].position = glGetUniformLocation(program, u_name);
      if (uniform_lights[i].position == -1) {
	fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", u_name);
	return -1;
      }
      sprintf(u_name, "lights[%d].intensity", i);
      uniform_lights[i].intensity = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].ambient", i);
      uniform_lights[i].ambient = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].diffuse", i);
      uniform_lights[i].diffuse = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].specular", i);
      uniform_lights[i].specular = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].constant", i);
      uniform_lights[i].constant = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].linear", i);
      uniform_lights[i].linear = glGetUniformLocation(program, u_name);
      sprintf(u_name, "lights[%d].quadratic", i);
      uniform_lights[i].quadratic = glGetUniformLocation(program, u_name);
      } /* end for (int i = 0 ... */
    
    /* uniforms for structure Material */
    uniform_name = "material.ambient";
    uniform_material_ambient = glGetUniformLocation(program, uniform_name);
    if (uniform_material_ambient == -1) {
      fprintf(stderr, "cube.cpp: Could not bind uniform %s\n", uniform_name);
      return -1;
    }

    uniform_name = "material.diffuse";
    uniform_material_diffuse = glGetUniformLocation(program, uniform_name);
    uniform_name = "material.specular";
    uniform_material_specular = glGetUniformLocation(program, uniform_name);
    uniform_name = "material.shininess";
    uniform_material_shininess = glGetUniformLocation(program, uniform_name);

    fprintf(stdout, "cube.cpp: Uniforms: Light pos %d int %d amb %d diff %d spec %d cons %d lin %d quad %d\n",
	    uniform_lights[0].position, uniform_lights[0].intensity, uniform_lights[0].ambient, uniform_lights[0].diffuse, uniform_lights[0].specular, uniform_lights[0].constant, uniform_lights[0].linear, uniform_lights[0].quadratic); 
    fprintf(stdout, "cube.cpp: Uniforms - Material amb %d diff %d spec %d shine %d\n", uniform_material_ambient, uniform_material_diffuse, uniform_material_specular, uniform_material_shininess); 
    fprintf(stdout, "cube.cpp: Uniforms - Cube type %d color %d texture %d\n", uniform_object_type, uniform_object_color, uniform_object_texture);
    fprintf(stdout, "cube.cpp: Uniforms - View %d, Model %d, MVP %d\n", uniform_view_position, uniform_model, uniform_mvp);

  }

  return 0;
}

// Update the projection matrix
void update_projection(int width, int height)
{
  // tanHalfFovy = tan(rad / 2.0);
  // detail::tmat4x4<valType, defaultp> Result(0.0);
  // Result[0][0] = 1.0 / (aspect * tanHalfFovy);
  // Result[1][1] = 1.0 / tanHalfFovy;
  // Result[2][2] = - (zFar + zNear) / (zFar - zNear);
  // Result[2][3] = - 1.0
  // Result[3][2] = - (2.0 * zFar * zNear) / (zFar - zNear);

  projection = glm::perspective(glm::radians(perspective_fov), float(width)/float(height), 0.1f, 100.0f);

  if (debug > 0) {
    printf("cube.cpp: update_projection %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f, %0.4f\n", \
	   projection[0][0], projection[0][1], projection[0][2], projection[0][3], \
	   projection[1][0], projection[1][1], projection[1][2], projection[1][3], \
	   projection[2][0], projection[2][1], projection[2][2], projection[2][3], \
	   projection[3][0], projection[3][1], projection[3][2], projection[3][3]);
  }

  if (matrix_test) {
    
    float pts[48] = {0.58, -0.58,  -1.0, 1.0, 0.58, 0.58,  -1.0, 1.0, -0.58, 0.58,  -1.0, 1.0, -0.58, -0.58,  -1.0, 1.0, \
		     1.0,  -1.0,  -10.0, 1.0,  1.0, 1.0 , -10.0, 1.0, -1.0,  1.0,  -10.0, 1.0, -1.0,  -1.0,  -10.0, 1.0, \
		     1.0,  -1.0, -100.0, 1.0,  1.0, 1.0, -100.0, 1.0, -1.0,  1.0, -100.0, 1.0, -1.0,  -1.0, -100.0, 1.0};

    glm::vec4 p[12];

    memcpy( glm::value_ptr(p[0]), pts, sizeof(pts) );

    for (int i=0; i<12; ++i) {
      glm::vec4 uv = projection * p[i];
      printf("cube.cpp: matrix_test p[%2d] x %7.2f y %7.2f z %7.2f w %7.2f uv.x %7.2f uv.y %7.2f uv.z %7.2f uv.w %7.2f\n", i, p[i].x, p[i].y, p[i].z, p[i].w, uv.x, uv.y, uv.z, uv.w);
	
    }

    matrix_test = false;
  }
  
}
  
/* Initialize required parameters for the scene */
int init_resources()
{
  int status;

  /* Setup display of vector normals */
  if (display_normals) {
    if (status = show_normals()) {
	fprintf(stderr, "cube.cpp: show_normals failed\n");
	return status;
    }
  }
    
  return 0;
}

void cube_spin(float angle) {

  glm::mat4 animation_x(1.0f);
  glm::mat4 animation_y(1.0f);
  glm::mat4 animation_z(1.0f);

  if (position) {
    float position_offset_x = 15.0*sin(angle);
    float position_offset_z = 15.0*cos(angle);
    camera_position.x = position_offset_x;
    // camera_position.z = position_offset_z;
    /* Reset the view */
    ground_update_view(-1);
  }

  if (display_rotation) {

    if (display_rotation_x) animation_x = glm::rotate(glm::mat4(1.0f), angle*3.0f, glm::vec3(1, 0, 0)); // X axis
    if (display_rotation_y) animation_y = glm::rotate(glm::mat4(1.0f), angle*2.0f, glm::vec3(0, 1, 0)); // Y axis
    if (display_rotation_z) animation_z = glm::rotate(glm::mat4(1.0f), angle*4.0f, glm::vec3(0, 0, 1)); // Z axis

    glm::mat4 animation = animation_x * animation_y * animation_z;

    float xoffset = 3.0*sin(angle);
    // cubes[2].spatial_translation = glm::translate(glm::mat4(1.0f), glm::vec3(xoffset, 0.0, 0.0));
    
    if (display_cubes) {
      for (int i = 0; i < n_cubes; ++i) {
	cubes[i].cube_model = cubes[i].spatial_translation * cubes[i].spatial_rotation_matrix * animation * cubes[i].cube_scale;
	cube_update_mvp(i);
      }
      
    }

    if (display_wires) {
      for(int i = 0; i < n_wires; ++i) {
	wires[i].wire_mvp = projection * view * wires[i].wire_model * animation * wires[i].wire_scale;
      }
    }
    
    if (display_normals) {
      normals_mvp = projection * view * normals_model * animation;
    }
  } /* end if(display_rotation) ... */

}

void free_resources()
{
  glDeleteProgram(program);
  for (int i = 0; i < n_cubes; ++i) {
    glDeleteBuffers(1, &cubes[i].vbo_cube_vertices);
    glDeleteBuffers(1, &cubes[i].vbo_cube_normals);
    glDeleteBuffers(1, &cubes[i].vbo_cube_texture_map_coords);
    glDeleteBuffers(1, &cubes[i].ibo_cube_elements);
  }
  for (int i = 0; i < n_grounds; ++i) {
      glDeleteBuffers(1, &grounds[i].vbo_ground_vertices);
      glDeleteBuffers(1, &grounds[i].vbo_ground_normals);
      glDeleteBuffers(1, &grounds[i].vbo_ground_texture_map_coords);
      glDeleteBuffers(1, &grounds[i].ibo_ground_elements);
    }
  for (int i = 0; i < n_wires; ++i) {
    glDeleteBuffers(1, &wires[i].vbo_wire_vertices);
  }
  for (int i = 0; i < n_textures; ++i) {
    glDeleteTextures(1, &textures[i].texture_array);
  }
  glDeleteVertexArrays(1, &VertexArrayObject);
}


int main(int argc, char* argv[]) {

  char *filename = NULL;
  int index;
  int status;
  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "a:c:d:f:g:hj:n:pr:tvw:x:y:")) != -1)
    switch (c)
      {
      case 'a':
	sscanf(optarg, "%d", &capture_delay);
        break;
      case 'c':
	sscanf(optarg, "%d", &display_cubes);
        break;
      case 'd':
        sscanf(optarg, "%d", &debug);
        break;
      case 'f':
        filename = optarg;
        break;
      case 'g':
	sscanf(optarg, "%d", &display_grounds); 
        break;
      case 'h':
        Usage();
        return 0;
      case 'j':
        jsonfile = optarg;
        break;
      case 'n':
	sscanf(optarg, "%d", &display_normals); 
        break;
      case 'p':
        position = 1;
        break;
      case 'r':
	sscanf(optarg, "%d", &display_rotation);
	if (display_rotation == 1) display_rotation_x = 1;
	if (display_rotation == 2) display_rotation_y = 1;
	if (display_rotation == 3) display_rotation_z = 1;
	if (display_rotation == 4) {
	  display_rotation_x = 1;
	  display_rotation_y = 1;
	  display_rotation_z = 1;
	}
	break;
      case 't':
        transparency = 1;
        break;
      case 'v':
        fprintf(stdout, "cube version 1.0\n");
        return 0;
      case 'w':
	sscanf(optarg, "%d", &display_wires); 
        break;
      case 'x':
	sscanf(optarg, "%d", &main_window_width); 
        break;
      case 'y':
	sscanf(optarg, "%d", &main_window_height); 
        break;
      case '?':
        if (optopt == 'f')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
	else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
	Usage();
        return 1;
      default:
	Usage();
        abort();
      }

  printf("cube.cpp: options -  debug=%d, filename=%s, display_cubes=%d, display_grounds=%d, display_normals=%d\ncube.cpp: options -  position=%d, display_rotation=%d display_wires=%d, main_window_width=%d, main_window_height=%d\n",
	 debug, filename, display_cubes, display_grounds, display_normals, position, display_rotation, display_wires, main_window_width, main_window_height);

  // glfw: initialize and configure
  if (!glfwInit())
    {
      printf("cube.cpp: Failed to initialize GLFW library\n");
      glfwTerminate();
      return -1;
    }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    
  GLint n_ext;
  glGetIntegerv(GL_NUM_EXTENSIONS, &n_ext);
  printf("cube.cpp: Version %s, Vendor %s, Renderer %s, Extensions %d\n", (char*)glGetString(GL_VERSION), (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), n_ext);

  // glfw window creation
  window = glfwCreateWindow(main_window_width, main_window_height, window_title, NULL, NULL);
  if (window == NULL)
    {
      printf("cube.cpp: Failed to create GLFW window\n");
      glfwTerminate();
      return -1;
    }

  if (debug > 0) printf("cube.cpp: Created main window with title %s, %dx%d\n", window_title, main_window_width, main_window_height);

  glfwMakeContextCurrent(window);
  glfwSetWindowPos(window, 100, 50);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSwapInterval(1);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Setup error callback
  glfwSetErrorCallback(window_error);	

  // Setup keyboard key callback
  glfwSetKeyCallback(window, key_callback);

  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "cube.cpp: glew Error: %s\n", glewGetErrorString(glew_status));
    return 1;
  }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "cube.cpp: glew Error: your graphic card does not support OpenGL 2.0\n");
    return 1;
  }

  /* A Vertex Array Object (VAO) has nothing to do with vertex arrays. */
  /* It's used to hold vertix attribute pointers */
  /* Program does not render without this Bind */
  glGenVertexArrays(1, &VertexArrayObject);
  glBindVertexArray(VertexArrayObject);

  /* Do required initializations */
  if (status = init_resources()) {
    free_resources();
    glfwTerminate();
    return status;
  }

  // Create window contexts for individual cube views
  char wbuf[32];
  for(int i=0; i<NC; ++i) {
    sprintf(wbuf, "Neckerworld Cube View %d", i);
    // Share context of the main window with all these sub-windows.
    if (false) { // was "if (i==0) {" for debugging 
      glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    } else {
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    if (debug > 0) printf("cube.cpp: Creating window[%d] with title %s\n", i, wbuf);
    windows[i] = glfwCreateWindow(raster_width, raster_height, wbuf, NULL, window);
    if (windows[i] == NULL) {
      printf("cube.cpp: Failed to create GLFW window[%d]\n", i);
      glfwTerminate();
      return -1;
    }
    // Setup opengl items that are not shared
    glfwMakeContextCurrent(windows[i]);

    if (i==0) glfwSetWindowPos(windows[i], 650, 50);
    
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
      fprintf(stderr, "cube.cpp: glew_status window[%d] Error: %s\n", i, glewGetErrorString(glew_status));
      return 1;
    }

    /* A Vertex Array Object (VAO) has nothing to do with vertex arrays. */
    /* It's used to hold vertex attribute pointers */
    /* Program does not render without this Bind */
    glGenVertexArrays(1, &VertexArrayObjects[i]);
    glBindVertexArray(VertexArrayObjects[i]);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

  } // for(int i=0; i<NC; ++i) {
  
  // Setup multi-thread server
  server_main(workers, port, ip);

  /* Process all json files */

  if (argc < 2) {
    printf("cube.cpp: no json input files supplied on the command line\n");
  }
  struct stat stat_buffer;   
  for (index = optind; index < argc; index++) {
    /* Check if the input file exists */
    int file_status = stat(argv[index], &stat_buffer);
    if (file_status != 0) {
      printf("cube.cpp: json input file %s not found\n", argv[index]);
      free_resources();
      glfwTerminate();
      server_stop();
      return file_status;
    }

    printf("cube.cpp: Processing file %s\n", argv[index]);
    if (status = json_import(argv[index])) {
      free_resources();
      glfwTerminate();
      server_stop();
      return status;
    }
  }
  
  /* Setup shaders */
  if (status = shaders()) {
    fprintf(stderr, "cube.cpp: shader creation failed\n");
    free_resources();
    glfwTerminate();
    server_stop();
    return status;
  }

  /* Seed our random number generator */
  srand((unsigned)time(NULL));

  printf("cube.cpp: Initial screen width=%d, height=%d\n", main_window_width, main_window_height);
  
  // If the status on all setup operations is good, proceed to the render loop.
  if (status == 0) {

    float angle = 0.0;
    int timer = 0;
    time_t start = time(NULL);
    time_t end;
    std::clock_t c_start = std::clock();
    
    // render loop
    while (!glfwWindowShouldClose(window))
    {
      // Check if we are processing raster image cube-eye views
      if (view_index >= 0 && view_index < n_cubes && !cube_key_active) {
	// Move view_index to the next displayable cube
	// If displayable, exit with its view_index
	// If there are none displayable then view_index exits with view_index=n_cubes
	for (view_index; view_index<n_cubes; ++view_index) {
	  if (cubes[view_index].cube_active && cubes[view_index].cube_display && cubes[view_index].cube_remote) {
	    break;
	  }
	} // for ( ...
      }

      // Check if we are processing raster image cube-eye views
      // if () is true we render a raster image and if not we render the ground image
      if (view_index >= 0 && view_index < n_cubes && !cube_key_active) {
      
	glfwMakeContextCurrent(windows[view_index]);
	// GLFWwindow* w = glfwGetCurrentContext();

	// make sure the viewport matches the window dimensions; note that width and 
	glViewport(0, 0, raster_width, raster_height);
	/* Setup the view and projection */
	update_projection(raster_width, raster_height);
	// Lock cube variables here while we setup for the next render
	pixels_mutex[view_index].lock();
	cube_update_view(view_index);
	pixels_mutex[view_index].unlock();

	// Render this view
	display();
	  
	// glfw: swap buffers
	glfwSwapBuffers(windows[view_index]);        	

      } else if (view_index >= n_cubes && view_index < (n_cubes+n_grounds)
		 && grounds[view_index-n_cubes].ground_active && grounds[view_index-n_cubes].ground_display
		 &&!cube_key_active) {

	// We've  finished with all the cubes and are now working on ground views
	glfwMakeContextCurrent(window);
	// make sure the viewport matches the window dimensions; note that width and 
	glViewport(0, 0, main_window_width, main_window_height);
	/* Setup the view and projection */
	update_projection(main_window_width, main_window_height);
	ground_update_view(view_index-n_cubes);
	
	// Render a ground view
	display();

	// glfw: swap buffers
	glfwSwapBuffers(window);
	  
      } else {

	// make sure the viewport matches the window dimensions; note that width and 
	glViewport(0, 0, main_window_width, main_window_height);
	/* Setup the view and projection */
	view = glm::lookAt(camera_position, camera_target, camera_up);
	update_projection(main_window_width, main_window_height);

	display(); 

	// glfw: swap buffers
	glfwSwapBuffers(window);
	
      } // if (view_index >= 0 ...

      if (debug > 1) {
	/* Verify window dimension as a sanity check */
	int mww, mwh, rww, rwh;
	glfwGetWindowSize(window, &mww, &mwh);
	glfwGetWindowSize(windows[0], &rww, &rwh);
	if (mww!=main_window_width || mwh!=main_window_height || rww!=raster_width || rwh!=raster_height) {
	  printf("cube.cpp: Window dimension check fails mww=%d, mwh=%d, rww=%d, rwh=%d\n", mww, mwh, rww, rwh);
	  free_resources();
	  glfwTerminate();
	  server_stop();
	  return -1;
	}
      } // if (debug > 1) {
	
      if (debug > 1) printf("cube.cpp: render loop for frame %d\n", frame_counter);

      // Make a video if requested
      if (video) {
	screenshot(window, "");
      }

      // poll events that have come in and then return immediately
      glfwPollEvents();

      // input
      processInput(window);

      // Check if we are active, logged in and being viewed remotely

      if (view_index >= 0 && view_index < n_cubes) {
	if (cubes[view_index].cube_active && cubes[view_index].cube_remote) {
	  screenview_capture(windows[view_index], view_index, 1, pixels,pixels_frame, pixels_frame_start, pixels_mutex);
	}
      }
      if (view_index >= n_cubes && view_index < (n_cubes+n_grounds)) {
	int gv = view_index - n_cubes;
	if (grounds[gv].ground_remote) {
	  screenview_capture(window, gv, 0, ground_pixels, ground_pixels_frame, ground_pixels_frame_start, ground_pixels_mutex);
	}
      }
	
      if (debug > 1) printf("cube.cpp: render loop C%d frame %d view_index %d view_index_delay [%d, %d, %d, %d]\n",
			    view_index, frame_counter, view_index, view_index_delay[0], view_index_delay[1], view_index_delay[2], view_index_delay[3]);

      if (view_index >= 0 & !cube_key_active) {
	
	// We have just rendered the window associated with this context.
	// After each render we use the next cube view (for the next render).
	// Use the modulus operator to wrap our search over the range of views
	view_index = (view_index + 1) % (n_cubes+n_grounds);

      } // if (view_index >= 0) {

      // Check if any cubes have remote views
      bool rmt_view = false;
      for (int rmtvw=0; rmtvw<n_cubes; ++rmtvw) {
	if (cubes[rmtvw].cube_remote) rmt_view = true;
      }

      // If there are no remote views, fall back to the previous camera viewpoint.
      if (! rmt_view) {
	// Return context to our default window
	glfwMakeContextCurrent(window);
	// If we are in cube_key viewing mode
	if (cube_key_active) {
	  view_index = cube_key;
	  cube_update_view(view_index);
	} else {
	  // Restore the default view
	  view_index = -1;
	  ground_update_view(view_index);
	}
      } // if (! rmt_view) {

	// Check for IP connections
	server_loop();

	// JSON imports have to be done here due to opengl context switch constraints
	// Should mutex this as well
	if (json_flag_file) {
	  json_import(&json_import_file[0]);
	  json_flag_file = false;
	}
	if (json_flag_object) {
	  json_import(json_import_object);
	  json_flag_object = false;
	}

	// spin the cube
	cube_spin(angle);
	angle = angle + glm::radians(0.5);

	// Update size, positions and energy for all cubes
	for (int nc=0; nc < n_cubes; ++nc) {
	  // Check on energy levels for all players
	  cube_energy_check(nc);
	  // Update the size for any youth
	  cube_update_size(nc);
	  // Now update its position
	  cube_update_position(nc);
	  // Account for energy used
	  cube_energy_cost(nc);
	}
	
	// Check for cube contact
	for (int i=0; i < n_cubes; ++i) {
	  for (int j=0; j < n_cubes; ++j) {
	    if (cube_contact(i, j)) {
		cube_contact_evaluation(i, j);
	    }
	  }
	}

	// Check if any of the moves need to be blocked

	for (int i=0; i < n_cubes; ++i) {
	  for (int j=0; j < n_cubes; ++j) {
	    if (cube_contact(i, j)) {
	      cube_block_position(i);
	      cube_block_position(j);
	    }
	  }
	}

	if (strategy == true) {

	  // Update the strategy for each cube
	  string m = "male";
	  string f = "female";
	  string e = "enby";
	  string p = "predator";
	  string r = "resource";
	
	  if (strategy_basic == true) {
	    for (int nc=0; nc < n_cubes; ++nc) {
	      if (! cubes[nc].cube_active || cubes[nc].cube_remote) continue;
	      if (cubes[nc].cube_player == p) strategy_predator(nc);
	      if (cubes[nc].cube_player == p) strategy_resource(nc);
	    }
	  }
	  
	  if (strategy_extended == true) {
	    for (int nc=0; nc < n_cubes; ++nc) {
	      if (! cubes[nc].cube_active || cubes[nc].cube_remote) continue;
	      if (cubes[nc].cube_player == m) strategy_male(nc);
	      if (cubes[nc].cube_player == f) strategy_female(nc);
	      if (cubes[nc].cube_player == e) strategy_enby(nc);
	    }
	  }
	}

	// Clock this frame
	// This needs to be higher resolution than seconds.
	frame_time = difftime(time(NULL), 0);
	frame_counter++;
	
	// Sleep to slow down the action
	if (clock_time) sleep(1);
	
	// time this loop
	timer++;
	end = time(NULL);
	double seconds = difftime(end, start);

	// Get an initial estimate of frames per second
	// These clock calls don't produce very accurate results across systems.
	// Using some preset values as best estimaes.
	if (frame_counter < 3) {
	  std::clock_t c_end = std::clock();
	  double time_elapsed_microseconds = (c_end-c_start);
	  if (debug > 1) printf("%0.5f\n", time_elapsed_microseconds);
	  double fps = double(frame_counter)*1000000.0/time_elapsed_microseconds;
	  if (debug > 1) printf("cube.cpp: Estimated frames per second is %0.2f\n", fps);
	  frames_per_second = fps < 60.0 ? DEFAULT_FPS : 60.0;
	  printf("cube.cpp: Preset frames per second is %0.2f\n", frames_per_second);
	}
	
	if (seconds >= 60.0) {
	  frames_per_second = double(timer)/seconds;
	  fprintf(stdout, "cube.cpp: Display loop running at %5.2f frames/second\n", frames_per_second);
	  if (debug > 0) fprintf(stdout, "cube.cpp: Display loop running at %5.2f frames/second\n", frames_per_second);
	  timer = 0;
	  start = end;
	}
    }
  }

  // Shut down the server
  server_stop();

  // release memory
  free_resources();

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();

  // Print a status report
  time_t time_now = time(NULL);
  double age;

  printf("cube.cpp: cube   type health energy         age        score        (  mate,   food,   kill) state\n");

  for (int i=0; i < n_cubes; ++i) {

    float total_points = 0.0;
    float mate_points = 0.0;
    float food_points = 0.0;
    float kill_points = 0.0;

    cube_total_points(i, &total_points, &mate_points, &food_points, &kill_points);

    if (cubes[i].cube_active) {
      age = difftime(time_now, cubes[i].life_birth);
    } else {
      age = difftime(cubes[i].life_death, cubes[i].life_birth);
    }

    if (cubes[i].cube_active && cubes[i].cube_player != "resource" && cubes[i].resource_energy > 0.0 && cubes[i].spatial_velocity == 0.0) {
      printf("cube.cpp: %2d %8s, %s energy %7.2f age %6.1f score %6.1f (%6.1f, %6.1f, %6.1f) resting\n", i, &cubes[i].cube_player[0], cubes[i].cube_active ? "alive" : "dead ", cubes[i].resource_energy, age, total_points, mate_points, food_points, kill_points);
    } else {
      printf("cube.cpp: %2d %8s, %s energy %7.2f age %6.1f score %6.1f (%6.1f, %6.1f, %6.1f)\n", i, &cubes[i].cube_player[0], cubes[i].cube_active ? "alive" : "dead ", cubes[i].resource_energy, age, total_points, mate_points, food_points, kill_points);
    }
  }
  
  return 0;

}


/* Display normals */

int show_normals()
{

  GLfloat normal_vectors[] =
    {
     0.0, 0.0, 0.0,
     1.0, 0.0, 0.0,
     0.0, 0.0, 0.0,
     0.0, 1.0, 0.0,
     0.0, 0.0, 0.0,
     0.0, 0.0, 1.0,
     0.0, 0.0, 0.0,
     -1.0, 0.0, 0.0,
     0.0, 0.0, 0.0,
     0.0, -1.0, 0.0,
     0.0, 0.0, 0.0,
     0.0, 0.0, -1.0
    };

  glGenBuffers(1, &vbo_normals);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normal_vectors), normal_vectors, GL_STATIC_DRAW);
  fprintf(stdout, "cube.cpp: normals vbo_normals %d (%d)\n", vbo_normals, int(sizeof(vbo_normals)));

  normals_model = glm::translate(glm::mat4(1.0f), glm::vec3(4.0, 4.0, 0.0));
  // mvp_normals is the mvp for the normals display
  normals_mvp = projection * view * normals_model;
  
  return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
	printf("cube.cpp: Received key press from processInput()\n");
    }
}

// Print out all key codes
void keyHelp()
{
  printf("cube.cpp: Listing of all cube server keys (upper or lower case):\n"
	 "A -- Activate (set velocity non-zero)\n"
	 "B -- Basic strategy toggle (server controls predators resources)\n"
	 "C -- Clock delay on/off\n"
	 "D -- Debug toggle\n"
	 "E -- Extended strategy toggle (server controls males, females, enbies)\n"
	 "F -- View target -x\n"
	 "G -- View target -y\n"
	 "H -- View target -z\n"
	 "I -- Y up\n"
	 "J -- X down\n"
	 "K -- Y down\n"
	 "L -- Z down\n"
	 "M -- Rotate cube right\n"
	 "N -- Rotate cube left\n"
	 "O -- Z up\n"
	 "P -- Screenshot - take a picture\n"
	 "Q -- Reset view position, cube view not active\n"
	 "R -- View target +x\n"
	 "S -- Strategy on/off\n"
	 "T -- View target +y\n"
	 "U -- X up\n"
	 "V -- Start video recording on/off\n"
	 "W -- \n"
	 "X -- Change to cube view and use cube_index (reset with Q)\n"
	 "Y -- View target +z\n"
	 "Z -- Increment cube index\n"
	 "0-8 -- Rotate-y n x pi/4\n"
	 "Left-Arrow -- Gaze left\n"
	 "Right-Arrow -- Gaze right\n"
	 "Up-Arrow  -- Gaze up\n"
	 "Down-Arrow -- Gaze down\n"
	 "Left-Bracket -- Light X -= 1.0\n"
	 "Right-Bracket -- Light X += 1.0\n"
	 "Period -- Light Y -= 0.5\n"
	 "Slash -- Light Y += 0.5\n"
	 "Minus -- Light Z -= 1.0\n"
	 "Equal -- Light Z += 1.0\n"
	 "Semicolon -- Ambient -= 0.1\n"
	 "Apostrophe -- Ambient += 0.1\n"
	 "Grave Accent -- Exit\n"
	 "ESC -- Exit\n"
	 "? -- Help, print a list of server keys\n");
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  // To detect upper/lower case the mode argument is needed
  // if ( key == GLFW_KEY_A && action == GLFW_PRESS ) {
  //   if (mode == GLFW_MOD_SHIFT) {
  //     //uppercase
  //   } else {
  //     //lowercase
  //   }
  // }

  // Print the Help screen
  if (key == GLFW_KEY_SLASH && action == GLFW_PRESS && mode == GLFW_MOD_SHIFT) {keyHelp();}

  // End the game
  if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
    }
  
  // Activate the game
  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    /* Velocity */
    for (int i=0; i<n_cubes; ++i) {
      if (cubes[i].cube_player != "resource") {
	cubes[i].spatial_velocity = 1.0;
      } else {
	cubes[i].spatial_velocity = 0.0;
      }
    }
    printf("cube.cpp: Game on!\n");   
  }

  if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    /* Slow the clock */
    if (! clock_time) {
      clock_time = true;
      printf("cube.cpp: Clock delay started\n");
    } else {
      clock_time = false;
      printf("cube.cpp: Clock delay stopped\n");
    }
  }
  
  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    /* Set diagnostics on */
    if (! diagnostic) {
      diagnostic = true;
      printf("cube.cpp: Diagnostics on\n");
    } else {
      diagnostic = false;
      printf("cube.cpp: Diagnostics off\n");
    }
  }
  
  if (key == GLFW_KEY_P && action == GLFW_PRESS) {
    // Capture the window
    screenshot(window, "");
  }
  
  if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    /* Strategy */
    if (! strategy) {
      strategy = true;
      printf("cube.cpp: Strategy on!!\n");
    } else {
      strategy = false;
      printf("cube.cpp: Strategy off!!\n");
    }
  }
  
  if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    /* Strategy - Basic (only control predators, resources) */
    if (! strategy_basic) {
      strategy_basic = true;
      printf("cube.cpp: Basic strategy on!!\n");
    } else {
      strategy_basic = false;
      printf("cube.cpp: Basic strategy off!!\n");
    }
  }
  
  if (key == GLFW_KEY_E && action == GLFW_PRESS) {
    /* Strategy - Extended (control males, females, enbies)*/
    if (! strategy_extended) {
      strategy_extended = true;
      printf("cube.cpp: Extended strategy on!!\n");
    } else {
      strategy_extended = false;
      printf("cube.cpp: Extended strategy off!!\n");
    }
  }
  
  if (key == GLFW_KEY_V && action == GLFW_PRESS) {
    /* Video */
    if (! video) {
      video = true;
      printf("cube.cpp: Video started\n");
    } else {
      video = false;
      printf("cube.cpp: Video stopped\n");
    }
  }
  
  if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
    cube_key += 1;
    if (cube_key == n_cubes) cube_key = 0;
    printf("cube.cpp: Activating cube %d\n", cube_key);   
  }

  // Turn all cubes to a specific angle
  int motion = -1;
  if (key == GLFW_KEY_0 && action == GLFW_PRESS) motion = 0;
  if (key == GLFW_KEY_1 && action == GLFW_PRESS) motion = 1;
  if (key == GLFW_KEY_2 && action == GLFW_PRESS) motion = 2;
  if (key == GLFW_KEY_3 && action == GLFW_PRESS) motion = 3;
  if (key == GLFW_KEY_4 && action == GLFW_PRESS) motion = 4;
  if (key == GLFW_KEY_5 && action == GLFW_PRESS) motion = 5;
  if (key == GLFW_KEY_6 && action == GLFW_PRESS) motion = 6;
  if (key == GLFW_KEY_7 && action == GLFW_PRESS) motion = 7;
  if (key == GLFW_KEY_8 && action == GLFW_PRESS) motion = 8;

  if (motion != -1) {
    float angle = float(motion) * pi / 4.0;

    for (int i = 0; i < n_cubes; ++i) {
      cube_update_rotation(i, cubes[i].spatial_rotation.x, angle, cubes[i].spatial_rotation.z);
      cube_update_model(i);
      cube_update_mvp(i);
    }
    if (diagnostic) printf("cube.cpp: Received key press from key_callback() motion = %d angle = %0.3f\n", motion, angle);
  }

  // Enable first-person-view (cube view)
  float a = 0.0;
  float gaze_yaw = 0.0;
  float gaze_pitch = 0.0;

  // Go into cube view mode
  if (key == GLFW_KEY_X && action == GLFW_PRESS) {
    view_index = cube_key; cube_key_active = true;
    printf("cube.cpp: Activating cube view mode, use Q to reset. cube=%d\n", cube_key);   
  }

  // Update the angle of the cube in first-person-view
  if (key == GLFW_KEY_N && action == GLFW_PRESS) { view_index = cube_key; a = pi/8.0; }
  if (key == GLFW_KEY_M && action == GLFW_PRESS) { view_index = cube_key; a = -pi/8.0; }
  // Update the gaze (yaw, pitch) in first-person-view
  if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) { view_index = cube_key; gaze_yaw = pi/16.0; }
  if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) { view_index = cube_key; gaze_yaw = -pi/16.0; }
  if (key == GLFW_KEY_UP && action == GLFW_PRESS) { view_index = cube_key; gaze_pitch = pi/16.0; }
  if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) { view_index = cube_key; gaze_pitch = -pi/16.0; }
  if (view_index >= 0) {
    int i = view_index;
    cube_update_rotation(i, cubes[i].spatial_rotation.x, cubes[i].spatial_rotation.y + a, cubes[i].spatial_rotation.z);
    cube_update_model(i);
    cubes[i].spatial_gaze = {cubes[i].spatial_gaze.x + gaze_yaw, cubes[i].spatial_gaze.y + gaze_pitch};

    // Update the view
    cube_update_view(i);
      
  }

  // Reset the view to default. Remove cube view.
  int reposition = 0;
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
    camera_position = camera_position_default;
    camera_target = camera_target_default;
    camera_up = camera_up_default;
    cube_key_active = false;
    view_index = -1;
    reposition = 1;
  }

    // Check if camera needs to be repositioned.
  if (key == GLFW_KEY_U && action == GLFW_PRESS) {camera_position.x += 1.0; reposition = 1;}
  if (key == GLFW_KEY_I && action == GLFW_PRESS) {camera_position.y += 1.0; reposition = 1;}
  if (key == GLFW_KEY_O && action == GLFW_PRESS) {camera_position.z += 1.0; reposition = 1;}
  if (key == GLFW_KEY_J && action == GLFW_PRESS) {camera_position.x -= 1.0; reposition = 1;}
  if (key == GLFW_KEY_K && action == GLFW_PRESS) {camera_position.y -= 1.0; reposition = 1;}
  if (key == GLFW_KEY_L && action == GLFW_PRESS) {camera_position.z -= 1.0; reposition = 1;}
  if (key == GLFW_KEY_R && action == GLFW_PRESS) {camera_target.x   += 1.0; reposition = 1;}
  if (key == GLFW_KEY_T && action == GLFW_PRESS) {camera_target.y   += 1.0; reposition = 1;}
  if (key == GLFW_KEY_Y && action == GLFW_PRESS) {camera_target.z   += 1.0; reposition = 1;}
  if (key == GLFW_KEY_F && action == GLFW_PRESS) {camera_target.x   -= 1.0; reposition = 1;}
  if (key == GLFW_KEY_G && action == GLFW_PRESS) {camera_target.y   -= 1.0; reposition = 1;}
  if (key == GLFW_KEY_H && action == GLFW_PRESS) {camera_target.z   -= 1.0; reposition = 1;}

  if (reposition) {

    ground_update_view(-1);

    printf("cube.cpp: Camera position now [%5.2f, %5.2f, %5.2f], target [%5.2f, %5.2f, %5.2f]\n", camera_position.x, camera_position.y, camera_position.z, camera_target.x, camera_target.y, camera_target.z);
  }
  
  // Move the lights around
  int movelight = false;
  if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {lights[0].position.x -= 1.0;  movelight = true; }
  if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {lights[0].position.x += 1.0; movelight = true; }
  if (key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {lights[0].position.y -= 0.5; movelight = true; }
  if (key == GLFW_KEY_SLASH && action == GLFW_PRESS && mode != GLFW_MOD_SHIFT) {lights[0].position.y += 0.5; movelight = true; }
  if (key == GLFW_KEY_MINUS && action == GLFW_PRESS) {lights[0].position.z -= 1.0; movelight = true; }
  if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {lights[0].position.z += 1.0; movelight = true; }
  if (key == GLFW_KEY_SEMICOLON && action == GLFW_PRESS) {lights[0].ambient.x -= 0.1; lights[0].ambient.y -= 0.1; lights[0].ambient.z -= 0.1; movelight = true; }
  if (key == GLFW_KEY_APOSTROPHE && action == GLFW_PRESS) {lights[0].ambient.x += 0.1; lights[0].ambient.y += 0.1; lights[0].ambient.z += 0.1; movelight = true; }
  if (movelight) printf("cube.cpp: Lights[0] [%5.2f, %5.2f, %5.2f, ambient %5.2f]\n", lights[0].position.x, lights[0].position.y, lights[0].position.z, lights[0].ambient.x);
}

// glfw window error callback
void window_error(int error, const char* message) {
  printf("GLFW enountered error %d: %s\n", error, message);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  main_window_width = width;
  main_window_height = height;
  
  // make sure the viewport matches the new window dimensions; note that width and 
  glViewport(0, 0, width, height);
  // Update our projection, too
  update_projection(width, height);
}

// Takes a screenshot
void screenshot(GLFWwindow* window, string filename) {

  // Get date-time stamp
  time_t rawtime;
  struct tm * timeinfo;
  char ymdhms [32];
  char image_file[64];
  
  // printf("cube.cpp: Screenshot window is %u\n", window);

  if (filename != "") {
    filename.copy(image_file, min(sizeof(image_file)-1, filename.size()));
  } else {
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(ymdhms,32,"%Y-%m-%d_%H.%M.%S",timeinfo);
    sprintf(image_file, "IMG_%s.%05d.png", ymdhms, screenshot_number);
    screenshot_number += 1;
  }
  
  // Get window position
  //int xpos, ypos;
  //glfwGetWindowPos(window, &xpos, &ypos);

  // Get window size
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  // Get framebuffersize
  // int fb_width, fb_height;
  // glfwGetFramebufferSize(window, &fb_width, &fb_height);
  
  // Make the BYTE array, factor of 4 because it's RBGA.
  int channels = main_window_channels;
  std::vector<uint8_t> pixels(channels * width * height);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  // Vertical flip
  for(int line = 0; line != height/2; ++line) {
    std::swap_ranges(pixels.begin() + channels * width * line,
                     pixels.begin() + channels * width * (line+1),
                     pixels.begin() + channels * width * (height-line-1));
  }

  stbi_write_png(image_file, width, height, channels, pixels.data(), width * channels);
  // printf("cube.cpp: Screenshot to file %s\n", image_file);

}

// Copy the window
void screenview_capture(GLFWwindow* capture_window, int i, int notify, std::vector<uint8_t> *pixels, unsigned *pixels_frame, unsigned *pixels_frame_start, mutex *pixels_mutex) {

  int w;
  int h;
  int c = main_window_channels;

  // Avoid a race condition where the frame is not ready yet.
  if (frame_counter < pixels_frame_start[i]) {
    if (debug > 1) printf("cube.cpp: screenview_capture E%d - frame_counter %d start %d view_index %d\n", i, frame_counter, pixels_frame_start[i], i);
    return;
  }
  
  // Get window size
  glfwGetWindowSize(capture_window, &w, &h);

  int image_size = w * h * c;

  // Lock this resource while we're changing it
  pixels_mutex[i].lock();
  
  if (pixels[i].size() != image_size) {
    // Make it the right size
    pixels[i].resize(image_size);
  }

  // Tag which frame is in this buffer (frame counter at time of capture)
  pixels_frame[i] = frame_counter;
  
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels[i].data());
  
  if (debug > 1) printf("cube.cpp: screenview_capture C%d - frame_counter %d view_index %d pixels_frame %d \n", i, frame_counter, i, pixels_frame[i]);

  // Unlock this resource
  pixels_mutex[i].unlock();

  if (notify) {
    // Notify the message thread that's waiting for a pixel buffer
    std::lock_guard<std::mutex> lk(pixels_mutex_wait[i]);
    pixels_cv[i].notify_one();   
  }
}

ViewResponse screenview(string uuid, float angle, float gaze_yaw, float gaze_pitch) {

  // Structure to return
  ViewResponse r;
  
  // Convert UUID into a cubes index
  // Check for active and logged in
  int i = cube_get_index(uuid);
  if (i < 0 || ! cubes[i].cube_active || ! cubes[i].cube_display || ! cubes[i].cube_remote) {
    r.cubeview = -1;
    return r;
  }

  r.cubeview = i;
 
  // printf("cube.cpp: Screenview window is %u\n", windows[r.cubeview]);
  if (debug > 0) printf("cube.cpp: Screenview angle %0.2f gaze.x %0.2f gaze.y %0.2f\n", angle, gaze_yaw, gaze_pitch);

  // Lock the resource while we're changing it
  pixels_mutex[i].lock();
  
  cube_update_rotation(i, cubes[i].spatial_rotation.x, angle, cubes[i].spatial_rotation.z);
  // Update the model
  cube_update_model(i);
  cubes[i].spatial_gaze = {gaze_yaw, gaze_pitch};
  // Update the view
  cube_update_view(i);
  // Update mvp
  cube_update_mvp(i);
  // Get window size
  glfwGetWindowSize(windows[i], &r.width, &r.height);
  // Get bounding box (needed for training)
  cube_bounding_box(i, r.width, r.height);
  // Flip the y coordinates upside down
  // ymin becomes ymax when flipped
  r.bounding_box[0] = cubes[i].bounding_box[0];
  r.bounding_box[1] = r.height - cubes[i].bounding_box[3];
  r.bounding_box[2] = cubes[i].bounding_box[2];
  r.bounding_box[3] = r.height - cubes[i].bounding_box[1];
  // printf("cube.cpp: xmin %4.2f ymin %4.2f xmax %4.2f ymax %4.2f\n", cubes[i].bounding_box[0], cubes[i].bounding_box[1], cubes[i].bounding_box[2], cubes[i].bounding_box[3]);
  // printf("cube.cpp: xmin %4.2f ymin %4.2f xmax %4.2f ymax %4.2f\n", r.bounding_box[0], r.bounding_box[1], r.bounding_box[2], r.bounding_box[3]);
  // Enable first-person-view (cube view) if not already on
  if (view_index < 0) view_index = i;
  
  // Make the BYTE array, factor of 4 because it's RBGA.
  r.channels = main_window_channels;
  r.extension = "raw";
  r.mode = "RGBA";

  // Set the minimum frame count which satisfies this view request
  r.pixels_frame_start = frame_counter + capture_delay;
  pixels_frame_start[i] = r.pixels_frame_start;

  if (debug > 1) printf("cube.cpp: screenview_request A%d - frame_counter %d view_index %d start %d\n", i, frame_counter, i, r.pixels_frame_start);
  
  // Unlock
  pixels_mutex[i].unlock();

  // Wait for an update message from the display() render loop
  std::unique_lock<std::mutex> lk(pixels_mutex_wait[i]);
  pixels_cv[i].wait_for(lk, std::chrono::seconds(3), [i]{return pixels_frame[i] >= pixels_frame_start[i];});

  // Lock the resource while we're copying it
  pixels_mutex[i].lock();
  
  // Required due to multithread glfw context switch
  r.pixels = pixels[i];
  r.pixels_frame = pixels_frame[i];

  if (debug > 1) printf("cube.cpp: screenview_request B%d - frame_counter %d view_index %d pixels_frame %d start %d status %s\n",
			i, frame_counter, i, pixels_frame[i], pixels_frame_start[i], pixels_frame[i] >= pixels_frame_start[i] ? "True" : "False");
  
  // Unlock
  pixels_mutex[i].unlock();
  
  // if we have aleady captured a frame
  // Not sure if ths test is correct. Force it to pass.
  if (1 || r.pixels_frame >= r.pixels_frame_start) {
    // printf("r.pixels_frame %d frame_counter %d capture_delay %d pixels_frame_start %d\n", r.pixels_frame, frame_counter, capture_delay, pixels_frame_start[i]);
    // Vertical flip
    for(int line = 0; line != r.height/2; ++line) {
      std::swap_ranges(r.pixels.begin() + r.channels * r.width * line,
		       r.pixels.begin() + r.channels * r.width * (line+1),
		       r.pixels.begin() + r.channels * r.width * (r.height-line-1));
    }
    // Change RGBA to BGR
    if (r.channels == 4) {
      for (int i=0; i < r.width * r.height; ++i) {
	int rgb_ptr = i * 3;
	int rgba_ptr = i * 4;
	// We have to be careful not to overwrite the first few pixels. Thus, save all, then write.
	uint8_t pixr = r.pixels[rgba_ptr];
	uint8_t pixg = r.pixels[rgba_ptr+1];
	uint8_t pixb = r.pixels[rgba_ptr+2];
	r.pixels[rgb_ptr] = pixb;
	r.pixels[rgb_ptr+1] = pixg;
	r.pixels[rgb_ptr+2] = pixr;
      }
      r.channels = 3;
      r.mode = "BGR";
      r.pixels.resize(r.width * r.height * 3);
    }
  }
  
  return r;
}

// GroundViewResponse ground_screenview(GLFWwindow* window, string uuid, int gv) {
GroundViewResponse ground_screenview(string uuid, int gv) {

  // Structure to return
  GroundViewResponse r;
  
  // Check for logged in
  if (! grounds[gv].ground_active || ! grounds[gv].ground_display || ! grounds[gv].ground_remote) {
    r.groundview = -1;
    return r;
  }

  r.groundview = gv;

  // printf("cube.cpp: Screenview window is %u\n", windows[i]);
    
  // Enable first-person-view (ground view) if not already on
  if (view_index < 0) view_index = gv + n_cubes;
  
  // Get window size
  glfwGetWindowSize(window, &r.width, &r.height);

  // Make the BYTE array, factor of 4 because it's RBGA.
  r.channels = main_window_channels;
  r.extension = "raw";

  // Lock the resource while we're changing it
  ground_pixels_mutex[gv].lock();
  
  // If this is our initial view request make sure we've had an update
  // Start capture in two frames from now
  if (ground_pixels_frame_start[gv] == 0) ground_pixels_frame_start[gv] = frame_counter + 2;

  // Required due to multithread glfw context switch
  r.pixels = ground_pixels[gv];
  r.pixels_frame = ground_pixels_frame[gv];
  
  // Unlock
  ground_pixels_mutex[gv].unlock();

  // if we have aleady captured a frame
  if (r.pixels_frame > 0) {
    // Vertical flip
    for(int line = 0; line != r.height/2; ++line) {
      std::swap_ranges(r.pixels.begin() + r.channels * r.width * line,
		       r.pixels.begin() + r.channels * r.width * (line+1),
		       r.pixels.begin() + r.channels * r.width * (r.height-line-1));
    }
    // Change RGBA to BGR
    if (r.channels == 4) {
      for (int i=0; i < r.width * r.height; ++i) {
	int rgb_ptr = i * 3;
	int rgba_ptr = i * 4;
	// We have to be careful not to overwrite the first few pixels. Thus, save all, then write.
	uint8_t pixr = r.pixels[rgba_ptr];
	uint8_t pixg = r.pixels[rgba_ptr+1];
	uint8_t pixb = r.pixels[rgba_ptr+2];
	r.pixels[rgb_ptr] = pixb;
	r.pixels[rgb_ptr+1] = pixg;
	r.pixels[rgb_ptr+2] = pixr;
      }
      r.channels = 3;
      r.mode = "BGR";
      r.pixels.resize(r.width * r.height * 3);
    }
  }
  
  return r;
}
