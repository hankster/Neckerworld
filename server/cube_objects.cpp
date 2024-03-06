/**
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <mutex>
using namespace std;

/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>

/* GLFW */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/* GLM */
// #define GLM_MESSAGES
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/* Library for image loading, processing */
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

/* Function prototypes for this program */
# include "cube.h"
# include "cube_client.h"

/* Suggested distance constants
 DistanceConstantLinearQuadratic
7	1.0	0.7	1.8
13	1.0	0.35	0.44
20	1.0	0.22	0.20
32	1.0	0.14	0.07
50	1.0	0.09	0.032
65	1.0	0.07	0.017
100	1.0	0.045	0.0075
160	1.0	0.027	0.0028
200	1.0	0.022	0.0019
325	1.0	0.014	0.0007
600	1.0	0.007	0.0002
3250	1.0	0.0014	0.000007
*/

/* Colors */
glm::vec4 color_red = {1.0f, 0.0f, 0.0f, 1.0f};
glm::vec4 color_green = {0.0f, 1.0f, 0.0f, 1.0f};
glm::vec4 color_blue = {0.0f, 0.0f, 1.0f, 1.0f};
glm::vec4 color_purple = {1.0f, 0.0f, 1.0f, 1.0f};
glm::vec4 color_black = {0.0f, 0.0f, 0.0f, 1.0f};
glm::vec4 color_white = {1.0f, 1.0f, 1.0f, 1.0f};

/* Light colors and intensities */
glm::vec3 light_color_orange = {0.98f, 0.725f, 0.278f};
glm::vec3 light_color_white = {1.0f, 1.0f, 1.0f};
glm::vec3 light_000 = {0.0f, 0.0f, 0.0f};
glm::vec3 light_025 = {0.25f, 0.25f, 0.25f};
glm::vec3 light_050 = {0.5f, 0.5f, 0.5f,};
glm::vec3 light_100 = {1.0f, 1.0f, 1.0f};
glm::vec3 light_ambient = {0.1f, 0.1f, 0.1f};
glm::vec3 light_diffuse = {0.8f, 0.8f, 0.8f};
glm::vec3 light_specular = {0.2f, 0.2f, 0.2f};
glm::vec3 light = {0.0, 0.0, 0.0};

/* Some standard materials */
glm::vec3 material_plain_ambient = {0.0f, 0.0f, 0.0f};
glm::vec3 material_plain_diffuse = {1.0f, 1.0f, 1.0f};
glm::vec3 material_plain_specular = {0.0f, 0.0f, 0.0f};
float     material_plain_shininess = 0.0f;
glm::vec3 material_highlight_ambient = {0.0f, 0.0f, 0.0f};
glm::vec3 material_highlight_diffuse = {0.8f, 0.8f, 0.8f};
glm::vec3 material_highlight_specular = {0.5f, 0.5f, 0.5f};
float     material_highlight_shininess = 0.25f;

/* Full six panel texture map */
float cube_texture_map_6_panes[] = {
				    0.00000, 0.00000, 0.16666, 0.00000, 0.16666, 1.00000, 0.00000, 1.00000,
				    0.33333, 0.00000, 0.50000, 0.00000, 0.50000, 1.00000, 0.33333, 1.00000,
				    0.83333, 0.00000, 0.83333, 1.00000, 0.66666, 1.00000, 0.66666, 0.00000,
				    1.00000, 0.00000, 1.00000, 1.00000, 0.83333, 1.00000, 0.83333, 0.00000,
				    0.16666, 1.00000, 0.16666, 0.00000, 0.33333, 0.00000, 0.33333, 1.00000,
				    0.66666, 0.00000, 0.66666, 1.00000, 0.50000, 1.00000, 0.50000, 0.00000
};

/* Standard vertices for a cube */
GLfloat cube_vertices[] =
  {
   // front (vec3 pos)
   -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
   -1.0,  1.0,  1.0,
   // top
   -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    1.0,  1.0, -1.0,
   -1.0,  1.0, -1.0,
   // back
    1.0, -1.0, -1.0,
   -1.0, -1.0, -1.0,
   -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
   // bottom
   -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0, -1.0,  1.0,
   -1.0, -1.0,  1.0,
   // left
   -1.0, -1.0, -1.0,
   -1.0, -1.0,  1.0,
   -1.0,  1.0,  1.0,
   -1.0,  1.0, -1.0,
   // right
    1.0, -1.0,  1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    1.0,  1.0,  1.0
  };

/* For bounding box calculations */
GLfloat cube_corners[] =
  {
   // front (vec3 pos)
   -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
   -1.0,  1.0,  1.0,
   // back
    1.0, -1.0, -1.0,
   -1.0, -1.0, -1.0,
   -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0
  };
  
/* Standard normals for a cube */
GLfloat cube_normals[] =
  {
   // front (vec3 normal)
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   0.0, 0.0, 1.0,
   // top
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   0.0, 1.0, 0.0,
   // back
   0.0, 0.0, -1.0,
   0.0, 0.0, -1.0,
   0.0, 0.0, -1.0,
   0.0, 0.0, -1.0,
   // bottom
   0.0, -1.0, 0.0,
   0.0, -1.0, 0.0,
   0.0, -1.0, 0.0,
   0.0, -1.0, 0.0,
   // left
   -1.0, 0.0, 0.0,
   -1.0, 0.0, 0.0,
   -1.0, 0.0, 0.0,
   -1.0, 0.0, 0.0,
   // right
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0,
   1.0, 0.0, 0.0
  };
  
/* Routine to create a new cube. */

