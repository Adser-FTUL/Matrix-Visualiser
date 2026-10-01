/*
 * Matrix Visualiser with OpenGL and GLFW
 * 
 * Usage:
 *    ./transformation <ihat_x> <ihat_y> <jhat_x> <jhat_y>
 *    For the form of a 2x2 matrix as so:
 *	      [ihat_x	ihat_y]		
 *		  [jhat_x   jhat_y]
 *	  Leave arguement empty for identity matrix	
 *
 * Description:
 *     Matrix Visualiser that takes in an arguement from a user in the form of a 2x2 matrix.
 *
 * Author: Adam K
 * 
 *
 * Dependencies:
 *     - GLFW3 library
 *     - GLEW
 *     - OpenGL libraries
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"

#define MAX_GRIDS 50000
#define DEFAULT_GRID_SPACING 10;

GLuint gridVAO, gridVBO;
float grid_vertices[MAX_GRIDS];
float x_spacing = 0.1f;
float y_spacing = 0.1f;
int x_num_grids;
int y_num_grids;

// Vertex shader to transform vertices and pass texture coordinates.
const char* vertex_shader_source =
    "#version 330 core\n"
    "\n"
    "layout(location = 0) in vec2 in_position;\n"
	"layout(location = 1) in vec3 in_color;\n"
	"out vec3 vertex_color;\n"
	"uniform mat4 transform;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = transform * vec4(in_position, 0.0, 1.0);\n"
	"	 vertex_color = in_color;\n"
    "}\n";

// Fragment shader to generate dynamic cloud rendering effect.
const char* fragment_shader_source =
    "#version 330 core\n"
	"in vec3 vertex_color;\n"
    "out vec4 frag_color;\n"
    "\n"
    "void main() {\n"
    "    frag_color = vec4(vertex_color, 1.0f);\n"
    "}\n";

// Vertex data for a full-screen quad (two triangles covering the screen).
// Each vertex has a position and texture coordinate.
float vertices[] = {
        // positions          // texture coords
        -1.0f,  1.0f,         1.0f, 1.0f, 1.0f,  // top left
        -1.0,   -1.0f,        1.0f, 1.0f, 1.0f,  // bottom left
         1.0f, -1.0f,         1.0f, 1.0f, 1.0f,  // bottom right 
         1.0f,  1.0f,         1.0f, 1.0f, 1.0f   // top right
    };

unsigned int indices[] = {
	0, 1, 2, // first triangle
	0, 2, 3, // second triangle
	};

// Vertex data for grid lines
float line_vertices[] = {
	// positions    // rgb
	 1.0f,  0.0f,	0.0f, 0.0f, 0.0f,
	-1.0f,  0.0f,	0.0f, 0.0f, 0.0f,
	 0.0f,  1.0f,	0.0f, 0.0f, 0.0f,
	 0.0f, -1.0f,   0.0f, 0.0f, 0.0f
};

// Helper function to compile a shader from source.
GLuint compile_shader(const char* source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    const GLchar* shader_source = source;
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Error compiling shader: %s\n", info_log);
        return 0;
    }
    return shader;
}

void create_grid(float* vertices) {
    int idx = 0;
	x_num_grids = 1 / x_spacing;
	y_num_grids = 1 / y_spacing;
	printf("x_num_grids: %d\n", x_num_grids);
	printf("y_num_grids: %d\n", y_num_grids); 
	printf("x_spacing: %lf\ny_spacing: %lf\n", x_spacing, y_spacing);
    
    // Horizontal lines
    for (int i = -x_num_grids; i <= x_num_grids; i++) {
        float y = i * x_spacing;
        
        // Start point - need 5 separate assignments
        vertices[idx++] = -1.0f;  // x
        vertices[idx++] = y;      // y
        vertices[idx++] = 0.5f;   // r
        vertices[idx++] = 0.5f;   // g
        vertices[idx++] = 0.0f;   // b
        
        // End point
        vertices[idx++] = 1.0f;   // x
        vertices[idx++] = y;      // y
        vertices[idx++] = 0.5f;   // r
        vertices[idx++] = 0.0f;   // g
        vertices[idx++] = 0.5f;   // b
    }

    // Vertical lines
    for (int i = -y_num_grids; i <= y_num_grids; i++) {
        float x = i * y_spacing;
        
        // Start point
        vertices[idx++] = x;      // x
        vertices[idx++] = -1.0f;  // y
        vertices[idx++] = 0.5f;   // r
        vertices[idx++] = 0.5f;   // g
        vertices[idx++] = 0.5f;   // b
        
        // End point
        vertices[idx++] = x;      // x
        vertices[idx++] = 1.0f;   // y
        vertices[idx++] = 0.5f;   // r
        vertices[idx++] = 0.5f;   // g
        vertices[idx++] = 0.5f;   // b
    }
	printf("Size of grid vertex: %d\n", idx);
}

void init_grid() {
	glGenVertexArrays(1, &gridVAO);
	glGenBuffers(1, &gridVBO);
	
	create_grid(grid_vertices);


	glBindVertexArray(gridVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), grid_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);    // Unbind the VBO and VAO for now.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void regenerate_grid() {
	memset(grid_vertices, 0, sizeof(grid_vertices));
	create_grid(grid_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), grid_vertices, GL_STATIC_DRAW);
}

// Function to test keyboard input for GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool valid_input = false;

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_UP && x_spacing <= 1) {
			x_spacing += 0.01f;
			valid_input = true;
		}
		else if (key == GLFW_KEY_DOWN && x_spacing >= 0) {
			x_spacing -= 0.01f;
			valid_input = true;
		}
		
		else if (key == GLFW_KEY_LEFT && y_spacing <= 1) {
			y_spacing += 0.01f;
			valid_input = true;
		}
		else if (key == GLFW_KEY_RIGHT && y_spacing >= 0) {
			y_spacing -= 0.01f;
			valid_input = true;
		}
	
		if (valid_input) {
			regenerate_grid();
		}
	}
}

int main(int argc, char *argv[]) {
	// Identity Matrix, if user doesn't enter a 2x2 matrix the Identity Matrix will be used
    float ihat_x = 1;
	float ihat_y = 0;
	float jhat_x = 0;
	float jhat_y = 1;
	
	if (argc == 5) {
		ihat_x = atof(argv[1]);
		ihat_y = atof(argv[2]);
		jhat_x = atof(argv[3]);
		jhat_y = atof(argv[4]);
	}
	else if (argc != 1) {
		printf("Incorrect usage\n");
		return 0;
	}
	

    // Create a windowed mode window and its OpenGL context.
	GLFWwindow* window;


	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	// Initialize GLFW library.
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 960, "Matrix Visualiser", NULL, NULL);	

    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current.
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

	glfwSetKeyCallback(window, key_callback);
    
	// Compile the vertex and fragment shaders.
    GLuint vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

    // Link the shaders into a single program.
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        fprintf(stderr, "Error linking shader program: %s\n", info_log);
    }

    // Get the uniform location for cloud_shift in the shader.
	GLint transform_loc = glGetUniformLocation(shader_program, "transform");

    // Clean up shaders as they're no longer needed.
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Generate a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO).
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

    // Bind the VAO to capture all subsequent vertex attribute configurations.
    glBindVertexArray(VAO);

    // Bind the VBO to the array buffer and populate it with the vertex data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Specify the layout of the vertex data. First, the positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Then, the texture coordinates.
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
	
	// Origin Lines
	GLuint lineVAO, lineVBO;
	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Grid Lines
	
	
	init_grid();
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	float rotation_angle = 0.0f;
	int rotate_direction = 1;  // 1 = clockwise, -1 = counter-clockwise
    
	// Render loop.
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen buffer.
        glClear(GL_COLOR_BUFFER_BIT);

		rotation_angle += 4.0f * rotate_direction;
		if (rotation_angle > 360.0f) rotation_angle -= 360.0f;
		if (rotation_angle < 0.0f) rotation_angle += 360.0f;
		

        // Use the compiled shader program.
        glUseProgram(shader_program);

        // Bind the VAO (with the quad data) and render.
		mat4 triangle_transform = GLM_MAT4_IDENTITY_INIT;
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, (float*)triangle_transform);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		mat4 grid_transform = GLM_MAT4_IDENTITY_INIT;
		
		// Applying the user Matrix
		grid_transform[0][0] = ihat_x;
		grid_transform[1][0] = ihat_y;
		grid_transform[0][1] = jhat_x;
		grid_transform[1][1] = jhat_y;
		
	//	printf("%lf %lf %lf %lf\n", grid_transform[0][0], grid_transform[0][1], grid_transform[1][0], grid_transform[1][1]);

		// glm_rotate(grid_transform, glm_rad(rotation_angle), (vec3){0.0f, 0.0f, 1.0f});
		
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, (float*)grid_transform);

		glBindVertexArray(gridVAO);
		glDrawArrays(GL_LINES, 0, (((x_num_grids * 2 + 1) + (y_num_grids * 2 + 1)) * 2));

		mat4 line_transform = GLM_MAT4_IDENTITY_INIT;
		glUniformMatrix4fv(transform_loc, 1, GL_FALSE, (float*)line_transform);
		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINES, 0, 4);

        // Swap the screen buffers and poll for events.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and terminate GLFW.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glDeleteVertexArrays(1, &lineVAO);
	glDeleteBuffers(1, &lineVBO);

    glDeleteProgram(shader_program);
    glfwTerminate();
    return 0;
}
