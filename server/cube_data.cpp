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

/* JSON Library */
#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
using namespace rapidjson;

/* Function prototypes for this program */
# include "cube.h"
# include "cube_client.h"

int json_import_document();
Document d;

struct jsonHandler {
    bool Null() { cout << "Null()" << endl; return true; }
    bool Bool(bool b) { cout << "Bool(" << boolalpha << b << ")" << endl; return true; }
    bool Int(int i) { cout << "Int(" << i << ")" << endl; return true; }
    bool Uint(unsigned u) { cout << "Uint(" << u << ")" << endl; return true; }
    bool Int64(int64_t i) { cout << "Int64(" << i << ")" << endl; return true; }
    bool Uint64(uint64_t u) { cout << "Uint64(" << u << ")" << endl; return true; }
    bool Double(double d) { cout << "Double(" << d << ")" << endl; return true; }
    bool RawNumber(const char* str, SizeType length, bool copy) { 
        cout << "Number(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool String(const char* str, SizeType length, bool copy) { 
        cout << "String(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool StartObject() { cout << "StartObject()" << endl; return true; }
    bool Key(const char* str, SizeType length, bool copy) {
        cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
        return true;
    }
    bool EndObject(SizeType memberCount) { cout << "EndObject(" << memberCount << ")" << endl; return true; }
    bool StartArray() { cout << "StartArray()" << endl; return true; }
    bool EndArray(SizeType elementCount) { cout << "EndArray(" << elementCount << ")" << endl; return true; }
};

/* Import scene objects and parameters */

// Argument is a filename of a JSON file
int json_import(char* jsonfile) {
  fprintf(stdout, "cube_data.cpp: Importing JSON file %s\n", jsonfile);

  // Document d;
  ifstream jf(jsonfile);
  IStreamWrapper jfw(jf);
  d.ParseStream(jfw);
  return json_import_document();

}

// Argument is a JSON character string
int json_import(string jsonobject) {
  fprintf(stdout, "cube_data.cpp: Importing JSON object -- %s\n", jsonobject.c_str());

  // Document d;
  d.Parse(jsonobject.c_str());
  return json_import_document();

}

/* Process a JSON document */
int json_import_document() {
    
  if (d.HasMember("dataset")) {
    const char* dataset = d["dataset"].GetString();
    fprintf(stdout, "cube_data.cpp: Dataset %s\n", dataset);
  }

  if (d.HasMember("window")) {
    const Value& w = d["window"];
    window_title = w["window_title"].GetString();
    main_window_width = w["main_window_width"].GetInt();
    main_window_height = w["main_window_height"].GetInt();
    main_window_channels = w["main_window_channels"].GetInt();
    window_background_color_r = (float)w["window_background_color"][0].GetDouble();
    window_background_color_g = (float)w["window_background_color"][1].GetDouble();
    window_background_color_b = (float)w["window_background_color"][2].GetDouble();
    window_background_color_a = (float)w["window_background_color"][3].GetDouble();

    glfwSetWindowTitle(window, window_title);
    glfwSetWindowSize(window, main_window_width, main_window_height);
    projection = update_projection(main_window_width, main_window_height);
    glViewport(0, 0, main_window_width, main_window_height);
    
    fprintf(stdout, "cube_data.cpp: Title %s, main window width %d, height %d, window color (%4.2f, %4.2f, %4.2f, %4.2f)\n", window_title, main_window_width, main_window_height, window_background_color_r, window_background_color_g, window_background_color_b, window_background_color_a);
  }

  if (d.HasMember("camera")) {
    const Value& c = d["camera"];
    camera_position_default.x = (float)c["camera_position"][0].GetDouble();
    camera_position_default.y = (float)c["camera_position"][1].GetDouble();
    camera_position_default.z = (float)c["camera_position"][2].GetDouble();
    camera_target_default.x = (float)c["camera_target"][0].GetDouble();
    camera_target_default.y = (float)c["camera_target"][1].GetDouble();
    camera_target_default.z = (float)c["camera_target"][2].GetDouble();
    camera_up_default.x = (float)c["camera_up"][0].GetDouble();
    camera_up_default.y = (float)c["camera_up"][1].GetDouble();
    camera_up_default.z = (float)c["camera_up"][2].GetDouble();

    /* Set the current camera position and view */
    camera_position = camera_position_default;
    camera_target = camera_target_default;
    camera_up = camera_up_default;
    
    /* Setup this ground's view. */

    if (n_grounds > 0) {
      grounds[0].ground_view_position = glm::vec3(camera_position_default.x, camera_position_default.y, camera_position_default.z);
      grounds[0].ground_view_target = glm::vec3(camera_target_default.x, camera_target_default.y, camera_target_default.z);
      grounds[0].ground_view_up = glm::vec3(camera_up_default.x, camera_up_default.y, camera_up_default.z);
      
      /* Setup the new view */
      view = glm::lookAt(camera_position, camera_target, camera_up);

      // Update the mvp
      ground_update_mvp(0);
    }
    
    fprintf(stdout, "cube_data.cpp: Camera position x %4.2f, y %4.2f, z %4.2f\n", camera_position.x, camera_position.y, camera_position.z); 
    fprintf(stdout, "cube_data.cpp: Camera target x %4.2f, y %4.2f, z %4.2f\n", camera_target.x, camera_target.y, camera_target.z); 
    fprintf(stdout, "cube_data.cpp: Camera up x %4.2f, y %4.2f, z %4.2f\n", camera_up.x, camera_up.y, camera_up.z); 
  }

  if (d.HasMember("lights")) {
    const Value& ll = d["lights"];
    for( SizeType i = 0; i < ll.Size(); ++i ) {
      const Value& l = ll[i];
      int light_index = l["light_index"].GetInt();
      float light_position_x = (float)l["light_position"][0].GetDouble();
      float light_position_y = (float)l["light_position"][1].GetDouble();
      float light_position_z = (float)l["light_position"][2].GetDouble();
      float light_intensity_r = (float)l["light_intensity"][0].GetDouble();
      float light_intensity_g = (float)l["light_intensity"][1].GetDouble();
      float light_intensity_b = (float)l["light_intensity"][2].GetDouble();
      float light_ambient_r = (float)l["light_ambient"][0].GetDouble();
      float light_ambient_g = (float)l["light_ambient"][1].GetDouble();
      float light_ambient_b = (float)l["light_ambient"][2].GetDouble();
      float light_diffuse_r = (float)l["light_diffuse"][0].GetDouble();
      float light_diffuse_g = (float)l["light_diffuse"][1].GetDouble();
      float light_diffuse_b = (float)l["light_diffuse"][2].GetDouble();
      float light_specular_r = (float)l["light_specular"][0].GetDouble();
      float light_specular_g = (float)l["light_specular"][1].GetDouble();
      float light_specular_b = (float)l["light_specular"][2].GetDouble();
      float light_constant = (float)l["light_constant"].GetDouble();
      float light_linear = (float)l["light_linear"].GetDouble();
      float light_quadratic = (float)l["light_quadratic"].GetDouble();

      glm::vec3 light_position = {light_position_x, light_position_y, light_position_z};
      glm::vec3 light_intensity = {light_intensity_r, light_intensity_g, light_intensity_b};
      glm::vec3 light_ambient = {light_ambient_r, light_ambient_g, light_ambient_b};
      glm::vec3 light_diffuse = {light_diffuse_r, light_diffuse_g, light_diffuse_b};
      glm::vec3 light_specular = {light_specular_r, light_specular_g, light_specular_b};

      int status = new_light(light_index, light_position, light_intensity, light_ambient, light_diffuse, light_specular, light_constant, light_linear, light_quadratic);

      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - light %d not configured\n", light_index);
	return -1;
      }

      fprintf(stdout, "cube_data.cpp: Light index %d processed\n", light_index);

      // Increment our total light count if needed
      if (light_index >= n_lights) n_lights = light_index + 1;

    }
  }