int new_cube(int nc, string cube_player, string cube_uuid, string cube_emoticon, string cube_firstname, float cube_scale_factor, int cube_type, string cube_color_class, glm::vec4 cube_color, int cube_material, string cube_surface, int cube_texture_index, float cube_texture_map[6*4*2], float x, float y, float z, float rotX, float rotY, float rotZ, float spatial_radius, float resource_energy)
{

  GLfloat cube_vertices_normals[] =
    {
     // front (vec3 pos, vec3 normal)
     -1.0, -1.0,  1.0, 0.0, 0.0, 1.0,
     1.0, -1.0,  1.0, 0.0, 0.0, 1.0,
     1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
    -1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
    // top
     -1.0,  1.0,  1.0, 0.0, 1.0, 0.0,
     1.0,  1.0,  1.0, 0.0, 1.0, 0.0,
     1.0,  1.0, -1.0, 0.0, 1.0, 0.0,
    -1.0,  1.0, -1.0, 0.0, 1.0, 0.0,
    // back
     1.0, -1.0, -1.0, 0.0, 0.0, -1.0,
    -1.0, -1.0, -1.0, 0.0, 0.0, -1.0,
    -1.0,  1.0, -1.0, 0.0, 0.0, -1.0,
     1.0,  1.0, -1.0, 0.0, 0.0, -1.0,
    // bottom
     -1.0, -1.0, -1.0, 0.0, -1.0, 0.0,
     1.0, -1.0, -1.0, 0.0, -1.0, 0.0,
     1.0, -1.0,  1.0, 0.0, -1.0, 0.0,
    -1.0, -1.0,  1.0, 0.0, -1.0, 0.0,
    // left
     -1.0, -1.0, -1.0, -1.0, 0.0, 0.0,
    -1.0, -1.0,  1.0, -1.0, 0.0, 0.0,
    -1.0,  1.0,  1.0, -1.0, 0.0, 0.0,
    -1.0,  1.0, -1.0, -1.0, 0.0, 0.0,
    // right
     1.0, -1.0,  1.0, 1.0, 0.0, 0.0,
     1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
     1.0,  1.0, -1.0, 1.0, 0.0, 0.0,
     1.0,  1.0,  1.0, 1.0, 0.0, 0.0
  };
  
  // This array must be supplied by incoming JSON data
  GLfloat cube_texture_map_coords[6*4*2];

  glGenBuffers(1, &cubes[nc].vbo_cube_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, cubes[nc].vbo_cube_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
  // fprintf(stdout, "cube.cpp: new_cube vbo_cube_vertices %d (%d), ", cubes[nc].vbo_cube_vertices, int(sizeof(cube_vertices))); 

  glGenBuffers(1, &cubes[nc].vbo_cube_normals);
  glBindBuffer(GL_ARRAY_BUFFER, cubes[nc].vbo_cube_normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
  // fprintf(stdout, "vbo_cube_normals %d (%d), ", cubes[nc].vbo_cube_normals, int(sizeof(cube_normals))); 

  // Copy our texture mapping coordinates from incoming JSON data
  for ( int n = 0; n < 6*4*2; ++n) {
    cube_texture_map_coords[n] = cube_texture_map[n];
    cubes[nc].cube_texture_map[n] = cube_texture_map[n];
  }
  
  glGenBuffers(1, &cubes[nc].vbo_cube_texture_map_coords);
  glBindBuffer(GL_ARRAY_BUFFER, cubes[nc].vbo_cube_texture_map_coords);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texture_map_coords), cube_texture_map_coords, GL_STATIC_DRAW);
  // fprintf(stdout, "vbo_cube_texture_map_coords %d (%d), ", cubes[nc].vbo_cube_texture_map_coords, int(sizeof(cube_texture_map_coords))); 

  GLushort cube_elements[] = {
    // front
    0,  1,  2,
    2,  3,  0,
    // top
    4,  5,  6,
    6,  7,  4,
    // back
    8,  9, 10,
    10, 11,  8,
    // bottom
    12, 13, 14,
    14, 15, 12,
    // left
    16, 17, 18,
    18, 19, 16,
    // right
    20, 21, 22,
    22, 23, 20
  };

  glGenBuffers(1, &cubes[nc].ibo_cube_elements);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubes[nc].ibo_cube_elements);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
  // fprintf(stdout, "ibo_cube_elements %d (%d)\n", cubes[nc].ibo_cube_elements, int(sizeof(cube_elements)));

  /* Cube index */
  cubes[nc].cube_index = nc;
  /* Cube player description */
  cubes[nc].cube_player = cube_player;
  /* Cube universal id */
  cubes[nc].cube_uuid = cube_uuid;
  /* cube emoticon */
  cubes[nc].cube_emoticon = cube_emoticon;
  /* cube firstname */
  cubes[nc].cube_firstname = cube_firstname;
  /* Activity state */
  cubes[nc].cube_active = true;
  /* Display state */
  cubes[nc].cube_display = true;
  /* Remote control state */
  cubes[nc].cube_remote = false;
  /* Set the scale factor */
  cubes[nc].cube_scale_factor = cube_scale_factor;
  cube_update_scale(nc, cube_scale_factor);
  /* type 0 = wireframe, type 1 = fill, type 2 = textured, type 3 = lighted */
  cubes[nc].cube_type = cube_type;
  /* Save the cube color class */
  cubes[nc].cube_color_class = cube_color_class;
  /* Set the drawing color if needed */
  cubes[nc].cube_color = cube_color;
  /* Material to use for this cube */
  cubes[nc].cube_material = cube_material;
  /* Surface used for this cube */
  cubes[nc].cube_surface = cube_surface;
  /* Texture layer to use for this cube */
  cubes[nc].cube_texture_index = cube_texture_index;
  /* Miscellaneous and extendable parameters */
  //cube_parameters    { ... }
  
  /* Spatial position */
  cubes[nc].spatial_position = glm::vec3(x, y, z);
  cubes[nc].spatial_position_previous = glm::vec3(x, y, z);
  cube_update_translation(nc);
  /* Spatial rotation */
  cube_update_rotation(nc, rotX, rotY, rotZ);
  /* Cubes viewpoint settings */
  cubes[nc].spatial_gaze = {0.0, 0.0};
  /* Cube's spatial radius */
  cubes[nc].spatial_radius = spatial_radius;
  /* Destination */
  cubes[nc].spatial_destination = {0.0, 0.0, 0.0};
  /* Velocity */
  cubes[nc].spatial_velocity = 0.0;
  /* Special timer for backup processing */
  cubes[nc].spatial_position_timer = 0;
  
  cube_update_model(nc);
  cube_update_mvp(nc);

  /* Initialize targets */
  cubes[nc].life_target_male = -1;
  cubes[nc].life_target_female = -1;
  cubes[nc].match_target = -1;
  cubes[nc].resource_target = -1;
  
  /* Initialize parents */
  cubes[nc].life_father = "";
  cubes[nc].life_mother = "";
  cubes[nc].life_birth = time(NULL);
  cubes[nc].life_death = 0;
  
  /* Initial resource energy */
  cubes[nc].resource_energy = resource_energy;

  // Record the starting waypoint
  cube_update_waypoints(nc, IDLE);

  return 0;

}

