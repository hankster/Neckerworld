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
#include "cube.h"
#include "cube_client.h"
#include "base64.h"

// Error messages which might get returned.
string goodbye = "{\"message_type\": \"GoodBye\"}";
string error_a = "{\"message_type\": \"Error\", \"error\": \"Cube not active.\"}";
string error_g = "{\"message_type\": \"Error\", \"error\": \"Ground View not allowed.\"}";
string error_i = "{\"message_type\": \"Error\", \"error\": \"Cube index not found.\"}";
string error_j = "{\"message_type\": \"Error\", \"error\": \"JSON file import failed.\"}";
string error_r = "{\"message_type\": \"Error\", \"error\": \"Remote not enabled.\"}";
string error_u = "{\"message_type\": \"Error\", \"error\": \"Unknown message type.\"}";
string error_v = "{\"message_type\": \"Error\", \"error\": \"Screen View not allowed.\"}";

// Decode an incoming message
string cube_decode_message(char* message, int message_length) {

  // Message containers
  struct LoginRequest msgLoginRequest;
  struct LogoutRequest msgLogoutRequest;
  struct MoveRequest msgMoveRequest;
  struct StatusRequest msgStatusRequest;
  struct ViewRequest msgViewRequest;
  struct GroundViewRequest msgGroundViewRequest;
  struct ImportJSONFileRequest msgImportJSONFileRequest;
  struct ImportJSONObjectRequest msgImportJSONObjectRequest;
  struct LoginResponse msgLoginResponse;
  struct MoveResponse msgMoveResponse;
  struct StatusResponse msgStatusResponse;
  struct ViewResponse msgViewResponse;
  struct GroundViewResponse msgGroundViewResponse;
  struct ImportJSONResponse msgImportJSONResponse;

  // Message header
  string mt;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  
  // Make sure this message is terminated like a C string. Program crashes without this.
  message[message_length] = '\0';
  if (debug > 0) printf("cube_simulation.cpp: cube_decode_message - Message of length %d received and is %s\n", message_length, message);
  
  Document d;
  // ToDo: Need to check for valid parse. Return error if any member missing.
  d.Parse(message);
  
  if (d.HasMember("message_type")) {
    mt = d["message_type"].GetString();
    if (debug > 0) printf("cube_simulation.cpp: cube_decode_message - Message Type is %s\n", &mt[0]);
  }

  // Extract common parameters
  if (d.HasMember("sequence")) { sequence = d["sequence"].GetUint();}
  if (d.HasMember("timestamp")) { timestamp = d["timestamp"].GetDouble();}
  if (d.HasMember("cube_uuid")) { cube_uuid = d["cube_uuid"].GetString();}

  // if (debug > 0) sleep(10);
  
  if (mt == "LoginRequest") {
    msgLoginRequest.message_type = mt;
    msgLoginRequest.sequence = sequence;
    msgLoginRequest.timestamp = timestamp;
    msgLoginRequest.cube_uuid = cube_uuid;
    msgLoginRequest.username = d["username"].GetString();
    msgLoginRequest.password = d["password"].GetString();
    string rsp = doLoginRequest(msgLoginRequest);
    return rsp;
  }

  if (mt == "LogoutRequest") {
    msgLogoutRequest.message_type = mt;
    msgLogoutRequest.sequence = sequence;
    msgLogoutRequest.timestamp = timestamp;
    msgLogoutRequest.cube_uuid = cube_uuid;
    string rsp = doLogoutRequest(msgLogoutRequest);
    return rsp;
  }

  if (mt == "ImportJSONFileRequest") {
    msgImportJSONFileRequest.message_type = mt;
    msgImportJSONFileRequest.sequence = sequence;
    msgImportJSONFileRequest.timestamp = timestamp;
    msgImportJSONFileRequest.JSONFilename = d["jsonfilename"].GetString();
    msgImportJSONFileRequest.cube_uuid = cube_uuid;
    string rsp = doImportJSONFileRequest(msgImportJSONFileRequest);
    return rsp;
  }
  
  if (mt == "ImportJSONObjectRequest") {
    msgImportJSONObjectRequest.message_type = mt;
    msgImportJSONObjectRequest.sequence = sequence;
    msgImportJSONObjectRequest.timestamp = timestamp;
    msgImportJSONObjectRequest.JSONObject = d["jsonobject"].GetString();
    msgImportJSONObjectRequest.cube_uuid = cube_uuid;
    string rsp = doImportJSONObjectRequest(msgImportJSONObjectRequest);
    return rsp;
  }
  
  // Check if we are using a valid cube id.
  int cube_index = cube_get_index(cube_uuid);
  if (cube_index < 0) return error_i;

  // Check if we are logged in and remote has been set.
  if (! cubes[cube_index].cube_remote) return error_r;

  if (mt == "MoveRequest") {
    msgMoveRequest.message_type = mt;
    msgMoveRequest.sequence = sequence;
    msgMoveRequest.timestamp = timestamp;
    msgMoveRequest.cube_uuid = cube_uuid;
    msgMoveRequest.spatial_angle = (float)d["spatial_angle"].GetDouble();
    msgMoveRequest.spatial_direction = (float)d["spatial_direction"].GetDouble();
    msgMoveRequest.spatial_direction_active = (float)d["spatial_direction_active"].GetBool();
    msgMoveRequest.distance = (float)d["distance"].GetDouble();
    msgMoveRequest.velocity = (float)d["velocity"].GetDouble();
    msgMoveRequest.gaze.x = (float)d["gaze"][0].GetDouble();
    msgMoveRequest.gaze.y = (float)d["gaze"][1].GetDouble();
    string rsp = doMoveRequest(msgMoveRequest);
    return rsp;
  }

  if (mt == "StatusRequest") {
    msgStatusRequest.message_type = mt;
    msgStatusRequest.sequence = sequence;
    msgStatusRequest.timestamp = timestamp;
    msgStatusRequest.cube_uuid = cube_uuid;
    string rsp = doStatusRequest(msgStatusRequest);
    return rsp;
  }
  
  if (mt == "ViewRequest") {
    msgViewRequest.message_type = mt;
    msgViewRequest.sequence = sequence;
    msgViewRequest.timestamp = timestamp;
    msgViewRequest.cube_uuid = cube_uuid;
    msgViewRequest.spatial_angle = (float)d["spatial_angle"].GetDouble();
    msgViewRequest.gaze.x = (float)d["gaze"][0].GetDouble();
    msgViewRequest.gaze.y = (float)d["gaze"][1].GetDouble();
    string rsp = doViewRequest(msgViewRequest);
    return rsp;
  }
  
  if (mt == "GroundViewRequest") {
    msgGroundViewRequest.message_type = mt;
    msgGroundViewRequest.sequence = sequence;
    msgGroundViewRequest.timestamp = timestamp;
    msgGroundViewRequest.cube_uuid = cube_uuid;
    msgGroundViewRequest.groundview = d["groundview"].GetInt();
    string rsp = doGroundViewRequest(msgGroundViewRequest);
    return rsp;
  }
  
  return error_u;

}

