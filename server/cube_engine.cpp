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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

/* Common shader programs */
#include "shader_utils.h"

/* Function prototypes for this program */
# include "cube.h"

// Render and draw function
void display()
{
  int size;
  
  /* Blank the window */
  glClearColor(window_background_color_r, window_background_color_g, window_background_color_b, window_background_color_a);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  // Scale normals for correct lighting.
  // Adds extra processing time and steps
  // glEnable(GL_NORMALIZE);
 
  /* The compiled shader program */
  glUseProgram(program);
  
  if (debug > 1) printf("cube_engine.cpp: Attributes - vertices %u normals %u textures %u\n", attribute_coord3d, attribute_normal, attribute_coordtexture); 

  /* Setup uniforms for the point lights */
  glUniform1i(uniform_n_lights, n_lights);
  for ( int i = 0; i < n_lights; ++i ) {
    glUniform3fv(uniform_lights[i].position, 1, glm::value_ptr(lights[i].position));
    glUniform3fv(uniform_lights[i].intensity, 1, glm::value_ptr(lights[i].intensity));
    glUniform3fv(uniform_lights[i].ambient, 1, glm::value_ptr(lights[i].ambient));
    glUniform3fv(uniform_lights[i].diffuse, 1, glm::value_ptr(lights[i].diffuse));
    glUniform1f(uniform_lights[i].constant, lights[i].constant);
    glUniform1f(uniform_lights[i].linear, lights[i].linear);
    glUniform1f(uniform_lights[i].quadratic, lights[i].quadratic);
    if (debug > 1) debug_lights(i);
  }

  /* The view position here is the same as the camera position */
  glUniform3fv(uniform_view_position, 1, glm::value_ptr(camera_position));

  /* Iterate over all the cubes and draw them all */
  if (display_cubes) {
    for (int cube_index = 0; cube_index < n_cubes; ++cube_index) {

      /* Check if this cube needs to be displayed */
      if (! cubes[cube_index].cube_display) continue;

      if (debug > 2) {
	// This is a check on the normals computation done in the shader programs.
	for (int nf=0; nf<6; ++nf) {
	  int normal_index = 12 * nf;
	  glm::vec3 normal_original = glm::vec3(cube_normals[normal_index], cube_normals[normal_index+1], cube_normals[normal_index+2]);
	  float cube_scale_factor = cubes[cube_index].cube_scale_factor;
	  glm::mat4 ns = glm::scale(glm::mat4(1.0f), glm::vec3(cube_scale_factor, cube_scale_factor, cube_scale_factor));
	  glm::vec3 n4 = glm::mat3(cubes[cube_index].cube_model) * normal_original;
	  glm::vec3 n = normalize(glm::vec3(n4));
	  printf("cube_engine.cpp: cube %d face %d n.x %5.2f, n.y %5.2f, n.z %5.2f, normal.x %5.2f, normal.y %5.2f, normal.z %5.2f\n", cube_index, nf, normal_original.x, normal_original.y, normal_original.z, n.x, n.y, n.z); 
	  printf("cube_engine.cpp: cube %d face %d ns\n%s\n", cube_index, nf, &glm::to_string(ns)[0]); 
	  printf("cube_engine.cpp: cube %d face %d n4 %s\n", cube_index, nf, &glm::to_string(n4)[0]); 
	}
      }
      
      /* Setup uniforms needed by the shaders */
      glUniform1i(uniform_object_type, cubes[cube_index].cube_type);
      glUniform4fv(uniform_object_color, 1, glm::value_ptr(cubes[cube_index].cube_color));
      glUniform1i(uniform_object_texture, /* GL_TEXTURE */ cubes[cube_index].cube_texture_index);
      glUniform3fv(uniform_material_ambient, 1, glm::value_ptr(materials[cubes[cube_index].cube_material].ambient));
      glUniform3fv(uniform_material_diffuse, 1, glm::value_ptr(materials[cubes[cube_index].cube_material].diffuse));
      glUniform3fv(uniform_material_specular, 1, glm::value_ptr(materials[cubes[cube_index].cube_material].specular));
      glUniform1f(uniform_material_shininess, materials[cubes[cube_index].cube_material].shininess);
      glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(cubes[cube_index].cube_model));
      glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(cubes[cube_index].cube_mvp));

      /* Different actions for different types of cubes */
      if (debug > 1) printf("cube_engine.cpp: Displaying cube %d type %d\n", cube_index, cubes[cube_index].cube_type); 
      switch (cubes[cube_index].cube_type) {
      case 0:
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	break;
      case 1:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case 2:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, textures[cubes[cube_index].cube_texture_index].texture_array);
	break;
      case 3:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, textures[cubes[cube_index].cube_texture_index].texture_array);
	break;
      case 4:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, textures[cubes[cube_index].cube_texture_index].texture_array);
	break;
      case 5:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, textures[cubes[cube_index].cube_texture_index].texture_array);
	break;
      default:
	fprintf(stderr, "cube.cpp: Invalid cube type %d\n", cubes[cube_index].cube_type);
	return;
      }

      // Describe our vertices array to OpenGL (it can't guess its format automatically)
      glBindBuffer(GL_ARRAY_BUFFER, cubes[cube_index].vbo_cube_vertices);
      // Enable attribute_coord3d
      glEnableVertexAttribArray(attribute_coord3d);
      glVertexAttribPointer
	(
	 attribute_coord3d, // attribute
	 //6,                 // number of elements per vertex, here (x,y,z,xn,yn,zn)
	 3,                 // number of elements per vertex, here (x,y,z)
	 GL_FLOAT,          // the type of each element
	 GL_FALSE,          // take our values as-is
	 0,                 // no extra data between each position
	 0                  // offset of first element
	);
      
      // Describe our normals array to OpenGL (it can't guess its format automatically)
      glBindBuffer(GL_ARRAY_BUFFER, cubes[cube_index].vbo_cube_normals);
      // Enable attribute_normal
      glEnableVertexAttribArray(attribute_normal);
      glVertexAttribPointer
	(
	 attribute_normal,  // attribute
	 3,                 // number of elements per vertex, here (xn,yn,zn)
	 GL_FLOAT,          // the type of each element
	 GL_FALSE,          // take our values as-is
	 0,                 // no extra data between each position
	 0                  // offset of first element
	);

      // Describe our texture
      glBindBuffer(GL_ARRAY_BUFFER, cubes[cube_index].vbo_cube_texture_map_coords);
      // Enable attribute_texcoord
      glEnableVertexAttribArray(attribute_coordtexture);
      glVertexAttribPointer
	(
	 attribute_coordtexture, // attribute
	 2,                  // number of elements per vertex, here (x,y)
	 GL_FLOAT,           // the type of each element
	 GL_FALSE,           // take our values as-is
	 0,                  // no extra data between each position
	 0                   // offset of first element
	);

      if (debug > 1) debug_materials(cubes[cube_index].cube_material);
      if (debug > 1) debug_cubes(cube_index);

      /* Push each element in cube buffer_vertices to the vertex shader */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubes[cube_index].ibo_cube_elements);
      glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
      glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

      /* Different cleanups for different types of cubes */
      switch (cubes[cube_index].cube_type) {
      case 0:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case 1:
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	break;
      case 2:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, 0);
	break;
      case 3:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, 0);
	break;
      case 4:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, 0);
	break;
      case 5:
	glActiveTexture(GL_TEXTURE0+cubes[cube_index].cube_texture_index);
	glBindTexture(GL_TEXTURE_2D, 0);
	break;
      default:
	fprintf(stderr, "cube.cpp: Invalid cleanup of cube index %d\n", cubes[cube_index].cube_type);
	return;
      }

      glDisableVertexAttribArray(attribute_coord3d);
      glDisableVertexAttribArray(attribute_normal);
      glDisableVertexAttribArray(attribute_coordtexture);

    } /* end for(cube_index ...) */
  } /* end if(display_cubes) ... */

  if (display_wires) {
    for (int wire_index = 0; wire_index < n_wires; ++wire_index) {
      // This is the drawing style
      glUniform1i(uniform_object_type, wires[wire_index].wire_type);
      // Setup object color
      glUniform4fv(uniform_object_color, 1, glm::value_ptr(wires[wire_index].wire_color));

      glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(wires[wire_index].wire_model));
      glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(wires[wire_index].wire_mvp));

      // Describe our vertices array to OpenGL (it can't guess its format automatically)
      glBindBuffer(GL_ARRAY_BUFFER, wires[wire_index].vbo_wire_vertices);
      // Enable attribute_coord3d
      glEnableVertexAttribArray(attribute_coord3d);
      glVertexAttribPointer
	(
	 attribute_coord3d, // attribute
	 3,                 // number of elements per vertex, here (x,y,z)
	 GL_FLOAT,          // the type of each element
	 GL_FALSE,          // take our values as-is
	 0,                 // no extra data between each position
	 0                  // offset of first element
	);

      /* Push each vector in the normals to the vertex shader */
      glBindBuffer(GL_BUFFER, wires[wire_index].vbo_wire_vertices);
      glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
      glDrawArrays(GL_LINES, 0, 24);

      glDisableVertexAttribArray(attribute_coord3d);

    } /* end for(wire_index ...) */
  } /* if (display_wires) { ... */

  if (display_normals) {
    // This is the drawing style
    glUniform1i(uniform_object_type, 0);
    // Setup object color
    glUniform4fv(uniform_object_color, 1, glm::value_ptr(color_black));

    glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(normals_model));
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(normals_mvp));

    // Describe our vertices array to OpenGL (it can't guess its format automatically)
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    // Enable attribute_coord3d
    glEnableVertexAttribArray(attribute_coord3d);
    glVertexAttribPointer
      (
       attribute_coord3d, // attribute
       3,                 // number of elements per vertex, here (x,y,z)
       GL_FLOAT,          // the type of each element
       GL_FALSE,          // take our values as-is
       0,                 // no extra data between each position
       0                  // offset of first element
      );

    /* Push each vector in the normals to the vertex shader */
    glBindBuffer(GL_BUFFER, vbo_normals);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawArrays(GL_LINES, 0, 12);

    glDisableVertexAttribArray(attribute_coord3d);

  } // if (display_normals) { ...

  /* Draw the ground plane here */
  if (display_grounds) {
    for (int ground_index = 0; ground_index < n_grounds; ++ground_index) {

      // This is the drawing style
      glUniform1i(uniform_object_type, grounds[ground_index].ground_type);
      // Setup object color
      glUniform4fv(uniform_object_color, 1, glm::value_ptr(grounds[ground_index].ground_color));
      // Use the ground plane texture
      glUniform1i(uniform_object_texture, /* GL_TEXTURE */ grounds[ground_index].ground_texture_index);
      // Setup our material
      glUniform3fv(uniform_material_ambient, 1, glm::value_ptr(materials[grounds[ground_index].ground_material].ambient));
      glUniform3fv(uniform_material_diffuse, 1, glm::value_ptr(materials[grounds[ground_index].ground_material].diffuse));
      glUniform3fv(uniform_material_specular, 1, glm::value_ptr(materials[grounds[ground_index].ground_material].specular));
      glUniform1f(uniform_material_shininess, materials[grounds[ground_index].ground_material].shininess);

      glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(grounds[ground_index].ground_model));
      glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(grounds[ground_index].ground_mvp));

      glActiveTexture(GL_TEXTURE0+grounds[ground_index].ground_texture_index);
      glBindTexture(GL_TEXTURE_2D, textures[grounds[ground_index].ground_texture_index].texture_array);

      // Describe our vertices array to OpenGL (it can't guess its format automatically)
      glBindBuffer(GL_ARRAY_BUFFER, grounds[ground_index].vbo_ground_vertices);
      // Enable attribute_coord3d
      glEnableVertexAttribArray(attribute_coord3d);
      glVertexAttribPointer
	(
	 attribute_coord3d, // attribute
	 3,                 // number of elements per vertex, here (x,y,z)
	 GL_FLOAT,          // the type of each element
	 GL_FALSE,          // take our values as-is
	 0,                 // no extra data between each position
	 0                  // offset of first element
	);

      // Describe our normals array to OpenGL (it can't guess its format automatically)
      glBindBuffer(GL_ARRAY_BUFFER, grounds[ground_index].vbo_ground_normals);
      // Enable attribute_normal
      glEnableVertexAttribArray(attribute_normal);
      glVertexAttribPointer
	(
	 attribute_normal,  // attribute
	 3,                 // number of elements per vertex, here (x,y,z)
	 GL_FLOAT,          // the type of each element
	 GL_FALSE,          // take our values as-is
	 0,                 // no extra data between each position
	 0                  // offset of first element
	);

      // Describe our texture
      glBindBuffer(GL_ARRAY_BUFFER, grounds[ground_index].vbo_ground_texture_map_coords);
      // Enable attribute_coordtexture
      glEnableVertexAttribArray(attribute_coordtexture);
      glVertexAttribPointer
	(
	 attribute_coordtexture, // attribute
	 2,                  // number of elements per vertex, here (x,y)
	 GL_FLOAT,           // the type of each element
	 GL_FALSE,           // take our values as-is
	 0,                  // no extra data between each position
	 0                   // offset of first element
	);

      if (debug > 1) debug_grounds(ground_index);

      /* Push each element in ground buffer_vertices to the vertex shader */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grounds[ground_index].ibo_ground_elements);
      glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
      glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

      glDisableVertexAttribArray(attribute_coord3d);
      glDisableVertexAttribArray(attribute_normal);
      glDisableVertexAttribArray(attribute_coordtexture);

    } /* end for(ground_index ... ) */
  } /* if (display_grounds) { ... */

  // glfw: swap buffers
  glfwSwapBuffers(window);

} // end display() ...