bool cube_mates(int male, int female) {

  // Check for space for another child cube
  if (n_cubes == NC) {
    if (diagnostic) printf("cube.cpp: No more room for babies\n");
    return false;
  }

  // Log the mate
  cubes[male].life_mates.push_back(cubes[female].cube_uuid);
  cubes[female].life_mates.push_back(cubes[male].cube_uuid);
  
  // Log the mate times
  time_t mate_time = time(NULL);
  cubes[male].life_mate_times.push_back(mate_time);
  cubes[female].life_mate_times.push_back(mate_time);

  // Log the mate energy provided
  float male_resource_energy = cubes[male].resource_energy / 2.0;
  float female_resource_energy = cubes[female].resource_energy / 2.0;
  float new_resource_energy = male_resource_energy + female_resource_energy;
  cubes[male].resource_energy -= male_resource_energy;
  cubes[female].resource_energy -= female_resource_energy;
  cubes[male].life_energy.push_back(male_resource_energy);
  cubes[female].life_energy.push_back(female_resource_energy);

  // Create a new cube from mating.
  int new_cube_index = n_cubes;
  int child = -1;
  string new_cube_player = "";
  float genes = random1();
  if (genes <= 0.475) {
    new_cube_player = "male";
    child = male;
  }
  if (genes > 0.475 && genes <= 0.95) {
    new_cube_player = "female";
    child = female;
  }
  if (genes > 0.95) {
    new_cube_player = "enby";
  }
  string new_cube_uuid = get_uuid();
  float new_cube_scale_factor = (cubes[male].cube_scale_factor + cubes[female].cube_scale_factor) / 2.0;
  string new_cube_emoticon;
  string new_cube_firstname;
  GLint new_cube_type;
  string new_cube_color_class;
  glm::vec4 new_cube_color;
  int new_cube_material;
  string new_cube_surface;
  int new_cube_texture_index;
  float* new_cube_texture_map;
  string new_cube_texture_file;
  if (child >= 0) {
    // Child will be a male or female
    new_cube_emoticon = cubes[child].cube_emoticon;
    new_cube_firstname = cubes[child].cube_firstname;
    new_cube_type = cubes[child].cube_type;
    new_cube_color_class = cubes[child].cube_color_class;
    new_cube_color = cubes[child].cube_color;
    new_cube_material = cubes[child].cube_material;
    new_cube_surface = cubes[child].cube_surface;
    new_cube_texture_index = cubes[child].cube_texture_index;
    new_cube_texture_map = cubes[child].cube_texture_map;
  } else {
    // Child will be enby.
    float enby_selector = random1();
    if (enby_selector < 0.33) {
      new_cube_emoticon = "1f60a";
      new_cube_firstname = "Bailey";
      new_cube_texture_file = "../assets/texture-1f60a-3072x512.png";
    }
    if (enby_selector >= 0.33 && enby_selector <= 0.66) {
      new_cube_emoticon = "1f61a";
      new_cube_firstname = "Hunter";
      new_cube_texture_file = "../assets/texture-1f61a-3072x512.png";
    }
    if (enby_selector > 0.66) {
      new_cube_emoticon = "1f633";
      new_cube_firstname = "Justice";
      new_cube_texture_file = "../assets/texture-1f633-3072x512.png";
    }
    new_cube_type = cubes[female].cube_type;
    new_cube_color_class = "p";
    new_cube_color = color_purple;
    new_cube_material = cubes[female].cube_material;
    new_cube_surface = "";
    new_cube_texture_index = n_textures;
    int status = new_texture(n_textures, &new_cube_texture_file[0]);
    new_cube_texture_map = cube_texture_map_6_panes;
  }
  float new_x = (cubes[male].spatial_position.x + cubes[female].spatial_position.x) / 2.0;
  // Cube will grow up.
  float new_y = 0.0;
  float new_z = (cubes[male].spatial_position.z + cubes[female].spatial_position.z) / 2.0;
  float new_rotx = (cubes[male].spatial_rotation.x + cubes[female].spatial_rotation.x) / 2.0;
  float new_roty = (cubes[male].spatial_rotation.y + cubes[female].spatial_rotation.y) / 2.0;
  float new_rotz = (cubes[male].spatial_rotation.z + cubes[female].spatial_rotation.z) / 2.0;
  // Cube will grow up.
  float new_spatial_radius = 0.0;

  int status = new_cube(new_cube_index, new_cube_player, new_cube_uuid, new_cube_emoticon, new_cube_firstname, new_cube_scale_factor, new_cube_type, new_cube_color_class, new_cube_color, new_cube_material, new_cube_surface, new_cube_texture_index, new_cube_texture_map, new_x, new_y, new_z, new_rotx, new_roty, new_rotz, new_spatial_radius, new_resource_energy); 

  if (status) {
    fprintf(stderr, "cube.cpp: new_cube_mate - cube %d not configured\n", new_cube_index);
    return false;
  }

  cubes[new_cube_index].life_father = cubes[male].cube_uuid;
  cubes[new_cube_index].life_mother = cubes[female].cube_uuid;
  // For a cube born here, let it grow
  cube_update_scale(new_cube_index, 0.0);
  cube_update_model(new_cube_index);
  cube_update_mvp(new_cube_index);
  
  fprintf(stdout, "cube.cpp: %8s %2d & %s %d congratulations!. Child %d born. It's a %s. scale factor %4.2f, emoticon %s, type %d, material %d, texture %d, energy %0.2f\n", &cubes[male].cube_player[0], male, &cubes[female].cube_player[0], female, new_cube_index, &new_cube_player[0], new_cube_scale_factor, &new_cube_emoticon[0], new_cube_type, new_cube_material, new_cube_texture_index, new_resource_energy);

  n_cubes++;
  
  return true;
}

