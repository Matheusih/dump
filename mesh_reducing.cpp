// Include standard headers
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include "windows.h" 


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* g_pWindow;
unsigned int g_nWidth = 1024, g_nHeight = 768;

// Include AntTweakBar
#include <AntTweakBar.h>
TwBar *g_pToolBar;

// Include GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <glerror.hpp>

#include <cmath>   


std::vector<glm::vec3> createPriorityList_test(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices);
std::vector<glm::vec3> findVerticesInRange(std::vector<glm::vec3> vertices, float range, glm::vec3 center, std::vector<unsigned short> indices);
std::vector<glm::vec3> updateNormals(std::vector<glm::vec3> normals, std::vector<glm::vec3> vertices, std::vector<unsigned short> indices);
glm::vec3 computeNormal(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & c);
bool is_in_vector(std::vector<glm::vec3> vertices, glm::vec3 target);
int myfloor(float x);
glm::vec3 midpoint(glm::vec3 a, glm::vec3 b);
std::vector<glm::vec3> createPriorityList(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices);
std::vector<unsigned short> updateIndices(std::vector<glm::vec3> vertices, std::vector<unsigned short> indices, glm::vec3 v1, glm::vec3 v2);
std::vector<glm::vec3> createPriorityList_Clustering(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices);
int getIndex(glm::vec3 v1, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices);

std::vector<glm::vec3> clustering_remove(
	std::vector<glm::vec3> vertices,
	std::vector<unsigned short> indices,
	float range,
	glm::vec3 v1
);

std::vector<glm::vec3> edge_collapse(
	std::vector<glm::vec3> vertices,
	std::vector<unsigned short> indices,
	glm::vec3 v1,
	glm::vec3 v2
);

void WindowSizeCallBack(GLFWwindow *pWindow, int nWidth, int nHeight) {

	g_nWidth = nWidth;
	g_nHeight = nHeight;
	glViewport(0, 0, g_nWidth, g_nHeight);
	TwWindowSize(g_nWidth, g_nHeight);
}

