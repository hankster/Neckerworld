/* Structures and Function prototypes for cube.cpp */

/* Spatial Waypoint definition */
struct spatial_waypoint {
  time_t seconds;
  int spatial_event;
  bool spatial_position_blocked;
  glm::vec3 spatial_position;
};

#define IDLE 0 
#define MOVEMENT 1
#define BLOCKED 2
#define REVERSE 3

/* Structure to hold object description and state. */
struct Cube {
  GLuint vbo_cube_vertices;
  GLuint vbo_cube_normals;
  GLuint vbo_cube_texture_map_coords;
  GLuint ibo_cube_elements;
  GLuint timestamp; // Time of birth in seconds.
  int cube_index; // Index number in the list of cubes.
  string cube_player; // Class of players for this cube ("male", "female", ...).
  string cube_uuid; // Unique ID for this cube.
  string cube_emoticon; // Emoticon for the cube face.
  string cube_firstname; // Cube's nickname.
  bool cube_active; // Cube is an active participant in the game.
  bool cube_display;  // Display this cube on the playing field.
  bool cube_remote; // Cube is under remote control of a client program.
  GLfloat cube_scale_factor; // Size of a cube (default 0.5 to 1.2).
  glm::mat4 cube_scale; // Scaler matrix.
  GLint cube_type; // Characteristic structure for this cube.
  string cube_color_class; // Color class ("blue", "white", ...).
  glm::vec4 cube_color; // Specific color for cube skin.
  GLint cube_material; // Material property for cube surfaces.
  string cube_surface; // Description of cube surface texture.
  GLint cube_texture_index; // Texture used for the cube skins.
  GLfloat cube_texture_map[6*4*2]; // Texture mapping coordinates.
  //cube_parameters    { ... }
  glm::vec3 spatial_position; // Location in space for the cube.
  glm::vec3 spatial_position_previous; // Previous location (needed for collision checking and blocked testing).
  glm::mat4 spatial_translation; // Matrix for translation.
  glm::vec3 spatial_rotation; // Rotation angles.
  glm::mat4 spatial_rotation_matrix; // Rotation matrix.
  float spatial_direction; // Direction to travel (used only during a backup operation). Spatial rotation Y defines normal direction of travel.
  bool spatial_direction_active; // When true, direction of travel is set by spatial_direction (otherwise spatial rotation).
  glm::vec2 spatial_gaze; // Horizontal and vertical view angle (yaw, pitch). Allows cube to look around without moving.
  GLfloat spatial_radius; // Minimal distance from the center of a cube to enclose it (i.e. 1/2 diagonal).
  glm::vec3 spatial_destination; // Target location.
  GLfloat spatial_distance; // Desired distance to travel.
  GLfloat spatial_distance_previous; // Distance to travel previous (needed for collisions and backups).
  GLfloat spatial_velocity; // Speed at which cube will travel.
  GLint spatial_position_timer; // Timer to countdown a backup move. Blocking collisions are discarded.
  bool spatial_position_blocked; // Cube has stopped after hitting another cube or running off the edge of the playing field.
  std::vector<spatial_waypoint> spatial_waypoints; // List of recorded positions (for waypoints.tsv)
  GLint life_target_male; 
  GLint life_target_female;
  time_t life_birth; // Time of birth.
  time_t life_death; // Time of death
  string life_father; // Father's UUID.
  string life_mother; // Mother's UUID
  std::vector<std::string> life_mates; // List of mates.
  std::vector<std::string> life_children; // Progeny
  std::vector<time_t> life_mate_times; // When mating was done.
  std::vector<GLfloat> life_energy; // Energy lost.
  GLint resource_target;
  GLfloat resource_energy; // Energy meter.
  std::vector<std::string> resource_list; // List of resources visited.
  std::vector<time_t> resource_times; // Times when resources were visited.
  std::vector<GLfloat>resource_capture; // Resource energy captured.
  GLint match_target; // Cube index for current target.
  std::vector<std::string> match_opponent; // UUID of opponents encountered.
  std::vector<time_t> match_times; // Times for combat.
  std::vector<GLfloat> match_energy; // Energy gained from combat.
  std::vector<bool> match_results; // Who won.
  GLfloat bounding_box[4]; // Bounding box coordinates.
  GLint paintball_count; // Experimental, not yet implemented.
  GLint paintball_type; // Experimental, not yet implemented.
  glm::vec3 paintball_color; // Experimental, not yet implemented.
  GLfloat paintball_bearing; // Experimental, not yet implemented.
  GLfloat paintball_elevation; // Experimental, not yet implemented.
  glm::mat4 cube_model; // Matrix for current cube model.
  glm::mat4 cube_mvp; // Matrix for current (model * view * position).
};

struct Wire {
  GLuint vbo_wire_vertices;
  GLint wire_type;
  glm::vec4 wire_color;
  glm::mat4 wire_scale;
  glm::mat4 wire_rotation;
  glm::mat4 wire_translation;
  glm::mat4 wire_model;
  glm::mat4 wire_mvp;
};