// Update scale for a cube
void cube_update_scale(int i, GLfloat cube_scale_factor) {
  cubes[i].cube_scale = glm::scale(glm::mat4(1.0f), glm::vec3(cube_scale_factor, cube_scale_factor, cube_scale_factor));
}

// Update the size of our cube - youth only
void cube_update_size(int i) {
  time_t time_now = time(NULL);
  double growth_time = difftime(time_now, cubes[i].life_birth);
  if (cubes[i].cube_active && cubes[i].life_mother != "") {
    GLfloat csf = cubes[i].cube_scale_factor;
    GLfloat csr = sqrt(2.0 * csf * csf);
    if (growth_time < LIFE_YOUTH_HOLD_TIME) {
      GLfloat csf_new = csf * (growth_time / LIFE_YOUTH_HOLD_TIME);
      cube_update_scale(i, csf_new);
      // printf("cube.cpp: %8s %2d csf_new %0.2f\n", &cubes[i].cube_player[0], i, csf_new);
      cubes[i].spatial_position.y = csf_new;
      cubes[i].spatial_position_previous.y = csf_new;
      cubes[i].spatial_radius = csr * (growth_time / LIFE_YOUTH_HOLD_TIME);
      cube_update_translation(i);
      cube_update_model(i);
      cube_update_mvp(i);
    } else {
      if (cubes[i].cube_scale[1][1] < csf) {
	cube_update_scale(i, csf);
	cubes[i].spatial_position.y = csf;
	cubes[i].spatial_position_previous.y = csf;
	cubes[i].spatial_radius = csr;
	cube_update_translation(i);
	cube_update_model(i);
	cube_update_mvp(i);
      }
    }
  }
}

// Update rotation for a cube
void cube_update_rotation(int i, GLfloat rotX, GLfloat rotY, GLfloat rotZ) {
  cubes[i].spatial_rotation = glm::vec3(rotX, rotY, rotZ);
  cubes[i].spatial_rotation_matrix = 
    glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0)) *  // X axis
    glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0)) *  // Y axis
    glm::rotate(glm::mat4(1.0f), rotZ, glm::vec3(0, 0, 1));   // Z axis
}

// Update the angle orientation of a cube.
void cube_update_angle(int me, float angle) {
  cube_update_rotation(me, cubes[me].spatial_rotation.x, angle, cubes[me].spatial_rotation.z);
}

// Update the translation far a cube
void cube_update_translation(int i) {
  cubes[i].spatial_translation = glm::translate(glm::mat4(1.0f), cubes[i].spatial_position);
}

// Update the model far a cube
void cube_update_model(int i) {
  cubes[i].cube_model = cubes[i].spatial_translation * cubes[i].spatial_rotation_matrix * cubes[i].cube_scale;
}

// Update the mvp for a cube
void cube_update_mvp(int i) {
  cubes[i].cube_mvp = projection * view * cubes[i].cube_model;
}

// Update the view if required
void cube_update_view(int i) {

  // See if we are in first person view mode
  if (view_index < 0) return;

  // If we own the view, update it.
  if (view_index == i) {
    float fpv_scale = cubes[i].cube_scale_factor;
    glm::vec3 fpv_position = cubes[i].spatial_position;
    glm::vec3 fpv_rotation = cubes[i].spatial_rotation;
    glm::vec2 fpv_gaze = cubes[i].spatial_gaze;
    glm::vec3 fpv_viewpoint = {fpv_position.x + fpv_scale*sin(fpv_rotation.y), fpv_position.y, fpv_position.z + fpv_scale*cos(fpv_rotation.y)};
    float radius = 10.0;
    float xt = fpv_viewpoint.x + radius*sin(fpv_rotation.y+fpv_gaze.x);
    float yt = fpv_viewpoint.y + radius*sin(fpv_gaze.y);
    float zt = fpv_viewpoint.z + radius*cos(fpv_rotation.y+fpv_gaze.x);
    glm::vec3 fpv_target = {xt, yt, zt};

    if (diagnostic) printf("cube.cpp: x %0.4f, z %0.4f, xv %0.4f, zv %0.4f, xt %0.4f, zt %0.4f\n", fpv_position.x, fpv_position.z, fpv_viewpoint.x, fpv_viewpoint.z, xt,  zt);		       
  
    /* Change the view */
    view = glm::lookAt(fpv_viewpoint, fpv_target, camera_up);
    
    // Now update the entire scene
    for (int i = 0; i < n_cubes; ++i) {
      cube_update_mvp(i);
    }

    // Take care of the grounds, too.
    for (int i = 0; i < n_grounds; ++i) {
      ground_update_mvp(i);
    }
  }
}