  if (d.HasMember("materials")) {
    const Value& mm = d["materials"];
    for( SizeType i = 0; i < mm.Size(); ++i ) {
      const Value& m = mm[i];
      int material_index = m["material_index"].GetInt();
      float material_ambient_r = (float)m["material_ambient"][0].GetDouble();
      float material_ambient_g = (float)m["material_ambient"][1].GetDouble();
      float material_ambient_b = (float)m["material_ambient"][2].GetDouble();
      float material_diffuse_r = (float)m["material_diffuse"][0].GetDouble();
      float material_diffuse_g = (float)m["material_diffuse"][1].GetDouble();
      float material_diffuse_b = (float)m["material_diffuse"][2].GetDouble();
      float material_specular_r = (float)m["material_specular"][0].GetDouble();
      float material_specular_g = (float)m["material_specular"][1].GetDouble();
      float material_specular_b = (float)m["material_specular"][2].GetDouble();
      float material_shininess = (float)m["material_shininess"].GetDouble();
      glm::vec3 material_ambient = {material_ambient_r, material_ambient_g, material_ambient_b};
      glm::vec3 material_diffuse = {material_diffuse_r, material_diffuse_g, material_diffuse_b};
      glm::vec3 material_specular = {material_specular_r, material_specular_g, material_specular_b};

      int status = new_material(material_index, material_ambient, material_diffuse, material_specular, material_shininess);
      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - material %d not configured\n", material_index);
	return -1;
      }
      fprintf(stdout, "cube_data.cpp: Material index %d processed\n", material_index);

      // Increment our total material count if needed
      if (material_index >= n_materials) n_materials = material_index + 1;

    }
  }

  if (d.HasMember("textures")) {
    const Value& tt = d["textures"];
    for( SizeType i = 0; i < tt.Size(); ++i ) {
      const Value& t = tt[i];
      int texture_index = t["texture_index"].GetInt();
      const char* texture_filename = t["texture_filename"].GetString();
    
      int status = new_texture(texture_index, texture_filename);

      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - texture %d (%s) not configured\n", texture_index, texture_filename);
	return -1;
      }

      fprintf(stdout, "cube_data.cpp: Texture index %d processed using file %s\n", texture_index, texture_filename);

      // Increment our total texture count if needed
      if (texture_index >= n_textures) n_textures = texture_index + 1;

    }
  }

  if (d.HasMember("grounds")) {
    const Value& gg = d["grounds"];
    float ground_texture_map[8];
    for( SizeType i = 0; i < gg.Size(); ++i ) {
      const Value& g = gg[i];
      int ground_index = g["ground_index"].GetInt();
      string ground_uuid = g["ground_uuid"].GetString();
      float ground_scale_factor = (float)g["ground_scale_factor"].GetDouble();
      int ground_type = g["ground_type"].GetInt();
      float ground_color_r = (float)g["ground_color"][0].GetDouble();
      float ground_color_g = (float)g["ground_color"][1].GetDouble();
      float ground_color_b = (float)g["ground_color"][2].GetDouble();
      float ground_color_a = (float)g["ground_color"][3].GetDouble();
      glm::vec4 ground_color = {ground_color_r, ground_color_g, ground_color_b, ground_color_a};
      int ground_material = g["ground_material"].GetInt();
      int ground_texture_index = g["ground_texture_index"].GetInt();
      for( int n = 0; n < 8; ++n) { ground_texture_map[n] = (float)g["ground_texture_map"][n].GetDouble(); }
      float ground_spatial_position_x = (float)g["ground_spatial_position"][0].GetDouble();
      float ground_spatial_position_y = (float)g["ground_spatial_position"][1].GetDouble();
      float ground_spatial_position_z = (float)g["ground_spatial_position"][2].GetDouble();
      float ground_spatial_rotation_x = (float)g["ground_spatial_rotation"][0].GetDouble();
      float ground_spatial_rotation_y = (float)g["ground_spatial_rotation"][1].GetDouble();
      float ground_spatial_rotation_z = (float)g["ground_spatial_rotation"][2].GetDouble();
      // Transfer groundview parameters
      float ground_view_position_x = (float)g["ground_view_position"][0].GetDouble();
      float ground_view_position_y = (float)g["ground_view_position"][1].GetDouble();
      float ground_view_position_z = (float)g["ground_view_position"][2].GetDouble();
      float ground_view_target_x = (float)g["ground_view_target"][0].GetDouble();
      float ground_view_target_y = (float)g["ground_view_target"][1].GetDouble();
      float ground_view_target_z = (float)g["ground_view_target"][2].GetDouble();
      float ground_view_up_x = (float)g["ground_view_up"][0].GetDouble();
      float ground_view_up_y = (float)g["ground_view_up"][1].GetDouble();
      float ground_view_up_z = (float)g["ground_view_up"][2].GetDouble();
      
      int status = new_ground(ground_index, ground_uuid, ground_scale_factor,  ground_type, ground_color, ground_material, ground_texture_index, ground_texture_map, ground_spatial_position_x, ground_spatial_position_y, ground_spatial_position_z, ground_spatial_rotation_x, ground_spatial_rotation_y, ground_spatial_rotation_z, ground_view_position_x, ground_view_position_y, ground_view_position_z, ground_view_target_x, ground_view_target_y, ground_view_target_z, ground_view_up_x, ground_view_up_y, ground_view_up_z);

      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - ground %d not configured\n", ground_index);
	return -1;
      }

      fprintf(stdout, "cube_data.cpp: Ground index %d processed - uuid %s, scale factor %4.2f, type %d, color (%4.2f, %4.2f, %4.2f, %4.2f), material %d, texture %d\n", ground_index, ground_uuid.c_str(), ground_scale_factor, ground_type, ground_color_r, ground_color_g, ground_color_b, ground_color_a, ground_material, ground_texture_index);

      // Increment our total ground count if needed
      if (ground_index >= n_grounds) n_grounds = ground_index + 1;

    } // end for( SizeType i = 0;
  } // if (d.HasMember("grounds")) { 

  if (d.HasMember("cubes")) {
    const Value& cc = d["cubes"];
    float cube_texture_map[6*4*2];
    for( SizeType i = 0; i < cc.Size(); ++i ) {
      const Value& c = cc[i];

      int cube_index = c["cube_index"].GetInt();
      if (cube_index >= NC) {
	fprintf(stderr, "cube_data.cpp: cube index (%d) exceeds allocation\n", cube_index);
	return -1;
      }

      // Index will be -1 if we are adding a new cube on the fly
      if (cube_index == -1) {
	if (n_cubes == NC) {
	  fprintf(stderr, "cube_data.cpp: maximum number of cubes (%d) already allocated\n", n_cubes);
	  return -1;
	}
	cube_index = n_cubes;
	n_cubes += 1;
      }

      fprintf(stdout, "cube_data.cpp: Processing cubes, index %d\n", cube_index);
      string cube_player = c["cube_player"].GetString();
      string cube_uuid = c["cube_uuid"].GetString();
      string cube_emoticon = c["cube_emoticon"].GetString();
      string cube_firstname = c["cube_firstname"].GetString();
      float cube_scale_factor = (float)c["cube_scale_factor"].GetDouble();
      int cube_type = c["cube_type"].GetInt();
      string cube_color_class = c["cube_color_class"].GetString();
      float cube_color_r = (float)c["cube_color"][0].GetDouble();
      float cube_color_g = (float)c["cube_color"][1].GetDouble();
      float cube_color_b = (float)c["cube_color"][2].GetDouble();
      float cube_color_a = (float)c["cube_color"][3].GetDouble();
      glm::vec4 cube_color = {cube_color_r, cube_color_g, cube_color_b, cube_color_a};
      int cube_material = c["cube_material"].GetInt();
      string cube_surface = c["cube_surface"].GetString();
      int cube_texture_index = c["cube_texture_index"].GetInt();
      for( int n = 0; n < 6*8; ++n) { cube_texture_map[n] = (float)c["cube_texture_map"][n].GetDouble();}
      float spatial_position_x = (float)c["spatial_position"][0].GetDouble();
      float spatial_position_y = (float)c["spatial_position"][1].GetDouble();
      float spatial_position_z = (float)c["spatial_position"][2].GetDouble();
      float spatial_rotation_x = (float)c["spatial_rotation"][0].GetDouble();
      float spatial_rotation_y = (float)c["spatial_rotation"][1].GetDouble();
      float spatial_rotation_z = (float)c["spatial_rotation"][2].GetDouble();
      float spatial_radius = (float)c["spatial_radius"].GetDouble();
      float resource_energy = (float)c["resource_energy"].GetDouble();
      
      if (cube_texture_index == -1) {
	string texture_filename = c["cube_texture_filename"].GetString();
	if (n_textures == NT) {
	  fprintf(stderr, "cube_data.cpp: maximum number of textures (%d) already installed\n", n_textures);
	  return -1;
	}

	cube_texture_index = n_textures;
	n_textures += 1;
	
	int status = new_texture(cube_texture_index, texture_filename.c_str());

	if (status) {
	  fprintf(stderr, "cube_data.cpp: json_import - texture %d (%s) not configured\n", cube_texture_index, texture_filename.c_str());
	  return -1;
	}

	fprintf(stdout, "cube_data.cpp: Texture index %d processed using file %s\n", cube_texture_index, texture_filename.c_str());
      }

      int status = new_cube(cube_index, cube_player, cube_uuid, cube_emoticon, cube_firstname, cube_scale_factor, cube_type, cube_color_class, cube_color, cube_material, cube_surface, cube_texture_index, cube_texture_map, spatial_position_x, spatial_position_y, spatial_position_z, spatial_rotation_x, spatial_rotation_y, spatial_rotation_z, spatial_radius, resource_energy);
      
      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - cube %d not configured\n", cube_index);
	return -1;
      }

      fprintf(stdout, "cube_data.cpp: Cube index %d processed - %s scale factor %4.2f, type %d, color (%4.2f, %4.2f, %4.2f, %4.2f), material %d, texture %d uuid %s\n", cube_index, &cube_player[0], cube_scale_factor, cube_type, cube_color_r, cube_color_g, cube_color_b, cube_color_a, cube_material, cube_texture_index, &cube_uuid[0]);

      // Increment our total cube count if needed
      if (cube_index >= n_cubes) n_cubes = cube_index + 1;

    } // end for( SizeType ...
  } // end if (d.HasMember("cubes")) {

  if (d.HasMember("wires")) {
    const Value& ww = d["wires"];
    for(SizeType i = 0; i < ww.Size(); ++i ) {
      const Value& w = ww[i];
      int wire_index = w["wire_index"].GetInt();
      float wire_scale_factor = (float)w["wire_scale_factor"].GetDouble();
      float wire_position_x = (float)w["wire_position"][0].GetDouble();
      float wire_position_y = (float)w["wire_position"][1].GetDouble();
      float wire_position_z = (float)w["wire_position"][2].GetDouble();
      float wire_rotation_x = (float)w["wire_rotation"][0].GetDouble();
      float wire_rotation_y = (float)w["wire_rotation"][1].GetDouble();
      float wire_rotation_z = (float)w["wire_rotation"][2].GetDouble();
      int wire_type = w["wire_type"].GetInt();
      float wire_color_r = (float)w["wire_color"][0].GetDouble();
      float wire_color_g = (float)w["wire_color"][1].GetDouble();
      float wire_color_b = (float)w["wire_color"][2].GetDouble();
      float wire_color_a = (float)w["wire_color"][3].GetDouble();
      glm::vec4 wire_color = {wire_color_r, wire_color_g, wire_color_b, wire_color_a};

      int status = new_wire(wire_index, wire_scale_factor, wire_position_x, wire_position_y, wire_position_z, wire_rotation_x, wire_rotation_y, wire_rotation_z, wire_type, wire_color);

      if (status) {
	fprintf(stderr, "cube_data.cpp: json_import - wire %d not configured\n", wire_index);
	return -1;
      }

      fprintf(stdout, "cube_data.cpp: Wire index %d processed\n", wire_index);

      // Increment our total wire count if needed
      if (wire_index >= n_wires) n_wires = wire_index + 1;

    }
  }

  return 0;

}

// Check login credentials
bool check_login(LoginRequest msg) {

  string user = msg.username;
  string pswd = msg.password;
  return true;
  
}