// Process a login request
string doLoginRequest(LoginRequest msgLoginRequest) {

  bool valid_user = check_login(msgLoginRequest);
  if (! valid_user) {
    return goodbye;
  }

  if (debug > 0) printf("cube_simulation.cpp: login cube_uuid %s\n", &msgLoginRequest.cube_uuid[0]);
  int cube_index = cube_get_index(msgLoginRequest.cube_uuid);
  if (debug > 0) printf("cube_simulation.cpp: login cube index %d\n", cube_index);

  if (cube_index >= 0) {

    // This cube is now remotely controlled
    cubes[cube_index].cube_remote = true;
    if (debug > 0) printf("cube_simulation.cpp: doLoginRequest - cube %d now under remote control\n", cube_index);

    // Allow ground view requests now, too.
    for (int i=0; i<n_grounds; ++i) {
      grounds[i].ground_remote = true;
    }
  }
  
  // document is the root of a json message
  Document d;
  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();
  d.AddMember("message_type", "LoginResponse", allocator);
  d.AddMember("sequence", msgLoginRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("cube_uuid", msgLoginRequest.cube_uuid, allocator);
  d.AddMember("frame", frame_counter, allocator);
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doLoginRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

// Process a logout request
string doLogoutRequest(LogoutRequest msgLogoutRequest) {

  int cube_index = cube_get_index(msgLogoutRequest.cube_uuid);
  if (cube_index < 0 || ! cubes[cube_index].cube_remote) {
    return goodbye;
  }
  
  // This cube is no longer remotely controlled
  cubes[cube_index].cube_remote = false;
  if (debug > 0) printf("cube_simulation.cpp: doLogoutRequest - cube %d no longer remote\n", cube_index);
  
  // Cancel ground view requests now, too.
  for (int i=0; i<n_grounds; ++i) {
    grounds[i].ground_remote = false;
  }

  // document is the root of a json message
  Document d;
  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();
  d.AddMember("message_type", "LogoutResponse", allocator);
  d.AddMember("sequence", msgLogoutRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("cube_uuid", msgLogoutRequest.cube_uuid, allocator);
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doLogoutRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

// Client wants to move the cube
string doMoveRequest(MoveRequest msgMoveRequest) {

  int cube_index = cube_get_index(msgMoveRequest.cube_uuid);

  float angle = msgMoveRequest.spatial_angle;
  float direction = msgMoveRequest.spatial_direction;
  bool direction_active = msgMoveRequest.spatial_direction_active;
  float distance = msgMoveRequest.distance;
  float velocity = msgMoveRequest.velocity;
  glm::vec2 gaze = glm::vec2(msgMoveRequest.gaze.x, msgMoveRequest.gaze.y);
  // Clear the blocked flag if the move uses the spatial_direction_active option
  if (direction_active) cubes[cube_index].spatial_position_blocked  = false;
  int status = cube_remote_move(cube_index, angle, direction, direction_active, distance, velocity, gaze);
  if (status != 0) return error_a;
  
  // document is the root of a json message
  Document d;
  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();
  d.AddMember("message_type", "MoveResponse", allocator);
  d.AddMember("sequence", msgMoveRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("cube_uuid", msgMoveRequest.cube_uuid, allocator);
  d.AddMember("frame", frame_counter, allocator);
  d.AddMember("spatial_position_blocked", cubes[cube_index].spatial_position_blocked, allocator);
  d.AddMember("resource_energy", cubes[cube_index].resource_energy, allocator);
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doMoveRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

string doStatusRequest(StatusRequest msgStatusRequest) {
  if (debug > 0) printf("cube_simulation.cpp: doStatusRequest for cube %s\n", &msgStatusRequest.cube_uuid[0]);
  StatusResponse s = cube_status(msgStatusRequest.cube_uuid);
  if (debug > 0) printf("cube_simulation.cpp: doStatusRequest for cube index %d\n", s.cube_index);

  // document is the root of a json message
  Document d;

  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();

  d.AddMember("message_type", "StatusResponse", allocator);
  d.AddMember("sequence", msgStatusRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("frame", frame_counter, allocator);
  d.AddMember("cube_uuid", msgStatusRequest.cube_uuid, allocator);
  d.AddMember("cube_player", s.cube_player, allocator);
  d.AddMember("cube_firstname", s.cube_firstname, allocator);
  d.AddMember("cube_active", s.cube_active, allocator);
  d.AddMember("cube_display", s.cube_display, allocator);
  d.AddMember("cube_remote", s.cube_remote, allocator);
  d.AddMember("cube_scale_factor", s.cube_scale_factor, allocator);
  d.AddMember("spatial_angle", s.spatial_angle, allocator);
  d.AddMember("spatial_direction", s.spatial_direction, allocator);
  d.AddMember("spatial_direction_active", s.spatial_direction_active, allocator);
  Value spatial_gaze(kArrayType);
  spatial_gaze.PushBack(s.spatial_gaze.x, allocator);
  spatial_gaze.PushBack(s.spatial_gaze.y, allocator);
  d.AddMember("spatial_gaze", spatial_gaze, allocator);
  d.AddMember("spatial_radius", s.spatial_radius, allocator);
  d.AddMember("spatial_distance", s.spatial_distance, allocator);
  d.AddMember("spatial_distance_previous", s.spatial_distance_previous, allocator);
  d.AddMember("spatial_velocity", s.spatial_velocity, allocator);
  d.AddMember("spatial_position_blocked", s.spatial_position_blocked, allocator);
  d.AddMember("life_birth", difftime(s.life_birth,0), allocator);
  d.AddMember("life_death", difftime(s.life_death,0), allocator);
  d.AddMember("life_father", s.life_father, allocator);
  d.AddMember("life_mother", s.life_mother, allocator);
  d.AddMember("resource_energy", s.resource_energy, allocator);
  Value total_points(kArrayType);
  total_points.PushBack(s.total_points[0], allocator);
  total_points.PushBack(s.total_points[1], allocator);
  total_points.PushBack(s.total_points[2], allocator);
  total_points.PushBack(s.total_points[3], allocator);
  d.AddMember("total_points", total_points, allocator);

  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doStatusRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

string doViewRequest(ViewRequest msgViewRequest) {

  string uuid = msgViewRequest.cube_uuid;
  float angle = msgViewRequest.spatial_angle;
  float gaze_yaw = msgViewRequest.gaze.x;
  float gaze_pitch = msgViewRequest.gaze.y;

  ViewResponse r = screenview(uuid, angle, gaze_yaw, gaze_pitch);

  if (r.cubeview < 0) return error_v;
			 
  // document is the root of a json message
  Document d;

  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();

  d.AddMember("message_type", "ViewResponse", allocator);
  d.AddMember("sequence", msgViewRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("frame", frame_counter, allocator);
  d.AddMember("cube_uuid", msgViewRequest.cube_uuid, allocator);
  d.AddMember("extension", r.extension, allocator);
  d.AddMember("mode", r.mode, allocator);
  d.AddMember("width", r.width, allocator);
  d.AddMember("height", r.height, allocator);
  d.AddMember("channels", r.channels, allocator);
  Value bounding_box(kArrayType);
  bounding_box.PushBack(r.bounding_box[0], allocator);
  bounding_box.PushBack(r.bounding_box[1], allocator);
  bounding_box.PushBack(r.bounding_box[2], allocator);
  bounding_box.PushBack(r.bounding_box[3], allocator);
  d.AddMember("bounding_box", bounding_box, allocator);

  int pixels_size = r.pixels.size();

  if (debug > 0) printf("cube_simulation.cpp: doViewRequest pixels size %d\n", pixels_size);

  // If we have pixel data
  if (pixels_size > 0) {

    // This set of steps takes the raw pixel buffer (e.g. 1280x720x4 RGBA)
    // and next does a zlib compression on all the pixels
    // followed by an encoding to base64 (binary data can't be serialized in a JSON string)
    // and then all that is added to our returned string which will be sent back to the client.

    int zlib_required_buffer = int(float(r.width * r.height * r.channels)*1.001+12.0);
    uint8_t* gz = new uint8_t[zlib_required_buffer];
    long unsigned int gz_size = zlib_required_buffer;

    int gz_error = compress2(gz, &gz_size, r.pixels.data(), pixels_size, Z_DEFAULT_COMPRESSION);
    if (gz_error != Z_OK) {
      printf("cube_simulation.cpp: zlib compression error %d. Returned size %d\n", gz_error, int(gz_size));
      gz_size = 0;
    }
    int max_encoded_length = Base64encode_len(gz_size);
    char* encoded = new char[max_encoded_length];
    unsigned char* array = gz;

    int result = Base64encode(encoded, (const char *)array, gz_size);

    // encoded is now a null terminated b64 string
    // "result" is the length of the string *including* the null terminator
    // don't forget to invoke "delete [] encoded" when done

    string img = encoded;

    d.AddMember("pixels_b64", img, allocator);
    d.AddMember("pixels_frame", r.pixels_frame, allocator);

    // Get rid of our temporary arrays
    delete gz;
    delete encoded;

    if (debug > 0) printf("cube_simulation.cpp: doViewRequest pixels_b64 size %d\n", (int)img.size());
  }
  
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doViewRequest - response message length %d\n", (int)strbuf.GetLength());
  return response;
}

string doGroundViewRequest(GroundViewRequest msgGroundViewRequest) {

  string uuid = msgGroundViewRequest.cube_uuid;
  int gv = msgGroundViewRequest.groundview;

  GroundViewResponse r = ground_screenview(uuid, gv);

  if (r.groundview < 0) return error_g;
			 
  // document is the root of a json message
  Document d;

  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();

  d.AddMember("message_type", "GroundViewResponse", allocator);
  d.AddMember("sequence", msgGroundViewRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  d.AddMember("frame", frame_counter, allocator);
  d.AddMember("groundview", msgGroundViewRequest.groundview, allocator);
  d.AddMember("extension", r.extension, allocator);
  d.AddMember("mode", r.mode, allocator);
  d.AddMember("width", r.width, allocator);
  d.AddMember("height", r.height, allocator);
  d.AddMember("channels", r.channels, allocator);
  Value bounding_box(kArrayType);
  bounding_box.PushBack(r.bounding_box[0], allocator);
  bounding_box.PushBack(r.bounding_box[1], allocator);
  bounding_box.PushBack(r.bounding_box[2], allocator);
  bounding_box.PushBack(r.bounding_box[3], allocator);
  d.AddMember("bounding_box", bounding_box, allocator);

  int pixels_size = r.pixels.size();
  if (debug > 0) printf("cube_simulation.cpp: doGroundViewRequest pixels size %d\n", pixels_size);

  // If we have pixel data
  if (pixels_size > 0) {

    // This set of steps takes the raw pixel buffer (e.g. 1280x720x4 RGBA)
    // and next does a zlib compression on all the pixels
    // followed by an encoding to base64 (binary data can't be serialized in a JSON string)
    // and then all that is added to our returned string which will be sent back to the client.

    int zlib_required_buffer = int(float(r.width * r.height * r.channels)*1.001+12.0);
    uint8_t* gz = new uint8_t[zlib_required_buffer];
    long unsigned int gz_size = zlib_required_buffer;

    int gz_error = compress2(gz, &gz_size, r.pixels.data(), pixels_size, Z_DEFAULT_COMPRESSION);
    if (gz_error != Z_OK) {
      printf("cube_simulation.cpp: zlib compression error %d. Returned size %d\n", gz_error, int(gz_size));
      gz_size = 0;
    }
    int max_encoded_length = Base64encode_len(gz_size);
    char* encoded = new char[max_encoded_length];
    unsigned char* array = gz;

    int result = Base64encode(encoded, (const char *)array, gz_size);

    // encoded is now a null terminated b64 string
    // "result" is the length of the string *including* the null terminator
    // don't forget to invoke "delete [] encoded" when done

    string img = encoded;

    d.AddMember("pixels_b64", img, allocator);
    d.AddMember("pixels_frame", r.pixels_frame, allocator);

    // Get rid of our temporary arrays
    delete gz;
    delete encoded;

    if (debug > 0) printf("cube_simulation.cpp: doGroundViewRequest pixels_b64 size %d\n", (int)img.size());
  }
  
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doGroundViewRequest - response message length %d\n", (int)strbuf.GetLength());
  return response;
}

string doImportJSONFileRequest(ImportJSONFileRequest msgImportJSONFileRequest) {

  json_import_file = msgImportJSONFileRequest.JSONFilename;
  bool add_JSON = msgImportJSONFileRequest.add_JSON;

  // Can't use json_import() here as it's in the multi-thread context.
  // So we save the filename and setup a flag and the import is called by cube.cpp in the main loop.
  // string jf = msgImportJSONFileRequest.JSONFilename;
  // int status = json_import(&jf[0]);
  // if (status != 0) return error_j;

  json_flag_file = true;
  
  // document is the root of a json message
  Document d;

  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();

  d.AddMember("message_type", "ImportJSONFileResponse", allocator);
  d.AddMember("sequence", msgImportJSONFileRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doImportJSONFileRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

string doImportJSONObjectRequest(ImportJSONObjectRequest msgImportJSONObjectRequest) {

  json_import_object = msgImportJSONObjectRequest.JSONObject;
  bool add_JSON = msgImportJSONObjectRequest.add_JSON;

  // Can't use json_import() here as it's in the multi-thread context.
  // So we save the filename and setup a flag and the import is called by cube.cpp in the main loop.
  // string jf = msgImportJSONObjectRequest.JSONObject;
  // int status = json_import(&jf[0]);
  // if (status != 0) return error_j;

  json_flag_object = true;
  
  // document is the root of a json message
  Document d;

  // define the document as an object rather than an array
  d.SetObject();
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();

  d.AddMember("message_type", "ImportJSONObjectResponse", allocator);
  d.AddMember("sequence", msgImportJSONObjectRequest.sequence, allocator);
  d.AddMember("timestamp", frame_time, allocator);
  
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);
  string response = strbuf.GetString();
  if (debug > 0) printf("cube_simulation.cpp: doImportJSONObjectRequest - %d %s\n", (int)strbuf.GetLength(), &response[0]);
  return response;
}

/*
bool json_write() {

  char cbuf[1024]; rapidjson::MemoryPoolAllocator<> allocator (cbuf, sizeof cbuf);
  rapidjson::Document meta (&allocator, 256);
  meta.SetObject();
  meta.AddMember ("foo", 123, allocator);

  typedef rapidjson::GenericStringBuffer<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>> StringBuffer;
  StringBuffer buf (&allocator);
  rapidjson::Writer<StringBuffer> writer (buf, &allocator);
  meta.Accept (writer);
  std::string json (buf.GetString(), buf.GetSize());

  std::ofstream of ("/tmp/example.json");
  of << json;
  if (!of.good()) throw std::runtime_error ("Can't write the JSON string to the file!");
  
}

bool j_write() {

  // document is the root of a json message
  Document d;
 
  // define the document as an object rather than an array
  d.SetObject();
 
  // must pass an allocator when the object may need to allocate memory
  Document::AllocatorType& allocator = d.GetAllocator();
 
  d.AddMember("Name", "XYZ", allocator);
  d.AddMember("Rollnumer", 2, allocator);
 
  // create a rapidjson object type
  rapidjson::Value object(rapidjson::kObjectType);
  object.AddMember("Math", "50", allocator);
  object.AddMember("Science", "70", allocator);
  object.AddMember("English", "50", allocator);
  object.AddMember("Social Science", "70", allocator);
  d.AddMember("Marks", object, allocator);
  //	fromScratch["object"]["hello"] = "Yourname";
 
  StringBuffer strbuf;
  Writer<StringBuffer> writer(strbuf);
  d.Accept(writer);

  std::cout << strbuf.GetString() << std::endl;


}
*/


// Check for contact between cubes "i" and "j"
bool cube_contact(int i, int j) {

  // Check for valid cubes
  if (i > n_cubes) return false;
  if (j > n_cubes) return false;

  // Check if it's me
  if (i == j) return false;
  
  // Check if each cube is still active in the game
  if (! cubes[i].cube_active || ! cubes[i].cube_display) return false;
  if (! cubes[j].cube_active || ! cubes[j].cube_display) return false;

  float ri = cubes[i].spatial_radius;
  float rj = cubes[j].spatial_radius;

  // If the distance is less than the sum of the two spatial radius's ...
  if (cube_distance(i, j) < ri + rj) {
    return true;
  } else {
    return false;
  }
}

// Cube player conversion -- string to integer
int cube_contact_index(string player) {

  string m = "male";
  string f = "female";
  string e = "enby";
  string p = "predator";
  string r = "resource";

  if (player == m) return 0;
  if (player == f) return 1;
  if (player == e) return 2;
  if (player == p) return 3;
  if (player == r) return 4;

  return -1;
  
}

// Evaluate what to do if two cubes contact
bool cube_contact_evaluation(int i, int j) {

  //                               male            female            enby           predator         resource
  int conflict_type[5][5] = {{0, 1, 0, 2, 3}, {1, 0, 0, 2, 3}, {0, 0, 0, 2, 3}, {2, 2, 2, 0, 0}, {3, 3, 3, 0, 0}};
  
  // Check for valid cubes
  if (i > n_cubes) return false;
  if (j > n_cubes) return false;

  string ip = cubes[i].cube_player;
  string jp = cubes[j].cube_player;
  // if (debug > 1) printf("cube.cpp: Contact between %s %d and %s %d\n", &ip[0], i, &jp[0], j);
  int ii = cube_contact_index(ip);
  int jj = cube_contact_index(jp);
  if (ii < 0 || jj < 0) return false;

  int conflict_rules = conflict_type[ii][jj];
  // if (debug > 1) printf("cube.cpp: Conflict rules %d\n", conflict_rules);

  // male vs. male or enby, femle vs. female or enby, enby vs. m or f or e, predator vs. predator or resource, resource vs. predator or resource
  if (conflict_rules == 0) {
    return cube_contact_no_action(i,j);
  }
  // mating male vs. female, female vs. male
  if (conflict_rules == 1) {
    int m = (ip == "male") ? i : j;
    int f = (jp == "female") ? j : i;
    return cube_contact_mate(m,f);
  }
  // male or female or enby vs. predator
  if (conflict_rules == 2) {
    int pl = (ip != "predator") ? i : j;
    int pr = (jp == "predator") ? j : i;
    return cube_contact_attack(pl,pr);
  }
  // male of female or enby vs. resource
  if (conflict_rules == 3) {
    if (ip != "resource") return cube_contact_resource(i, j);
  }
  
  return false;
  
} // cube_contact_evaluation

// Action routines for contact events - Two cubes collide, zero out velocity
bool cube_contact_no_action(int player1, int player2) {
  if (cubes[player1].spatial_velocity > 0.0) {
    if (diagnostic) printf("cube.cpp: %8s %2d collides with %s %d\n", &cubes[player1].cube_player[0], player1, &cubes[player2].cube_player[0], player2);
  }
  if (cubes[player2].spatial_velocity > 0.0) {
    if (diagnostic) printf("cube.cpp: %8s %2d collides with %s %d\n", &cubes[player2].cube_player[0], player2, &cubes[player1].cube_player[0], player1);
  }
  return true;
}

// Action routines for contact events - Male and Female cubes mate
bool cube_contact_mate(int male, int female) {

  time_t time_now = time(NULL);
  
  // Check for space for another child cube
  if (n_cubes == NC) {
    if (diagnostic) printf("cube.cpp: No more room for babies\n");
    return false;
  }

  // No incest. Don't mate with your mother or father.
  if (cubes[male].life_mother == cubes[female].cube_uuid || cubes[female].life_father == cubes[male].cube_uuid) {
    if (diagnostic) printf("cube.cpp: %8s %2d please no incest with %s %d\n", &cubes[male].cube_player[0], male, &cubes[female].cube_player[0], female);
    return false;
  }

  // Check to see if either of these cubes is still a youth
  if (difftime(time_now, cubes[male].life_birth) < LIFE_YOUTH_HOLD_TIME) {
  if (diagnostic) printf("cube.cpp: %8s %2d cannot mate now. Too young (%0.1f).\n", &cubes[male].cube_player[0], male, difftime(time_now, cubes[male].life_birth));
    return false;
  }
if (difftime(time_now, cubes[female].life_birth) < LIFE_YOUTH_HOLD_TIME) {
    if (diagnostic) printf("cube.cpp: %8s %2d cannot mate now. Too young (%0.1f).\n", &cubes[female].cube_player[0], female, difftime(time_now, cubes[female].life_birth));
    return false;
  }
  // Check to see if mate time has elapsed
if (cubes[female].life_mate_times.size() > 0 && difftime(time_now, cubes[female].life_mate_times.back()) < LIFE_MATE_HOLD_TIME) {
    if (diagnostic) printf("cube.cpp: %8s %2d cannot mate now. Zip your pants %d\n", &cubes[female].cube_player[0], female, male);
    return false;
  }
  
  // Log the mate
  cubes[male].life_mates.push_back(cubes[female].cube_uuid);
  cubes[female].life_mates.push_back(cubes[male].cube_uuid);
  
  // Log the mate times
  time_t mate_time = time(NULL);
  cubes[male].life_mate_times.push_back(mate_time);
  cubes[female].life_mate_times.push_back(mate_time);

  // Create a new cube from mating.
  cube_mates(male, female);

  return true;
}

// Action routines for contact events - Player vs. predator attack
bool cube_contact_attack(int player, int predator) {

  // Note opponents
  cubes[player].match_opponent.push_back(cubes[predator].cube_uuid);
  cubes[predator].match_opponent.push_back(cubes[player].cube_uuid);
  // Note time
  time_t match_time = time(NULL);
  cubes[player].match_times.push_back(match_time);
  cubes[predator].match_times.push_back(match_time);
  // Both match targets are done for now
  cubes[player].match_target = -1;
  cubes[predator].match_target = -1;

  // If we're bigger than the predator, it's game over.
  if (cubes[player].cube_scale_factor > cubes[predator].cube_scale_factor) {
    // Predator took on someone bigger. Too bad. This predator is out of the game
    cubes[predator].cube_active = false;
    cubes[predator].life_death = time(NULL);
    // This predator disappears from the screen
    cubes[predator].cube_display = false;
    // No energy gets transferred to either side, the winner gets the energy points
    cubes[predator].match_energy.push_back(0.0);
    cubes[player].match_energy.push_back(cubes[predator].resource_energy);
    cubes[player].match_results.push_back(true);
    cubes[predator].match_results.push_back(false);
    printf("cube.cpp: %8s %2d eliminates predator %d\n", &cubes[player].cube_player[0], player, predator);
  } else {
    // Uh oh, the predator is bigger. We still might win.
    // The method for determining if the player can knock off a larger predator is this:
    // The size advantage can go from 1.0 to 2.4 (0.5/0.5 to 1.2/0.5)
    // At 1.0 the player has a 100% chance of killing off the predator.
    // At 2.4 the player has a 0.0% chance of killing off the predator.
    // We pick a random number from 0.0 to 1.0.
    // If that number is greater than the required threshold based on size advantage the player wins.
    // So if the size advantage is 2.4 the picked number must be 1.0 (impossible).
    // So if the size advantage is 1.0 the picked number must be greater than 0.0 (always wins).
    // We give special superpowers to enbies and set their size advantage to "superpower" or less.
    bool isEnby = cubes[player].cube_player == "enby";
    float superpower = 0.2;
    // The size advantage is the ratio of the size of the two cubes
    float size_advantage = cubes[predator].cube_scale_factor/cubes[player].cube_scale_factor;
    // The threshold_requirement ranges from 0.0 to 1.0
    float threshold_requirement = (size_advantage - 1.0)/1.4;
    // Give enbies superpowers
    if (isEnby) threshold_requirement = min(threshold_requirement, superpower); 
    // Compute a random number from 0.0 to 1.0
    float r = random1();
    printf("cube.cpp: size advantage (predator/player) = %0.4f, threshold = %0.4f, random number = %0.4f\n", size_advantage, threshold_requirement, r);
    // If we can exceed the threshold then the player lucks out and beats the predator 
    if (r > threshold_requirement) {
      // Player beats predator
      cubes[predator].cube_active = false;
      cubes[predator].life_death = time(NULL);
      // This predator disappears from the screen
      cubes[predator].cube_display = false;
      // No energy gets transferred to either side, the winner gets the energy points
      cubes[predator].match_energy.push_back(0.0);
      cubes[player].match_energy.push_back(cubes[predator].resource_energy);
      cubes[player].match_results.push_back(true);
      cubes[predator].match_results.push_back(false);
      printf("cube.cpp: %8s %2d eliminates predator %d through heroic effort\n", &cubes[player].cube_player[0], player, predator);
    } else {
      // Predator beats player. Game over for the player.
      cubes[player].cube_active = false;
      cubes[player].life_death = time(NULL);
      // This player disappears from the screen
      cubes[player].cube_display = false;
      // And the predator gets all the player's energy
      cubes[predator].resource_energy += cubes[player].resource_energy;
      cubes[predator].match_energy.push_back(cubes[player].resource_energy);
      cubes[player].resource_energy = 0.0;
      cubes[player].match_energy.push_back(0.0);
      cubes[predator].match_results.push_back(true);
      cubes[player].match_results.push_back(false);
      printf("cube.cpp: %8s %2d eliminates %s %d\n", &cubes[predator].cube_player[0], predator, &cubes[player].cube_player[0], player);
    }
    return true;
  }
  return false;
}

// Action routines for contact events - Player gets the resource
bool cube_contact_resource(int player, int resource) {

  // Check if player is already at the liimit
  if (cubes[player].resource_energy > ENERGY_LIMIT) {
    return false;
  }
  // Player energy from the resource
  float energy_transferred = min(cubes[resource].resource_energy, ENERGY_MAX_TRANSFER);
  cubes[player].resource_energy += energy_transferred;
  // Log the resource capture
  cubes[player].resource_list.push_back(cubes[resource].cube_uuid);
  cubes[player].resource_times.push_back(time(NULL));
  cubes[player].resource_capture.push_back(energy_transferred);
  // Decommission this resource if it's empty
  cubes[resource].resource_energy -= energy_transferred;
  if (cubes[resource].resource_energy < 0.001) {
    // This resource is out of the game
    cubes[resource].cube_active = false;
    cubes[resource].life_death = time(NULL);
    // This resource disappears from the screen
    cubes[resource].cube_display = false;
    printf("cube.cpp: %8s %2d exhausted.\n", &cubes[resource].cube_player[0], resource);
  }
  printf("cube.cpp: %8s %2d gets resource %d and now has energy %0.2f\n", &cubes[player].cube_player[0], player, resource, cubes[player].resource_energy);
  return true;

}

// Find nearest active player or resource
int find_nearest(int me, string type) {

  int nearest = -1;
  float distance = 1000000.00;
  time_t time_now = time(NULL);

  for (int i=0; i<n_cubes; ++i) {
    if (i == me) continue;
    string prospect = cubes[i].cube_player;
    if (! cubes[i].cube_active || ! cubes[i].cube_display || (type != prospect)) {
      continue;
    }
    // Special condition - female targeting a male cannot be her father
    if (cubes[me].cube_player == "female" && prospect == "male" && cubes[i].cube_uuid == cubes[me].life_father) {
      // This particular male won't do. Try again.
      continue;
    }
    // Special condition - male targeting a female cannot be his mother
    if (cubes[me].cube_player == "male" && prospect == "female" && cubes[i].cube_uuid == cubes[me].life_mother) {
      // This particular male won't do. Try again.
      continue;
    }
    // Special condition - male cannot target a young female. No statutory rape in this game.
    if (cubes[me].cube_player == "male" && prospect == "female" && difftime(time_now, cubes[i].life_birth) < LIFE_YOUTH_HOLD_TIME) {
      // This female is underage. Try again.
      continue;
    }
    // Special condition - female cannot target a young male. No cougars in this game.
    if (cubes[me].cube_player == "female" && prospect == "male" && difftime(time_now, cubes[i].life_birth) < LIFE_YOUTH_HOLD_TIME) {
      // This particular male is too young. Try again.
      continue;
    }
    float d = cube_distance(me, i);
    if (d < distance) {
      distance = d;
      nearest = i;
    }
  }

  return nearest;
  
}

// Given delta in x and z, find the angle.
float find_angle(float dz, float dx) {
  return 2.0 * atan(dx / (dz + sqrt(dz*dz + dx*dx)));
}

// Backup motion activity check
bool backup_active(int me) {

  if (cubes[me].spatial_position_timer > 0) {
    // We're still doing some backup steps
    cubes[me].spatial_position_timer -= 1;
    return true;
  }
  
  if (cubes[me].spatial_position_blocked) {
    // Turn around and try to get out.
    cubes[me].spatial_direction = cubes[me].spatial_rotation.y + pi * (random1()+0.5);
    cubes[me].spatial_direction_active = true;
    cubes[me].spatial_rotation.y = cubes[me].spatial_direction + pi;
    cube_update_angle(me, cubes[me].spatial_rotation.y);
    cube_update_model(me);
    cubes[me].spatial_velocity = random1() + 0.5;
    // Backup for 0.5 seconds
    cubes[me].spatial_position_timer = int(0.5 * frames_per_second);
    cubes[me].spatial_position_blocked = false;
    return true;
  }

  return false;

}

void strategy_male(int me) {
  string r = "resource";
  string p = "predator";
  int target = -1;
  float energy = cubes[me].resource_energy;
  // if (debug > 1) printf("cube.cpp: %d mt %d rt %d energy %0.2f\n",  me, cubes[me].match_target, cubes[me].resource_target, energy);

  // Check if we're doing a backup motion
  if (backup_active(me)) return;
      
  // If we've got enough energy, go after predators
  if (energy > ENERGY_THRESHOLD) {
    int mt = cubes[me].match_target;
    if (mt >= 0 && cubes[mt].cube_active) {
      target = mt;
    } else {
      target = find_nearest(me, p);
      cubes[me].match_target = target;
      if (target >= 0) {
	cubes[me].resource_target = -1;
	printf("cube.cpp: %8s %2d hunting %s %d near me\n", &cubes[me].cube_player[0], me, &p[0], target);
      }
    }
  }
  // If we don't yet have a target, continue looking for a resource
  int rt = cubes[me].resource_target;
  if (target < 0 && rt >= 0 && cubes[rt].cube_active && energy < ENERGY_LIMIT) {
    target = rt;
  }
  if (target < 0 && energy < ENERGY_LIMIT) {
    target = find_nearest(me, r);
    cubes[me].resource_target = target;
    cubes[me].match_target = -1;
    if (target >= 0) printf("cube.cpp: %8s %2d foraging %s %d near me. My energy is %0.2f.\n", &cubes[me].cube_player[0], me, &r[0], target, cubes[me].resource_energy);
  }
  if (target >= 0 && cubes[target].cube_active) {
    cube_move_to_target(me, target);
  } else {
    // Nothing's available. Just shut down for now.
    cubes[me].match_target = -1;
    cubes[me].resource_target = -1;
    if (cubes[me].spatial_velocity > 0.0) printf("cube.cpp: %8s %2d resting.\n", &cubes[me].cube_player[0], me);
    cubes[me].spatial_velocity = 0.0;
  }
  
  return;
}

void strategy_female(int me) {

  string r = "male";
  string nr = "resource";
  int target = -1;

  // Check if we're doing a backup motion
  if (backup_active(me)) return;
      
  // If we just had a child, we're recuperating
  time_t time_now = time(NULL);
  time_t time_of_birth = cubes[me].life_birth;
  time_t time_last_mated = 0;
  if (cubes[me].life_mate_times.size() > 0) time_last_mated = cubes[me].life_mate_times.back();
  bool recuperating = difftime(time_now, time_last_mated) < LIFE_MATE_HOLD_TIME;
  // See if we're of age
  bool youth = difftime(time_now, time_of_birth) < LIFE_YOUTH_HOLD_TIME;

  // if (diagnostic) printf("cube.cpp: %8s %2d age %0.2f recuperating %s youth %s\n", &cubes[me].cube_player[0], me, difftime(time_now, time_of_birth), recuperating?"yes":"no", youth?"yes":"no"); 
  
  if (recuperating) {
    // Cancel any previous male seearch
    cubes[me].life_target_male = -1;
    if (diagnostic) printf("cube.cpp: %8s %2d recuperating\n", &cubes[me].cube_player[0], me);
  }
  
  // Check if we're low on energy and are looking for resources
  int rt = cubes[me].resource_target;
  float energy = cubes[me].resource_energy;
  if (rt >= 0 && cubes[rt].cube_active && energy < ENERGY_LIMIT) {
    target = rt;
  }
  // Check our energy level and if not at max and low, or if recuperating,  make sure we switch to foraging
  if (target < 0 && energy < ENERGY_LIMIT && (recuperating || energy < ENERGY_THRESHOLD)) {
    target = find_nearest(me, nr);
    if (target >= 0) {
      // We've found an energy resource. Switch to it
      cubes[me].resource_target = target;
      // Cancel any old target, now obsolete
      cubes[me].life_target_male = -1;
      printf("cube.cpp: %8s %2d low on energy, switching to find %s %d. My energy is %0.2f.\n", &cubes[me].cube_player[0], me, &cubes[target].cube_player[0], target, energy);
    }
  }
  // If we still don't have a target, see if we are using a life target 
  int lt = cubes[me].life_target_male;
  if (target < 0 && !recuperating && lt >= 0 && cubes[lt].cube_active) {
    target = lt;
  }
  // No target yet. See if we can find one.
  if (target < 0 && !recuperating && !youth) {
    // Dad will not be returned by find_nearest()
    target = find_nearest(me, r);
    // We found a male life target
    cubes[me].life_target_male = target;
    cubes[me].resource_target = -1;
    if (target >= 0) printf("cube.cpp: %8s %2d chasing %s %d near me. I'm %0.1f.\n", &cubes[me].cube_player[0], me, &r[0], target, difftime(time_now, time_of_birth));
  }
  if (target >= 0 && cubes[target].cube_active) {
    cube_move_to_target(me, target);
  } else {
    // Nothing's available. Just shut down for now.
    cubes[me].life_target_male = -1;
    cubes[me].resource_target = -1;
    if (cubes[me].spatial_velocity > 0.0) printf("cube.cpp: %8s %2d resting.\n", &cubes[me].cube_player[0], me);
    cubes[me].spatial_velocity = 0.0;
  }
  
  return;
}

void strategy_enby(int me) {

  string r = "predator";
  string nr = "resource";
  int target = -1;

  // Check if we're doing a backup motion
  if (backup_active(me)) return;
      
  // Check if we're low on energy and are looking for resources
  int rt = cubes[me].resource_target;
  if (rt >= 0 && cubes[rt].cube_active) {
    target = rt;
  }
  // Check our energy level and if low make sure we switch to foraging
  if (target < 0 && cubes[me].resource_energy < ENERGY_THRESHOLD) {
    target = find_nearest(me, nr);
    if (target >= 0) {
      // We've found an energy resource. Switch to it
      cubes[me].resource_target = target;
      // Cancel any old target, no obsolete
      cubes[me].match_target = -1;
      printf("cube.cpp: %8s %2d low on energy, switching to find %s %d\n", &cubes[me].cube_player[0], me, &cubes[target].cube_player[0], target);
    }
  }
  // If we still don't have a target, see if we are using a match target
  int mt = cubes[me].match_target;
  if (target < 0 && mt >= 0 && cubes[mt].cube_active) {
    target = mt;
  }
  // No target yet. See if we can find one.
  if (target < 0) {
    target = find_nearest(me, r);
    cubes[me].match_target = target;
    if (target >= 0) printf("cube.cpp: %8s %2d hunting %s %d near me\n", &cubes[me].cube_player[0], me, &r[0], target);
  }
  if (target >= 0 && cubes[target].cube_active) {
    cube_move_to_target(me, target);
  } else {
    // Nothing's available. Just shut down for now.
    cubes[me].life_target_male = -1;
    cubes[me].match_target = -1;
    cubes[me].resource_target = -1;
    if (cubes[me].spatial_velocity > 0.0) printf("cube.cpp: %8s %2d resting.\n", &cubes[me].cube_player[0], me);
    cubes[me].spatial_velocity = 0.0;
  }
  
  return;
}

void strategy_predator(int me) {

  string m = "male";
  string f = "female";
  string e = "enby";

  // Check if we're doing a backup motion
  if (backup_active(me)) return;
      
  int target = -1;
  int mt = cubes[me].match_target;
  if (mt >= 0 && cubes[mt].cube_active) {
    target = mt;
  } else {
    int target_m = find_nearest(me, m);
    int target_f = find_nearest(me, f);
    int target_e = find_nearest(me, e);
    float distance = 1000000.00;
    if (target_m >= 0 && cubes[target_m].cube_scale_factor < cubes[me].cube_scale_factor) {
      float td = cube_distance(me, target_m);
      if (td < distance) {
	distance = td;
	target = target_m;
      }
    }
    if (target_f >= 0 && cubes[target_f].cube_scale_factor < cubes[me].cube_scale_factor) {
      float td = cube_distance(me, target_f);
      if (td < distance) {
	distance = td;
	target = target_f;
      }
    }
    if (target_e >= 0 && cubes[target_e].cube_scale_factor < cubes[me].cube_scale_factor) {
      float td = cube_distance(me, target_e);
      if (td < distance) {
	distance = td;
	target = target_e;
      }
    }
    cubes[me].match_target = target;
    if (target >= 0) printf("cube.cpp: %8s %2d stalking %s %d near me\n", &cubes[me].cube_player[0], me, &cubes[target].cube_player[0], target);
  }
  if (target >= 0 && cubes[target].cube_active) {
    cube_move_to_target(me, target);
  } else {
    // Nobody to prey on. Just shut down.
    cubes[me].match_target = -1;
    if (cubes[me].spatial_velocity > 0.0) printf("cube.cpp: %8s %2d resting.\n", &cubes[me].cube_player[0], me);
    cubes[me].spatial_velocity = 0.0;
  }
  
  return;
}

void strategy_resource(int me) {

  // The "strategy" for a resource is very simple. If there is no enrgy left, take it off the plaaying field.

  if (cubes[me].resource_energy < 10.0) {
    cubes[me].cube_active = 0;
    cubes[me].cube_display = 0;
  }

  return;
}