// Compute the distance between two cubes
GLfloat cube_distance(int i, int j) {

  float xi = cubes[i].spatial_position.x;
  float zi = cubes[i].spatial_position.z;
  float xj = cubes[j].spatial_position.x;
  float zj = cubes[j].spatial_position.z;
  float dx = xi - xj;
  float dz = zi - zj;
  return sqrt(dx*dx + dz*dz);
}

// Compute the cube bounding box
void cube_bounding_box(int i, int width, int height) {
  cubes[i].bounding_box[0] = 1000000.0; // x_min
  cubes[i].bounding_box[1] = 1000000.0; // y_min
  cubes[i].bounding_box[2] = 0.0; // x_max
  cubes[i].bounding_box[3] = 0.0; // y_max
  // Check all 8 corners of the cube to set bounding box values
  // The model matrix specifies where the object is positioned in the scene.
  // The view matrix specifies the relative position of the positioned object with respect to the camera.
  // The projection matrix represents the lenses and aperture of a camera, and is what actually deforms the scene in a way that it simulates perspective, making objects that are far away smaller.
  glm::mat4 model = cubes[i].cube_model;
  int gv = 0;
  glm::mat4 view = glm::lookAt(grounds[gv].ground_view_position, grounds[gv].ground_view_target, grounds[gv].ground_view_up);
  glm::mat4 modelview = view * model;
  // Make a local copy of the projection
  glm::mat4 p = update_projection(width, height);
  // The viewport specifies the size and position of your drawing area.
  glm::vec4 viewport(0.0f, 0.0f, float(width), float(height));
  for (int k=0; k < 8; ++k) {
    int kidx = 3 * k;
    int corner_idx = 4 + (2 * k);
    glm::vec3 corner = glm::vec3(cube_corners[kidx], cube_corners[kidx+1], cube_corners[kidx+2]);
    // The project function does the magic of projecting the original point to the screen:
    glm::vec3 projected = glm::project(corner, modelview, p, viewport);
    float px = projected[0];
    float py = projected[1];
    // Save all corner projections for debugging
    cubes[i].bounding_box[corner_idx] = px;
    cubes[i].bounding_box[corner_idx+1] = py;
    if (debug > 1) printf("cube_objects.cpp: i %d k %d px %4.2f py %4.2f (%4.2f, %4.2f, %4.2f)\n", i, k, px, py, cube_corners[kidx], cube_corners[kidx+1], cube_corners[kidx+2]);
    // Check for minimum and maximum x and y values
    // Check if bounds are totally outside the window, too.
    if (px < cubes[i].bounding_box[0]) cubes[i].bounding_box[0] = min(max(px, 0.0f), float(width));
    if (py < cubes[i].bounding_box[1]) cubes[i].bounding_box[1] = min(max(py, 0.0f), float(height));
    if (px > cubes[i].bounding_box[2]) cubes[i].bounding_box[2] = max(min(px, float(width)), 0.0f);
    if (py > cubes[i].bounding_box[3]) cubes[i].bounding_box[3] = max(min(py, float(height)), 0.0f);
  }
  if (debug > 1) printf("cube_objects.cpp: xmin %4.2f ymin %4.2f xmax %4.2f ymax %4.2f\n", cubes[i].bounding_box[0], cubes[i].bounding_box[1], cubes[i].bounding_box[2], cubes[i].bounding_box[3]);
}

// Cube move to target
void cube_move_to_target(int me, int target) {
    float dx = cubes[target].spatial_position.x - cubes[me].spatial_position.x;
    float dz = cubes[target].spatial_position.z - cubes[me].spatial_position.z;
    float d = sqrt(dx*dx + dz*dz);
    float angle = find_angle(dz, dx);
    //printf("cube.cpp: d %02f, x %0.2f, z %0.2f, tx %0.2f, tz %0.2f, dx %0.2f, dz %0.2f, a %0.2f(%0.2f)\n", d, cubes[me].spatial_position.x, cubes[me].spatial_position.z, cubes[target].spatial_position.x, cubes[target].spatial_position.z, dx, dz, angle, 360.0*angle/(2.0*pi));
    cubes[me].spatial_direction_active = false;
    cube_update_angle(me, angle);
    cube_update_model(me);
    cube_update_view(me);
    cube_update_mvp(me);
    cubes[me].spatial_velocity = d/10.0;
    //cube_update_waypoints(me, MOVEMENT);
}

// Update the position for a cube
void cube_update_position(int i) {

  // Check if this cube is active
  if (! cubes[i].cube_active) return;

  // printf("cube.cpp: %02d %0.2f (%0.2f, %0.2f, %0.2f)\n", i, cubes[i].spatial_velocity, cubes[i].spatial_position.x, cubes[i].spatial_position.y, cubes[i].spatial_position.z); 
  
  // See if we are in motion.
  float v = cubes[i].spatial_velocity;

  if (v > 0.0 && cubes[i].resource_energy > 0.0) {

    // Save our current position and distance in case we need to retract
    cubes[i].spatial_position_previous = cubes[i].spatial_position;
    cubes[i].spatial_distance_previous = cubes[i].spatial_distance;
    
    float x = cubes[i].spatial_position.x;
    float z = cubes[i].spatial_position.z;
    float a = cubes[i].spatial_rotation.y;
    if (cubes[i].spatial_direction_active) a = cubes[i].spatial_direction;

    int gscale = grounds[0].ground_scale_factor;
    // Check for bounds
    if (x>gscale || x<-gscale || z>gscale || z<-gscale) {
      // printf("cube.cpp: %8s %2d out of bounds - stopping.\n", &cubes[i].cube_player[0], i);
      cubes[i].spatial_velocity = 0.0;
      cubes[i].spatial_position_blocked = true;

      // Record the blocked waypoint
      cube_update_waypoints(i, BLOCKED);

      return;
    }

    // Compute unit distance moved per frame
    float udpf = 1.0 / frames_per_second;
    float d = udpf;
    
    // See if there is a distance limit on our travels
    float sd = cubes[i].spatial_distance;
    if (sd > 0.0) {
      // Move either the distance that remains or the distance per frame
      d = min(sd, udpf);
      cubes[i].spatial_distance -= d;
      // If we've travelled the distance, halt.
      if (cubes[i].spatial_distance <= 0.0) cubes[i].spatial_velocity = 0.0;
    }

    float dx = v * d * sin(a);
    float dz = v * d * cos(a);

    // Move our position
    cubes[i].spatial_position.x = x + dx;
    cubes[i].spatial_position.z = z + dz;

    // Update the translation
    cube_update_translation(i);

  } // end if (v > 0.0 && cubes[i].resource_energy > 0.0) {

  // Update the model - needed if rotation has changed, too
  cube_update_model(i);
  // Update the view
  cube_update_view(i);
  // Update the mvp
  cube_update_mvp(i);
  
}