struct Ground {
  GLuint vbo_ground_vertices;
  GLuint vbo_ground_normals;
  GLuint vbo_ground_texture_map_coords;
  GLuint ibo_ground_elements;
  bool ground_active;
  bool ground_display;
  bool ground_remote;
  string ground_uuid;
  GLfloat ground_scale_factor;
  glm::mat4 ground_scale;
  GLuint ground_type;
  glm::vec4 ground_color;
  GLuint ground_material;
  string ground_surface;
  GLuint ground_texture_index;
  glm::vec3 ground_spatial_position;
  glm::mat4 ground_spatial_translation;
  glm::vec3 ground_spatial_rotation;
  glm::mat4 ground_spatial_rotation_matrix;
  glm::mat4 ground_model;
  glm::mat4 ground_mvp;
  glm::vec3 ground_view_position;
  glm::vec3 ground_view_target;
  glm::vec3 ground_view_up;
};

struct Light {
  glm::vec3 position;
  glm::vec3 intensity;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float constant;
  float linear;
  float quadratic;
};

struct Material {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  float shininess;
}; 

struct Texture {
  GLuint texture_array;
  GLint width;
  GLint height;
  GLint bytes_per_pixel; /* 3:RGB, 4:RGBA */
  GLuint format;
};

/* Uniforms for our array of point lights */
struct Ul {
  GLint position;
  GLint intensity;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint constant;
  GLint linear;
  GLint quadratic;
};

// Global vaiables -- most located in cube.cpp
extern GLFWwindow* window;
extern GLFWwindow* windows[];
extern GLuint VertexArrayObject;
extern GLuint VertexArrayObjects[];

/* Main Window settings (This is the ground view.) */
extern const char * window_title;
extern int main_window_width;
extern int main_window_height;
extern int main_window_channels;
extern float window_background_color_r;
extern float window_background_color_g;
extern float window_background_color_b;
extern float window_background_color_a;

/* Image size used for transmission to client, object detection and analysis */
extern int raster_width;
extern int raster_height;

/* World scene variables */
extern glm::mat4 view;
extern glm::mat4 projection;

/* Camera position and target settings */
extern glm::vec3 camera_position_default;
extern glm::vec3 camera_target_default;
extern glm::vec3 camera_up_default;
extern glm::vec3 camera_position;
extern glm::vec3 camera_target;
extern glm::vec3 camera_up;

/* Frame counter and timestamp */
extern double frames_per_second;
extern GLuint frame_counter;
extern double frame_time;

/* json import parameters */
extern string json_import_file;
extern string json_import_object;
extern bool json_flag_file;
extern bool json_flag_object;
extern int json_cube_id;
extern string json_cube_uuid;

/* Cube genrating parameters */
extern int n_cubes;
extern Cube cubes[];
extern GLfloat cube_normals[];

/* Wire cube generating parameters */
extern int n_wires;
extern Wire wires[];

/* Ground generation parameters */
extern int n_grounds;
extern Ground grounds[];

/* Light generating parameters */
extern int n_lights;
extern Light lights[];

/* Material definitions */
extern int n_materials;
extern Material materials[];

/* Texture related definitions*/
extern int n_textures;
extern Texture textures[];

/* Normal vector parameters */
extern GLuint vbo_normals;
extern glm::mat4 normals_model;
extern glm::mat4 normals_mvp;

/* Normal vector parameters */
extern GLuint vbo_normals;
extern glm::mat4 normals_model;
extern glm::mat4 normals_mvp;

/* Shader programs */
extern GLuint program;
extern GLint attribute_coord3d;
extern GLint attribute_normal;
extern GLint attribute_coordtexture;
extern GLint uniform_model;
extern GLint uniform_mvp;
extern GLint uniform_object_texture;
extern GLint uniform_object_type;
extern GLint uniform_object_color;
extern GLint uniform_view_position;

/* Uniforms for our array of point lights */
/* With structs need to do it by each member */
extern GLint uniform_n_lights;
extern Ul uniform_lights[];

/* Material uniforms */
extern GLint uniform_material_ambient;
extern GLint uniform_material_diffuse;
extern GLint uniform_material_specular;
extern GLint uniform_material_shininess;;

/* For debugging drawing code */
extern GLint debug;
extern bool   clock_time;
extern GLint  view_index;
extern GLuint diagnostic;
extern GLuint display_cubes;
extern GLuint display_wires;
extern GLuint display_normals;
extern GLuint display_grounds;
extern GLuint display_rotation;
extern GLuint display_rotation_x;
extern GLuint display_rotation_y;
extern GLuint display_rotation_z;
extern GLuint position;
extern GLuint strategy;
extern GLuint strategy_basic;
extern GLuint strategy_extended;
extern GLuint transparency;
extern bool   video;

/* Constants */
extern float pi;

/* Colors */
extern glm::vec4 color_red;
extern glm::vec4 color_green;
extern glm::vec4 color_blue;
extern glm::vec4 color_purple;
extern glm::vec4 color_black;
extern glm::vec4 color_white;