/* Print out a lot of scene information for debug purposes */
void debug_lights(int light_index)
{
  fprintf(stdout, "cube_engine.cpp: Light[%d] pos %4.2f %4.2f %4.2f\n", light_index, lights[light_index].position.x, lights[light_index].position.y, lights[light_index].position.z);
  fprintf(stdout, "cube_engine.cpp: Light[%d] int %4.2f %4.2f %4.2f\n", light_index, lights[light_index].intensity.x, lights[light_index].intensity.y, lights[light_index].intensity.z);
  fprintf(stdout, "cube_engine.cpp: Light[%d] amb %4.2f %4.2f %4.2f\n", light_index, lights[light_index].ambient.x, lights[light_index].ambient.y, lights[light_index].ambient.z);
  fprintf(stdout, "cube_engine.cpp: Light[%d] dif %4.2f %4.2f %4.2f\n", light_index, lights[light_index].diffuse.x, lights[light_index].diffuse.y, lights[light_index].diffuse.z);
  fprintf(stdout, "cube_engine.cpp: Light[%d] spe %4.2f %4.2f %4.2f\n", light_index, lights[light_index].specular.x, lights[light_index].specular.y, lights[light_index].specular.z);
  fprintf(stdout, "cube_engine.cpp: Light[%d] con %4.2f lin %4.2f quad %4.2f\n", light_index, lights[light_index].constant, lights[light_index].linear, lights[light_index].quadratic);
  return;
}