// A contact conflict remains between two players
void cube_block_position(int i) {

  // Check if this cube is active or a resource
  if (! cubes[i].cube_active || cubes[i].cube_player == "resource") return;

  // Allow moves during an update timer
  if (cubes[i].spatial_position_timer > 0) return;
  
  // If we were in motion
  if (cubes[i].spatial_velocity > 0.0) {
    // Stop
    cubes[i].spatial_velocity = 0.0;
    // Restore previous position
    cubes[i].spatial_position = cubes[i].spatial_position_previous;
    cubes[i].spatial_distance = cubes[i].spatial_distance_previous;
    // Update the translation
    cube_update_translation(i);
    // Update the model
    cube_update_model(i);
    // Update the view
    cube_update_view(i);
    // Update the mvp
    cube_update_mvp(i);
    // Set the blocked flag
    cubes[i].spatial_position_blocked = true;
    // Record the blocked waypoint
    cube_update_waypoints(i, BLOCKED);

    // printf("cube.cpp New move blocked for %d\n", i);
  }
}

// Execute a remote control move
int cube_remote_move(int i, float angle, float direction, bool direction_active, float distance, float velocity,  glm::vec2 gaze) {

  // Check if this cube is active
  if (! cubes[i].cube_active) return -1;

  // This is the cube orientation
  cube_update_angle(i, angle);
  
  // This is the optional direction of travel
  cubes[i].spatial_direction = direction;
  cubes[i].spatial_direction_active = direction_active;

  // Set the distance to travel
  cubes[i].spatial_distance = distance;

  // Set the velocity to what was requested or the limit
  if (velocity <= VELOCITY_MAX) {
    cubes[i].spatial_velocity = velocity;
  } else {
    cubes[i].spatial_velocity = VELOCITY_MAX;
  }

  // Update our gaze as requested
  cubes[i].spatial_gaze = gaze;

  // Record the starting waypoint
  cube_update_waypoints(i, MOVEMENT);

  // Now make our move
  cube_update_position(i);
  
  return 0;
}


// Update the waypoint list
void cube_update_waypoints(int i, int waypoint_event) {

  spatial_waypoint w;

  w.seconds = time(NULL);
  w.spatial_event = waypoint_event;
  w.spatial_position = cubes[i].spatial_position;
  w.spatial_position_blocked = cubes[i].spatial_position_blocked;
  cubes[i].spatial_waypoints.push_back(w);
  
}


// Convert cube uuid to a cube index
int cube_get_index(string uuid) {
  for (int i=0; i<n_cubes; ++i) {
    if (cubes[i].cube_uuid == uuid) return i;
  }
  return -1;
}

// Convert ground uuid to a ground index
int ground_get_index(string uuid) {
  for (int i=0; i<n_grounds; ++i) {
    if (grounds[i].ground_uuid == uuid) return i;
  }
  return -1;
}

// Compute total points for a cube
void cube_total_points(int i, float *total, float *mate, float *food, float *kill) {

  float total_points = 0.0;
  float mate_points = 0.0;
  float food_points = 0.0;
  float kill_points = 0.0;
  if (cubes[i].cube_player != "resource") {
    // Compute game points
    for (int m=0; m<cubes[i].life_energy.size(); ++m) mate_points += cubes[i].life_energy[m];
    for (int f=0; f<cubes[i].resource_capture.size(); ++f) food_points += cubes[i].resource_capture[f];
    for (int k=0; k<cubes[i].match_energy.size(); ++k) kill_points += cubes[i].match_energy[k];
    total_points = mate_points + kill_points + food_points;
  }
  *total = total_points;
  *mate = mate_points;
  *food = food_points;
  *kill = kill_points;
  return;
}
    
// Get status on a cube for a remote client
StatusResponse cube_status(string uuid) {

  StatusResponse s;
  
  s.cube_index = cube_get_index(uuid);
  if (s.cube_index < 0) return s;

  s.cube_remote = cubes[s.cube_index].cube_remote;
  if (! s.cube_remote) return s;
  
  int i = s.cube_index;

  s.cube_player = cubes[i].cube_player;
  s.cube_firstname = cubes[i].cube_firstname;
  s.cube_active = cubes[i].cube_active;
  s.cube_display = cubes[i].cube_display;
  s.cube_remote = cubes[i].cube_remote;
  s.cube_scale_factor = cubes[i].cube_scale_factor;
  s.spatial_angle = cubes[i].spatial_rotation.y;
  s.spatial_direction = cubes[i].spatial_direction;
  s.spatial_direction_active = cubes[i].spatial_direction_active;
  s.spatial_gaze = cubes[i].spatial_gaze;
  s.spatial_radius = cubes[i].spatial_radius;
  s.spatial_distance = cubes[i].spatial_distance;
  s.spatial_distance_previous = cubes[i].spatial_distance_previous;
  s.spatial_velocity = cubes[i].spatial_velocity;
  s.spatial_position_blocked = cubes[i].spatial_position_blocked;
  s.life_birth = cubes[i].life_birth;
  s.life_death = cubes[i].life_death;
  s.life_father = cubes[i].life_father;
  s.life_mother = cubes[i].life_mother;
  s.resource_energy = cubes[i].resource_energy;

  cube_total_points(i, &s.total_points[0], &s.total_points[1], &s.total_points[2], &s.total_points[3]);

  return s;
}

