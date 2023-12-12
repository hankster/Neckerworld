// Structures and defines for interaction with the client application

struct LoginRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  string ground_uuid;
  string username;
  string password;

};

struct LogoutRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  string ground_uuid;

};

struct MoveRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  float spatial_angle;
  float spatial_direction;
  bool spatial_direction_active;
  float velocity;
  float distance;
  glm::vec2 gaze;
  
};

struct StatusRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  
};

struct ViewRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  float spatial_angle;
  glm::vec2 gaze;
  
};

struct GroundViewRequest {

  string message_type;
  unsigned sequence;
  double timestamp;
  string ground_uuid;
  int groundview;
  
};

struct ImportJSONFileRequest {
  
  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  string JSONFilename;
  bool add_JSON;
  bool in_line;

};

struct ImportJSONObjectRequest {
  
  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  string JSONObject;
  bool add_JSON;
  bool in_line;

};

struct LoginResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  string ground_uuid;
  unsigned frame;
  
};

struct MoveResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  unsigned frame;
  bool spatial_position_blocked;
  GLfloat resource_energy;
  
};

struct StatusResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  unsigned frame;
  GLint cube_index;
  string cube_player;
  string cube_firstname;
  bool cube_active;
  bool cube_display;
  bool cube_remote;
  GLfloat cube_scale_factor;
  float spatial_angle;
  float spatial_direction;
  bool spatial_direction_active;
  glm::vec2 spatial_gaze;
  GLfloat spatial_radius;
  GLfloat spatial_distance;
  GLfloat spatial_distance_previous;
  GLfloat spatial_velocity;
  bool spatial_position_blocked;
  time_t life_birth;
  time_t life_death;
  string life_father;
  string life_mother;
  GLfloat resource_energy;
  GLfloat total_points[4];
  
};

struct ViewResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  unsigned frame;
  GLint cubeview;
  string extension;
  string mode;
  int width;
  int height;
  int channels;
  unsigned pixels_frame;
  unsigned pixels_frame_start;
  GLfloat bounding_box[4];
  std::vector<uint8_t> pixels;
  
};

struct GroundViewResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string ground_uuid;
  unsigned frame;
  int groundview;
  string extension;
  string mode;
  int width;
  int height;
  int channels;
  unsigned pixels_frame;
  GLfloat bounding_box[4];
  std::vector<uint8_t> pixels;
  
};

struct ImportJSONResponse {

  string message_type;
  unsigned sequence;
  double timestamp;
  string cube_uuid;
  
};



// Message passing to/from the outside
string cube_decode_message(char* msg, int msglen);
string doLoginRequest(LoginRequest r);
string doLogoutRequest(LogoutRequest r);
string doMoveRequest(MoveRequest r);
string doStatusRequest(StatusRequest r);
string doViewRequest(ViewRequest r);
string doGroundViewRequest(GroundViewRequest r);
string doImportJSONFileRequest(ImportJSONFileRequest r);
string doImportJSONObjectRequest(ImportJSONObjectRequest r);

// Login credentials check
bool check_login(LoginRequest msg);

// Cube status
StatusResponse cube_status(string cube_uuid);

// View request
ViewResponse screenview(string cube_uuid, float angle, float gaze_yaw, float gaze_pitch);
GroundViewResponse ground_screenview(string ground_uuid, int groundview);