void debug_materials(int material_index) {
  fprintf(stdout, "cube_engine.cpp: Material[%d] amb %4.2f %4.2f %4.2f\n", material_index, materials[material_index].ambient.x, materials[material_index].ambient.y, materials[material_index].ambient.z);
  fprintf(stdout, "cube_engine.cpp: Material[%d] dif %4.2f %4.2f %4.2f\n", material_index, materials[material_index].diffuse.x, materials[material_index].diffuse.y, materials[material_index].diffuse.z);
  fprintf(stdout, "cube_engine.cpp: Material[%d] spe %4.2f %4.2f %4.2f\n", material_index, materials[material_index].specular.x, materials[material_index].specular.y, materials[material_index].specular.z);
  fprintf(stdout, "cube_engine.cpp: Material[%d] shi %4.2f\n", material_index, materials[material_index].shininess);
  return;
}

void debug_cubes(int cube_index)
{
  fprintf(stdout, "cube_engine.cpp: Cube type=%d, texture=%d, color=(%4.2f, %4.2f, %4.2f, %4.2f)\n", cubes[cube_index].cube_type, cubes[cube_index].cube_texture_index, cubes[cube_index].cube_color.x, cubes[cube_index].cube_color.y, cubes[cube_index].cube_color.z, cubes[cube_index].cube_color.w);
 
  fprintf(stdout, "cube_engine.cpp: Model[0] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_model[0].x, cubes[cube_index].cube_model[0].y, cubes[cube_index].cube_model[0].z, cubes[cube_index].cube_model[0].w);;
  fprintf(stdout, "cube_engine.cpp: Model[1] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_model[1].x, cubes[cube_index].cube_model[1].y, cubes[cube_index].cube_model[1].z, cubes[cube_index].cube_model[1].w);;
  fprintf(stdout, "cube_engine.cpp: Model[2] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_model[2].x, cubes[cube_index].cube_model[2].y, cubes[cube_index].cube_model[2].z, cubes[cube_index].cube_model[2].w);;
  fprintf(stdout, "cube_engine.cpp: Model[3] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_model[3].x, cubes[cube_index].cube_model[3].y, cubes[cube_index].cube_model[3].z, cubes[cube_index].cube_model[3].w);;
  fprintf(stdout, "cube_engine.cpp: MVP[0] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_mvp[0].x, cubes[cube_index].cube_mvp[0].y, cubes[cube_index].cube_mvp[0].z, cubes[cube_index].cube_mvp[0].w);;
  fprintf(stdout, "cube_engine.cpp: MVP[1] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_mvp[1].x, cubes[cube_index].cube_mvp[1].y, cubes[cube_index].cube_mvp[1].z, cubes[cube_index].cube_mvp[1].w);;
  fprintf(stdout, "cube_engine.cpp: MVP[2] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_mvp[2].x, cubes[cube_index].cube_mvp[2].y, cubes[cube_index].cube_mvp[2].z, cubes[cube_index].cube_mvp[2].w);;
  fprintf(stdout, "cube_engine.cpp: MVP[3] x %4.2f, y %4.2f, z %4.2f, w %4.2f\n", cubes[cube_index].cube_mvp[3].x, cubes[cube_index].cube_mvp[3].y, cubes[cube_index].cube_mvp[3].z, cubes[cube_index].cube_mvp[3].w);;
  return;
}

void debug_grounds(int ground_index)
{  fprintf(stdout, "cube_engine.cpp: Ground index %d, scale factor %4.2f, type %d, material %d, texture %d\n", ground_index, grounds[ground_index].ground_scale_factor, grounds[ground_index].ground_type, grounds[ground_index].ground_material, grounds[ground_index].ground_texture_index);
  return;
}