//*****************************
// PRESS U TO DECIMATE MESH~~~~
//****************************
int main(void)
{
	int nUseMouse = 0;

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	g_pWindow = glfwCreateWindow(g_nWidth, g_nHeight, "CG UFPel", NULL, NULL);
	if (g_pWindow == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(g_pWindow);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	check_gl_error();//OpenGL error from GLEW

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(g_nWidth, g_nHeight);

	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback(g_pWindow, (GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetCursorPosCallback(g_pWindow, (GLFWcursorposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetScrollCallback(g_pWindow, (GLFWscrollfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback(g_pWindow, (GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback(g_pWindow, (GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar
	glfwSetWindowSizeCallback(g_pWindow, WindowSizeCallBack);

	//create the toolbar
	g_pToolBar = TwNewBar("CG UFPel ToolBar");
	// Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
	double speed = 0.0;
	TwAddVarRW(g_pToolBar, "speed", TW_TYPE_DOUBLE, &speed, " label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");
	// Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
	vec3 oColor(0.0f);
	TwAddVarRW(g_pToolBar, "bgColor", TW_TYPE_COLOR3F, &oColor[0], " label='Background color' ");

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(g_pWindow, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(g_pWindow, g_nWidth / 2, g_nHeight / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture = loadDDS("mesh/uvmap.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;    //vertex array
	std::vector<glm::vec2> uvs;			//coordinates representing the 3D object projected in a 2D plane OR color mapping regarding vectors?
	std::vector<glm::vec3> normals;		//normal vectors, 1 for each face
	bool res = loadOBJ("mesh/suzanne.obj", vertices, uvs, normals);
	//vertices = reduceMesh(vertices, uvs, normals);
	//normals = updateNormals(normals, vertices);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	std::vector<glm::vec3> original_model_v = indexed_vertices;
	std::vector<glm::vec3> original_model_n = indexed_normals;
	std::vector<unsigned short> original_model_i = indices;

	double lastTime = glfwGetTime();
	int nbFrames = 0;
	std::vector<glm::vec3> priorList;
	priorList = createPriorityList_test(indexed_vertices, indices);    // smallest edges first
	printf("Generated a priority list with %zu edges!\n", priorList.size() / 2);
	printf("Model has %zu vertices\n", indices.size());
	//priorList = createPriorityList_Clustering(indexed_vertices, indices);    //bigger num of neighbours first
	int i1 = 0, i2 = 1, wireframe = 0, target = 0;  //utils
	glm::vec3 last1, last2, last_vertex;

	last_vertex = priorList[priorList.size() - 1];

	float range = 0.1f;  // the range in which we grab vertices around our cluster cell
	int vertex_update_rate = 10;  // the higher the less the vertex will move in direction of cluster cell! if 1 it will move right away!

	std::vector<glm::vec3> inRange;  //finds all vertices around target
	inRange = findVerticesInRange(indexed_vertices, range, priorList[i1], indices);
	do {
		if (glfwGetKey(g_pWindow, GLFW_KEY_U) == GLFW_PRESS) {

			if ((uint)i1 < (priorList.size()-2) && (uint)i2 < (priorList.size()-2)) {
				if (inRange.size() > 0) {
					for (int i = 0; i < inRange.size(); ++i) {
						if (glm::distance(inRange[i], priorList[i1]) > 0.001f) {   //smooth resolution change! converges the vertices in range until they're very close and then cluster em!
							for (int j = 0; j < indexed_vertices.size(); ++j) {
								if (indexed_vertices[j] == inRange[i]) {
									target = j;
									break;
								}
							}
							indexed_vertices[target].x += (priorList[i1].x - indexed_vertices[target].x) / vertex_update_rate; //updates vertex position in indexed buffer
							indexed_vertices[target].y += (priorList[i1].y - indexed_vertices[target].y) / vertex_update_rate;
							indexed_vertices[target].z += (priorList[i1].z - indexed_vertices[target].z) / vertex_update_rate;

							inRange[i].x += (priorList[i1].x - inRange[i].x) / vertex_update_rate;   //updates vertex position in inRange buffer
							inRange[i].y += (priorList[i1].y - inRange[i].y) / vertex_update_rate;
							inRange[i].z += (priorList[i1].z - inRange[i].z) / vertex_update_rate;
						}
						else {
							inRange.erase(inRange.begin() + i);
						}
						if (inRange.size() == 0) {
							indexed_vertices = clustering_remove(indexed_vertices, indices, range, priorList[i1]);

						}
					}
				}/*
				else if (priorList[i1] == last1 && priorList[i2] == last2) {
					i1 += priorList.size();
				}*/
				else {
					//indexed_vertices = clustering_remove(indexed_vertices, indices, range, priorList[i1]);
					i1 += 1;
					if (i1 < priorList.size() - 2) {
						inRange = findVerticesInRange(indexed_vertices, range, priorList[i1], indices);
						indices = updateIndices(indexed_vertices, indices, priorList[i1], priorList[i1]);    // change first priorList[i1] to midPoint(priorList[i1],priorList[i2]) if using half edge collapse
					}
					//indexed_vertices = edge_collapse(indexed_vertices, indices, priorList[i1], priorList[i2]);
				}
			}
			else {

				printf("Priority List ended!\n");
				printf("Model has now %zu vertices\n", indices.size());
				if (indices.size() > 3 && indexed_vertices.size() > 3) {
					indexed_normals = updateNormals(normals, indexed_vertices, indices);	// updates normals of model
					printf("CREATING NEW ONE!!\n\n");
					priorList = createPriorityList_test(indexed_vertices, indices);
					//last_vertex = priorList[priorList.size() - 1];
					i1 = 0;
					//i2 = 1;
					//range += 0.005f;  //increases clustering range for further reducing!
					range += range;
				}
				else {
					printf("No further reducing possible\n\n");
				}
			}
			//i1+=1;   // set this guy to 2 if using edge_collapse
			//i2+=2;

			if (indices.size()) {
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
				glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
				glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
			}


		}
		if (glfwGetKey(g_pWindow, GLFW_KEY_R) == GLFW_PRESS) {
			indexed_vertices = original_model_v;  // Restores model if R is pressed
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
			glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
			glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
		}

		
		if (glfwGetKey(g_pWindow, GLFW_KEY_W) == GLFW_PRESS) {
			if (!wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				wireframe = 1;
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				wireframe = 0;
			}
		}
		check_gl_error();

		//use the control key to free the mouse
		if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
			nUseMouse = 1;
		else
			nUseMouse = 0;

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			//printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(nUseMouse, g_nWidth, g_nHeight);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,        // mode
			indices.size(),      // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0             // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Draw tweak bars
		TwDraw();

		// Swap buffers
		glfwSwapBuffers(g_pWindow);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(g_pWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(g_pWindow) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Terminate AntTweakBar and GLFW
	TwTerminate();
	glfwTerminate();

	return 0;
}


//Clustering collapse :: Collapses all vertecis in range into v1
std::vector<glm::vec3> clustering_remove(
	std::vector<glm::vec3> vertices,
	std::vector<unsigned short> indices,
	float range,
	glm::vec3 v1
) {

	std::vector<glm::vec3> inRange = findVerticesInRange(vertices, range, v1, indices);
	for (uint i = 0; i < indices.size(); i++) {
		for (uint j = 0; j < inRange.size(); ++j) {
			if (inRange[j] == vertices[indices[i]]) {
				vertices[indices[i]] = v1;
			}
		}
	}

	return vertices;
}
// Collapses a vertex in either full collapse or half edge  NOTE: IF USING HALF EDGE REMEMBER TO PASS midPoint(v1,v2) to updateIndices. Else indices will not be cleaned.
std::vector<glm::vec3> edge_collapse(
	std::vector<glm::vec3> vertices,
	std::vector<unsigned short> indices,
	glm::vec3 v1,
	glm::vec3 v2
) {
	glm::vec3 midVertex = midpoint(v1, v2);

	for (uint i = 0; i < indices.size(); i++) {
		if (v1 == vertices[indices[i]] || v2 == vertices[indices[i]]) {
			vertices[indices[i]] = v1;  //full edge collapse
			//vertices[indices[i]] = midVertex;  //half edge collapse
		}
	}
	return vertices;
}

//update indices buffer removing deprecated triangles, updating indices NOTE: if using half edge collapse pass midPoint as v1!!!
std::vector<unsigned short> updateIndices(std::vector<glm::vec3> vertices,
	std::vector<unsigned short> indices, glm::vec3 v1, glm::vec3 v2) {
	unsigned short new_index = -1;
	//printf("Updating Indices!\n");
	for (uint i = 0; i < indices.size(); ++i) {
		if (vertices[indices[i]] == v1) {
			new_index = indices[i];
		}
	}
	for (uint i = 0; i < indices.size(); ++i) {
		if (vertices[indices[i]] == v1 || vertices[indices[i]] == v2) {
			indices[i] = new_index;
		}
	}
	for (uint i = 0; i < indices.size(); i += 3) {
		if (indices[i] == indices[i + 1] || indices[i] == indices[i + 2] || indices[i + 1] == indices[i + 2]) {
			indices.erase(indices.begin() + i, indices.begin() + (i + 3));
			//printf("ERASING INDICES FROM BUFFER\N");
		}
	}
	return indices;
}

//finds middle point between two vertex
glm::vec3 midpoint(glm::vec3 a, glm::vec3 b) {
	glm::vec3 ret;
	ret.x = (a.x + b.x) / 2;
	ret.y = (a.y + b.y) / 2;
	ret.z = (a.z + b.z) / 2;
	return ret;
}


//Creates a priority list smallest edges -> biggest edges
std::vector<glm::vec3> createPriorityList(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices) {
	std::vector<glm::vec3> priorList;
	glm::vec3 a, b, c;
	float edge1 = 0, edge2 = 0, edge3 = 0;
	//float last_biggest = 0.0f;
	int smallest_i1 = -1, smallest_i2 = -1;
	float smallest = 0.0f;
	std::vector<glm::vec3> smallestVectors;
	int count = 0;
	do {
		float smallest = 999999.0f;
		for (uint i = 0; i < indices.size(); i += 3) {
			edge1 = glm::distance(indexed_vertices[indices[i]], indexed_vertices[indices[i + 1]]);
			edge2 = glm::distance(indexed_vertices[indices[i]], indexed_vertices[indices[i + 2]]);
			edge3 = glm::distance(indexed_vertices[indices[i + 1]], indexed_vertices[indices[i + 2]]);
			if (edge1 < smallest) {		//if smallest
				if ( !is_in_vector(priorList, indexed_vertices[indices[i]]) && !is_in_vector(priorList, indexed_vertices[indices[i + 1]]) ) { //if edge is already in priorlist
					smallest = edge1;
					smallest_i1 = indices[i];
					smallest_i2 = indices[i + 1];
				}
			}
			if (edge2 < smallest) {
				if (!is_in_vector(priorList, indexed_vertices[indices[i]]) && !is_in_vector(priorList, indexed_vertices[indices[i + 2]])) {
					smallest = edge1;
					smallest_i1 = indices[i];
					smallest_i2 = indices[i + 2];
				}
			}
			if (edge3 < smallest) {
				if (!is_in_vector(priorList, indexed_vertices[indices[i+1]]) && !is_in_vector(priorList, indexed_vertices[indices[i + 2]])) {
					smallest = edge1;
					smallest_i1 = indices[i+1];
					smallest_i2 = indices[i + 2];
				}
			}
		}
		priorList.insert(priorList.end(), indexed_vertices[smallest_i2]);
		priorList.insert(priorList.end(), indexed_vertices[smallest_i1]);

	} while (priorList.size() < indices.size() / 2);


	return priorList;
}


//Find all vertices around center within given range
std::vector<glm::vec3> findVerticesInRange(std::vector<glm::vec3> vertices, float range,
	glm::vec3 center, std::vector<unsigned short> indices) {
	std::vector<glm::vec3> verticesInBox;
	float xmax = center.x + range; float xmin = center.x - range;
	float ymax = center.y + range; float ymin = center.y - range;
	float zmax = center.z + range; float zmin = center.z - range;
	for (uint i = 0; i < indices.size(); ++i) {
		float x = vertices[indices[i]].x;
		float y = vertices[indices[i]].y;
		float z = vertices[indices[i]].z;
		if (
			x < xmax && x > xmin &&
			y < ymax && y > ymin &&
			z < zmax && z > zmin
			) {
			verticesInBox.push_back(vertices[indices[i]]);
		}
	}

	return verticesInBox;
}

//update normals of every single vertex
std::vector<glm::vec3> updateNormals(std::vector<glm::vec3> normals, std::vector<glm::vec3> vertices, std::vector<unsigned short> indices) {
	glm::vec3 a, b, c;
	glm::vec3 normal;
	glm::vec3 sum_vector;
	std::vector<glm::vec3> faces, blacklist, new_normals;
	uint count = 0, face_index = 0;
	for (uint i = 0; i < indices.size() - 3; i += 3) {  //calculates the normal of every single triangle
		a = vertices[indices[i]];
		b = vertices[indices[i + 1]];
		c = vertices[indices[i + 2]];
		normal = computeNormal(a, b, c);
		faces.insert(faces.end(), normal);
	}

	for (uint i = 0; i < indices.size() - 1; ++i) {
		sum_vector *= 0.0f;
		if (!is_in_vector(blacklist, vertices[indices[i]])) {
			for (uint j = 0; j < indices.size() - 1; ++j) {
				if (j == i) {
					continue;
				}
				else {
					if (vertices[indices[j]] == vertices[indices[i]]) {  //sum normals of every triangle near vertex
						face_index = myfloor((float)(j / 3));
						if (face_index < faces.size() - 1) {
							//printf("accessing face_index: %d\n", face_index);
							sum_vector += faces[face_index];
						}
					}
				}
			}
		}
		sum_vector = glm::normalize(sum_vector);
		new_normals.insert(new_normals.end(), sum_vector);
		blacklist.push_back(vertices[indices[i]]);

	}
	while (new_normals.size() < indices.size()) {
		new_normals.insert(new_normals.end(), sum_vector);
	}
	printf("updating Normals done!\n");
	return new_normals;
}

bool is_in_vector(std::vector<glm::vec3> vertices, glm::vec3 target) {
	for (uint i = 0; i < vertices.size(); ++i) {
		if (target == vertices[i])
			return true;
	}
	return false;
}
glm::vec3 computeNormal(
	glm::vec3 const & a,
	glm::vec3 const & b,
	glm::vec3 const & c)
{
	return glm::normalize(glm::cross(c - a, b - a));
}

int myfloor(float x) {
	int y = (int)x;
	if ((x - y) > 0.5f) {
		return y + 1;
	}
	else {
		return y;
	}
}


//Creates a priority list :: most number of neighbours -> least number of neighbours
std::vector<glm::vec3> createPriorityList_Clustering(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices) {
	std::vector<glm::vec3> priorList;
	int flag = 0;
	int cur_index = -1;
	int count = 0, index = -1;
	int max = 0;
	do {
		for (uint i = 0; i < indexed_vertices.size(); ++i) {
			flag = 0;
			for (uint j = 0; j < priorList.size(); ++j) {
				if (priorList[j] == indexed_vertices[i]) {  //check if vertex is already in priority queue
					flag = 1;
				}
			}
			if (flag == 0) {  //if not, counts its neibs
				index = getIndex(indexed_vertices[i], indexed_vertices, indices);
				count = std::count(indices.begin(), indices.end(), index);
				if (count > max) {
					max = count;
					cur_index = index;
				}
			}
		}
		priorList.insert(priorList.end(), indexed_vertices[cur_index]);
	} while (priorList.size() < indexed_vertices.size());

	return priorList;
}

//get index of a vertex in indices buffer
int getIndex(glm::vec3 v1, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices) {
	for (uint i = 0; i < indices.size(); ++i) {
		if (indexed_vertices[indices[i]] == v1)
			return indices[i];
	}
	return -1;
}



/*
	references:
	http://graphics.stanford.edu/courses/cs468-10-fall/LectureSlides/08_Simplification.pdf
	http://jerrytalton.net/research/t-ssmsa-04/paper.pdf

*/




std::vector<glm::vec3> createPriorityList_test(std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices) {
	std::vector<glm::vec3> priorList;
	std::vector<glm::vec3> plist;
	glm::vec3 a, b, c;
	float edge1 = 0, edge2 = 0, edge3 = 0;
	//float last_biggest = 0.0f;
	int smallest_i1 = -1, smallest_i2 = -1;
	//float smallest = 0.0f;
	std::vector<glm::vec3> smallestVectors;
	int count = 0;
	float dist1,dist2,dist3;
	float smallest = 999999.0f;

	for (uint i = 0; i < indices.size(); i += 3) {
		priorList.insert(priorList.end(), indexed_vertices[indices[i]]);
		priorList.insert(priorList.end(), indexed_vertices[indices[i+1]]);

		priorList.insert(priorList.end(), indexed_vertices[indices[i]]);
		priorList.insert(priorList.end(), indexed_vertices[indices[i + 2]]);

		priorList.insert(priorList.end(), indexed_vertices[indices[i + 1]]);
		priorList.insert(priorList.end(), indexed_vertices[indices[i + 2]]);
	}

	smallest = 999999.0f;

	int i = 0;
	do {
		smallest = 999999.0f;
		i = 0;
		do {
			if (i > priorList.size() - 3)
				break;
			dist1 = glm::distance(priorList[i], priorList[i + 1]);
			dist2 = glm::distance(priorList[i], priorList[i + 2]);
			dist3 = glm::distance(priorList[i + 1], priorList[i + 2]);
			if (dist1 < smallest) {
				smallest = dist1;
				smallest_i1 = i;
				smallest_i2 = i + 1;
			}
			if (dist2 < smallest) {
				smallest = dist2;
				smallest_i1 = i;
				smallest_i2 = i + 2;
			}
			if (dist3 < smallest) {
				smallest = dist3;
				smallest_i1 = i + 1;
				smallest_i2 = i + 2;
			}

			i += 3;
		} while (true);
		
		plist.insert(plist.end(), priorList[smallest_i1]);
		plist.insert(plist.end(), priorList[smallest_i2]);
		priorList.erase(priorList.begin() + smallest_i1, priorList.begin() + smallest_i2);
	} while (priorList.size() > 3);

	if (priorList.size() > 0) {
		while (priorList.size() > 0) {
			plist.insert(plist.end(), priorList[0]);
			priorList.erase(priorList.begin());
		}
	}

	return plist;
}