// Check if we have enough energy to continue
void cube_energy_check(int i) {

  // We burn energy by just staying in the game, even if not moving.
  if (cubes[i].cube_active && cubes[i].cube_player != "resource") {
    cubes[i].resource_energy = max(cubes[i].resource_energy -  (ENERGY_COST * cubes[i].cube_scale_factor / (float)frames_per_second), 0.0f);
    // printf("cube.cpp: Burning energy for player %d now at %0.2f\n", i, cubes[i].resource_energy);
    if (cubes[i].resource_energy <= 0.0) {
      cubes[i].cube_active = false;
      cubes[i].life_death = time(NULL);
      cubes[i].cube_display = false;
      printf("cube.cpp: %8s %2d out of energy.\n", &cubes[i].cube_player[0], i);
      return;
    }
  }
}

// Update energy used for motion activity
void cube_energy_cost(int i) {

  if (! cubes[i].cube_active) return;
  
  float v = cubes[i].spatial_velocity;
  
  // Account for energy used
  cubes[i].resource_energy -= v * ENERGY_FACTOR / (float)frames_per_second;
    if (cubes[i].resource_energy <= 0.0) {
      cubes[i].cube_active = false;
      cubes[i].life_death = time(NULL);
      cubes[i].cube_display = false;
      cubes[i].spatial_velocity = 0.0;
      printf("cube.cpp: %8s %2d ran out of energy\n", &cubes[i].cube_player[0], i);
    }
}

/* Display a wireframe cube */
int new_wire(int nw, float wire_scale_factor, float x, float y, float z, float rotX, float rotY, float rotZ, int type, glm::vec4 color)
{

  GLfloat wire_vectors[] =
    {
     -1.0, -1.0, 1.0,
     1.0, -1.0, 1.0,
     1.0, -1.0, 1.0,
     1.0, 1.0, 1.0,
     1.0, 1.0, 1.0,
     -1.0, 1.0, 1.0,
     -1.0, 1.0, 1.0,
     -1.0, -1.0, 1.0,

     -1.0, -1.0, -1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0, -1.0,
     1.0, 1.0, -1.0,
     1.0, 1.0, -1.0,
     -1.0, 1.0, -1.0,
     -1.0, 1.0, -1.0,
     -1.0, -1.0, -1.0,

     -1.0, -1.0, -1.0,
     -1.0, -1.0, 1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0, 1.0,
     1.0, 1.0, -1.0,
     1.0, 1.0, 1.0,
     -1.0, 1.0, -1.0,
     -1.0, 1.0, 1.0
     
    };

  /* type 0 = wireframe, type 1 = fill, type 2 = textured */
  wires[nw].wire_type = type;
  /* Set the drawing color if needed */
  wires[nw].wire_color = color;

  /* Set the scale factor */
  wires[nw].wire_scale = glm::scale(glm::mat4(1.0f), glm::vec3(wire_scale_factor, wire_scale_factor, wire_scale_factor));
  
  wires[nw].wire_rotation = 
    glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0)) *  // X axis
    glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0)) *  // Y axis
    glm::rotate(glm::mat4(1.0f), rotZ, glm::vec3(0, 0, 1));   // Z axis

  wires[nw].wire_translation = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
  wires[nw].wire_model = wires[nw].wire_translation * wires[nw].wire_rotation * wires[nw].wire_scale;
  wires[nw].wire_mvp = projection * view * wires[nw].wire_model;
  
  glGenBuffers(1, &wires[nw].vbo_wire_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, wires[nw].vbo_wire_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(wire_vectors), wire_vectors, GL_STATIC_DRAW);
  fprintf(stdout, "cube.cpp: new_wire %d vbo_wire_vertices %d (%d)\n", nw, wires[nw].vbo_wire_vertices, int(sizeof(wires[nw].vbo_wire_vertices)));

  return 0;
}

