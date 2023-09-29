// Our light information
struct Light {
    vec3 position;
    vec3 intensity;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

// Up to NL lights supported
#define NL 5

// Structure of a material property
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

varying vec4 f_position;
varying vec3 f_normal;
varying vec2 f_coordtexture;
uniform sampler2D texture;
uniform int object_type;
uniform vec4 object_color;
uniform Light lights[NL];  
uniform int n_lights;
uniform Material material;
uniform vec3 view_position;

vec3 norm;
vec3 light_direction;

// Compute the distance between this fragment and the point source of light and derive attenuation.
float compute_attenuation(int lx)
{
    float distance = length(lights[lx].position - vec3(f_position));
    float attenuation = 1.0 / (lights[lx].constant + lights[lx].linear * distance + lights[lx].quadratic * (distance * distance));
    return attenuation;
}

// Compute the ambient light contribution from this source.
vec3 compute_ambient(int lx)
{
    vec3 ambient = lights[lx].intensity * lights[lx].ambient * material.ambient;
    return ambient;
}

// Compute the diffuse light contribution from this source.
vec3 compute_diffuse(int lx)
{
    light_direction = normalize(lights[lx].position - vec3(f_position));
    float diff = max(dot(f_normal, light_direction), 0.0);
    vec3 diffuse = diff * lights[lx].intensity * lights[lx].diffuse * material.diffuse;
    return diffuse;
}

// Compute the specular light contribution from this source.
vec3 compute_specular(int lx)
{
    vec3 view_direction = normalize(view_position - vec3(f_position));
    vec3 reflect_direction = reflect(-light_direction, norm);  
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), material.shininess);
    vec3 specular = lights[lx].intensity * lights[lx].specular * (spec * material.specular);
    return specular;
}

void main(void) {

  float attenuation;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 result;
  
  // Flip y coordinate to match texture
  vec2 flipped_coordtexture = vec2(f_coordtexture.x, 1.0 - f_coordtexture.y);

if (object_type == 0 || object_type == 1) {

    // Change fragment color here
    gl_FragColor = vec4(lights[0].intensity, 1.0f) * object_color;
    return;

  }

  /* Only use texture mapping */
  if (object_type == 2) {

    gl_FragColor = vec4(lights[0].intensity, 1.0) * texture2D(texture, flipped_coordtexture);
    return;

  } /* if (object_type == 2) ... */

  /* Use diffuse + ambient light */
  if (object_type == 3) {
 
    // Iterate through all our light sources.
    for ( int i=0; i < n_lights; ++i) {

    	// Compute attunuation
	attenuation = compute_attenuation(i);

	vec3 diff = attenuation * compute_diffuse(i);
	diffuse = diffuse + diff;

	// ambient
    	vec3 amb = compute_ambient(i);
	ambient = ambient + amb;

	}

} /* if (object_type == 3) ... */

  /* Use ambient + diffuse + specular */
  if (object_type == 4) {

    // Iterate through all our light sources.
    for ( int i=0; i < n_lights; ++i) {

	// ambient
    	vec3 amb = compute_ambient(i);
	ambient = ambient + amb;
  	
 	// diffuse
    	vec3 diff = compute_diffuse(i);
	diffuse = diffuse + diff;
    
	// specular
    	vec3 spec = compute_specular(i);
	specular = specular + spec;
	
	} /* for ( i=0; ... */
  } /* if (object_type == 4) ... */

  /* Use ambient + diffuse + specular with attenuation */
  if (object_type == 5) {

    // Iterate through all our light sources.
    for ( int i=0; i < n_lights; ++i) {

	// ambient
    	vec3 amb = compute_ambient(i);
	ambient = ambient + amb;
  	
 	// diffuse
    	vec3 diff = compute_diffuse(i);
	diffuse = diffuse + diff;
    
	// specular
    	vec3 spec = compute_specular(i);
	specular = specular + spec;
	
    	// attenuation
    	attenuation = compute_attenuation(i);
    
    	ambient  = attenuation * ambient; 
    	diffuse  = attenuation * diffuse;
    	specular = attenuation * specular;

	} /* for ( i=0; ... */
  } /* if (object_type == 5) ... */

  result = ambient + diffuse + specular;

  // vec3 color_white = vec3(1.0f, 1.0f, 1.0f);
  // vec3 color_red = vec3(1.0f, 0.0f, 0.0f);
  // vec3 color_green = vec3(0.0f, 1.0f, 0.0f);
  // vec3 color_blue = vec3(0.0f, 0.0f, 1.0f);
  // vec3 color_black = vec3(0.0f, 0.0f, 0.0f);

  // if (f_normal.z > 0.8f) result = color_white;
  // if (f_normal.z > 0.6f && f_normal.z <= 0.8f) result = color_red;
  // if (f_normal.z > 0.4f && f_normal.z <= 0.6f) result = color_green;
  // if (f_normal.z > 0.0f && f_normal.z <= 0.4f) result = color_blue;
  // if (f_normal.z <= 0.0f) result = color_black;

  gl_FragColor = vec4(result, 1.0) * texture2D(texture, flipped_coordtexture);

  return;
}