// Function located in cube.cpp
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void window_error(int error, const char* message);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
float random1();
void screenshot(GLFWwindow* window, string filename);
void screenview_capture(GLFWwindow* window, int i, int notify, std::vector<uint8_t>* pixels, unsigned* pixels_frame, unsigned* pixels_frame_start, std::mutex* pixels_mutex);
void update_projection(int width, int height);

// Multi-thread server
int server_main(int workers, int port, string ip);
void server_loop();
void server_stop();

// Function located in cube_data.cpp
int json_import(char* jsonfile);
int json_import(string jsonobject);

// Function located in cube_engine.cpp
void display();
void debug_cubes(int i);
void debug_grounds(int i);
void debug_lights(int i);
void debug_materials(int i);

// Function located in cube_objects.cpp
// Object creation
// Create a new cube
int new_cube(int nc, string cube_player, string cube_uuid, string cube_emoticon, string cube_firstname, float cube_scale_factor, int cube_type, string cube_color_class, glm::vec4 cube_color, int cube_material, string cube_surface, int cube_texture_index, float cube_texture_map[6*4*2], float x, float y, float z, float rotX, float rotY, float rotZ, float spatial_radius, float resource_energy);
// Create a new wire cube
int new_wire(int nw, float wire_scale_factor, float x, float y, float z, float rotX, float rotY, float rotZ, int type, glm::vec4 color);
// Create a new ground plane
int new_ground(int ng, string ground_uuid, GLfloat ground_scale_factor, GLuint ground_type, glm::vec4 ground_color, GLuint ground_material, GLuint ground_texture_index, float ground_texture_map[8], float x, float y, float z, float rotX, float rotY, float rotZ, float viewX, float viewY, float viewZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ);
// Create a new light
int new_light(int light_index, glm::vec3 position, glm::vec3 intensity, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);
// Create a new material
int new_material(int material_index, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);
// Create a new texture map
int new_texture(int index, const char *filename);
// Create a baby cube
bool cube_mates(int male, int female);

// Position related
void cube_update_scale(int i, GLfloat cube_scale_factor);
void cube_update_size(int i);
void cube_update_position(int i);
void cube_update_angle(int i, GLfloat angle);
void cube_update_rotation(int i, GLfloat rotX, GLfloat rotY, GLfloat rotZ);
void cube_update_translation(int i);
void cube_update_model(int i);
void cube_update_view(int i);
void cube_update_mvp(int i);
void cube_bounding_box(int i, int width, int height);
GLfloat cube_distance(int i, int j);
void cube_move_to_target(int i, int target);
void cube_block_position(int i);
int cube_remote_move(int cube_index, float angle, float direction, bool direction_active, float distance, float velocity, glm::vec2 gaze);
void cube_update_waypoints(int i, int waypoint_event);

// UUID
int cube_get_index(string uuid);
int ground_get_index(string uuid);
string get_uuid();

// Energy related
void cube_energy_check(int i);
void cube_energy_cost(int i);
void cube_total_points(int i, float *total, float *mate, float *food, float *kill);

GLfloat find_angle(GLfloat dz, GLfloat dx);
int find_nearest_me(int i, string type);

void ground_update_model(int i);
void ground_update_mvp(int i);

int show_normals();

// Functions located in cube_simulation.cpp
// Prototypes for action routines
bool cube_contact(int i, int j);
int cube_contact_index(string player);
bool cube_contact_evaluation(int i, int j);
bool cube_contact_no_action(int player1, int player2);
bool cube_contact_mate(int male, int female);
bool cube_contact_attack(int player, int predator);
bool cube_contact_resource(int player, int resource);

// Strategy
void strategy_male(int i);
void strategy_female(int i);
void strategy_enby(int i);
void strategy_predator(int i);
void strategy_resource(int i);

/* Default window size */
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define WINDOW_CHANNELS 4

/* Size of image used for transmission, object identification */
#define IMAGE_RASTER_WIDTH 512
#define IMAGE_RASTER_HEIGHT 512

/* Multi-thread server */
#define MT_WORKERS 30
#define MT_PORT 2020
#define MT_IP ""

/* Default frames per second */
#define DEFAULT_FPS 6.5

/* Cube genrating parameters */
#define NC 50

/* Wire cube generating parameters */
#define NW 20

/* Ground generation parameters */
#define NG 5

/* Light generating parameters */
#define NL 5

/* Material definitions */
#define NM 5

/* Texture related definitions*/
#define NT 52

// Motion related
#define VELOCITY_MAX 5.0

// Various energy limits
#define ENERGY_THRESHOLD 50.0f
#define ENERGY_MAX_TRANSFER 200.0f
#define ENERGY_LIMIT 1000.0f
#define ENERGY_COST 0.01f
#define ENERGY_FACTOR 0.16f

// Various life parameters
#define LIFE_MATE_HOLD_TIME 30.0
#define LIFE_YOUTH_HOLD_TIME 60.0