/* Create the ground plane */
int new_ground(int ng, string ground_uuid, GLfloat ground_scale_factor, GLuint ground_type, glm::vec4 ground_color, GLuint ground_material, GLuint ground_texture_index, float ground_texture_map[8], float x, float y, float z, float rotX, float rotY, float rotZ, float viewX, float viewY, float viewZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ)
{
  GLfloat ground_vertices[] =
    {
    // groundplane (x,y,z)
     1.0,  0.0, -1.0,
    -1.0,  0.0, -1.0,
    -1.0,  0.0,  1.0,
     1.0,  0.0,  1.0,
  };

  GLfloat ground_vertices_normals[] =
    {
    // groundplane (x,y,z,xn,yn,zn)
     1.0,  0.0, -1.0, 0.0, 1.0, 0.0,
    -1.0,  0.0, -1.0, 0.0, 1.0, 0.0,
    -1.0,  0.0,  1.0, 0.0, 1.0, 0.0,
     1.0,  0.0,  1.0, 0.0, 1.0, 0.0
  };

  glGenBuffers(1, &grounds[ng].vbo_ground_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, grounds[ng].vbo_ground_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);
  fprintf(stdout, "cube.cpp: ground vbo_ground_vertices %d (%d), ", grounds[ng].vbo_ground_vertices, int(sizeof(ground_vertices)));
  
  GLfloat ground_normals[] =
    {
     0.0, 1.0, 0.0,
     0.0, 1.0, 0.0,
     0.0, 1.0, 0.0,
     0.0, 1.0, 0.0
    };

  glGenBuffers(1, &grounds[ng].vbo_ground_normals);
  glBindBuffer(GL_ARRAY_BUFFER, grounds[ng].vbo_ground_normals);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ground_normals), ground_normals, GL_STATIC_DRAW);
  fprintf(stdout, "vbo_ground_normals %d (%d), ", grounds[ng].vbo_ground_normals, int(sizeof(ground_normals)));
  
  GLfloat ground_texture_map_coords[1*4*2];
  
  for ( int n = 0; n < 1*4*2; ++n) { ground_texture_map_coords[n] = ground_texture_map[n]; }
  
  glGenBuffers(1, &grounds[ng].vbo_ground_texture_map_coords);
  glBindBuffer(GL_ARRAY_BUFFER, grounds[ng].vbo_ground_texture_map_coords);
  glBufferData(GL_ARRAY_BUFFER, sizeof(ground_texture_map_coords), ground_texture_map_coords, GL_STATIC_DRAW);
  fprintf(stdout, "vbo_ground_texture_map_coords %d (%d), ", grounds[ng].vbo_ground_texture_map_coords, int(sizeof(ground_texture_map_coords)));

  GLushort ground_elements[] = {
    // groundplane
    0,  1,  2,
    2,  3,  0,
  };

  glGenBuffers(1, &grounds[ng].ibo_ground_elements);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grounds[ng].ibo_ground_elements);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_elements), ground_elements, GL_STATIC_DRAW);
  fprintf(stdout, "ibo_ground_elements %d (%d)\n", grounds[ng].ibo_ground_elements, int(sizeof(ground_elements)));

  /* Save ground uuid */
  grounds[ng].ground_uuid = ground_uuid;
  
  /* Setup the ground scale */

  grounds[ng].ground_scale_factor = ground_scale_factor;
  grounds[ng].ground_scale = glm::scale(glm::mat4(1.0f), glm::vec3(ground_scale_factor, ground_scale_factor, ground_scale_factor));

  /* Save the type to use for the ground plane */
  grounds[ng].ground_type = ground_type;

  /* Save the color to use for the ground plane */
  grounds[ng].ground_color = ground_color;

  /* Save the material properties describing the ground plane */
  grounds[ng].ground_material = ground_material;

  /* Save the texture to use for the ground plane */
  grounds[ng].ground_texture_index = ground_texture_index;

  /* Setup all the spatial coordinates */
  grounds[ng].ground_spatial_position = glm::vec3(x, y, z);
  grounds[ng].ground_spatial_translation = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

  grounds[ng].ground_spatial_rotation = glm::vec3(rotX, rotY, rotZ);
  grounds[ng].ground_spatial_rotation_matrix = 
    glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0)) *  // X axis
    glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0)) *  // Y axis
    glm::rotate(glm::mat4(1.0f), rotZ, glm::vec3(0, 0, 1));   // Z axis
  
  /* Setup this ground's view. */

  grounds[ng].ground_view_position = glm::vec3(viewX, viewY, viewZ);
  grounds[ng].ground_view_target = glm::vec3(targetX, targetY, targetZ);
  grounds[ng].ground_view_up = glm::vec3(upX, upY, upZ);
  
  // Update the model
  ground_update_model(ng);
  // Update the mvp
  ground_update_mvp(ng);

  // Set active on
  grounds[ng].ground_active = true;
  
  // Set display on
  grounds[ng].ground_display = true;
  
  // Set remote view off
  grounds[ng].ground_remote = false;
  
  return 0;

}

// Update the model far a ground
void ground_update_model(int i) {
  grounds[i].ground_model = grounds[i].ground_spatial_translation * grounds[i].ground_spatial_rotation_matrix * grounds[i].ground_scale;
}

// Update the mvp for a ground
void ground_update_mvp(int i) {
  grounds[i].ground_mvp = projection * view * grounds[i].ground_model;
}

/* Add another light source */
int new_light(int light_index, glm::vec3 position, glm::vec3 intensity, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
{
  
  lights[light_index].position = position;
  lights[light_index].intensity = intensity;
  lights[light_index].ambient = ambient;
  lights[light_index].diffuse = diffuse;
  lights[light_index].specular = specular;
  lights[light_index].constant = constant;
  lights[light_index].linear = linear;
  lights[light_index].quadratic = quadratic;
  
  return 0;

}

/* Add a new material */
int new_material(int material_index, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
{
  materials[material_index].ambient = ambient;
  materials[material_index].diffuse = diffuse;
  materials[material_index].specular = specular;
  materials[material_index].shininess = shininess;
  
  return 0;

}

/* Organize the textures to be used */
int new_texture(int index, const char *filename)
{
  unsigned char* texture_data;
  
  glGenTextures(1, &textures[index].texture_array);

  /* Load the texture for the cube */
  texture_data = stbi_load(filename, &textures[index].width, &textures[index].height, &textures[index].bytes_per_pixel, 0);
  fprintf(stdout, "cube.cpp: new_texture loaded image %s with width %d, height %d, channels %d\n", filename, textures[index].width, textures[index].height, textures[index].bytes_per_pixel);

  GLuint texture_internal_format = transparency ? GL_RGBA : GL_RGB;
  textures[index].format = textures[index].bytes_per_pixel==4 ? GL_RGBA : GL_RGB;
  
  /* Create the texture for the cube */
  glActiveTexture(GL_TEXTURE0+index);
  glBindTexture(GL_TEXTURE_2D, textures[index].texture_array);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, // target
	       0,  // level, 0 = base, no mipimap,
	       texture_internal_format, // internalformat
	       textures[index].width,  // width
	       textures[index].height,  // height
	       0,  // border, always 0 in OpenGL ES
	       textures[index].format,  // format
	       GL_UNSIGNED_BYTE, // type
	       texture_data);

  stbi_image_free(texture_data);
  
  return 0;

}

// Generate a home-grown UUID
string get_uuid() {
    static random_device dev;
    static mt19937 rng(dev());

    uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

