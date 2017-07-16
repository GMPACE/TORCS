/*
 * topview.cpp
 *
 *  Created on: 2017. 7. 8.
 *      Author: kang
 */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <cmath>
#include "vec.hpp"
#include "mat.hpp"
#include "transform.hpp"
#include "Camera.hpp"
#include <cstring>
#include "SOIL.h"

using namespace std;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};

void init();
void mydisplay();
void myIdle();
GLuint create_shader_from_file(const std::string& filename, GLuint shader_type);
void set_texture(const char* filename, GLuint* texid_);
GLuint program; // 쉐이더 프로그램 객체의 레퍼런스 값
GLint loc_a_position;
GLint loc_a_color;
GLint loc_a_texcoord;
GLint loc_u_texid;
GLint loc_u_tex_flag;
GLint loc_u_M;
GLint loc_u_V;
GLint loc_u_P;
GLuint texid;
float ang = 0.0f;
float eyex = 0.0f, eyey = 0.0f, eyez = 1000.0f, centerx = 0.0f, centery = 0.0f,
		centerz = 0.0f, upx = 0.0f, upy = 1.0f, upz = 0.0f;
kmuvcl::Camera cam(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);

struct Color {
	float r;
	float g;
	float b;
	float a;
};
struct Particle {
	kmuvcl::math::vec2i position;
	Color color;
	int use_tex;
};
/* shared memory */
int shmid;
int semid;
union semun sem_union;
char* shared_memory[1024];

char buff[1024];
int skey = 1234;

char* receive_data[1024];

void init_shared_memory() {
	shmid = shmget((key_t) skey, sizeof(int), 0777 | IPC_CREAT);
	if (shmid == -1) {
		perror("shmget failed");
		exit(0);
	}
	shared_memory[0] = (char*) shmat(shmid, (void *) 0, 0);
	if (!shared_memory) {
		perror("shmat failed");
		exit(0);
	}
	receive_data[0] = shared_memory[0];
}
void delete_shared_memory() {
	if (shmdt(shared_memory) < 0) {
		perror("shmdt failed");
		exit(0);
	}
}
/* shared memory */
void draw_Particle(Particle& p) {
	GLfloat position[16];
	GLfloat color[16];
	GLuint indices[6];
//	GLfloat texcoords[] = {
//	      1,1,  1,0, 0,0,  0,1, //right
//	};
//	GLfloat texcoords[] = {
//			0,1,  0,0, 1,0,  1,1, // left
//	 };




	for (int i = 0; i < 16; i += 4) {
		color[i] = p.color.r;
		color[i + 1] = p.color.g;
		color[i + 2] = p.color.b;
		color[i + 3] = p.color.a;
	}
	position[0] = p.position(0) - 100;
	position[1] = p.position(1) - 100;
	position[2] = 0.f;
	position[3] = 1.f;

	position[4] = p.position(0) - 100;
	position[5] = p.position(1) + 100;
	position[6] = 0.f;
	position[7] = 1.f;

	position[8] = p.position(0) + 100;
	position[9] = p.position(1) + 100;
	position[10] = 0.f;
	position[11] = 1.f;

	position[12] = p.position(0) + 100;
	position[13] = p.position(1) - 100;
	position[14] = 0.f;
	position[15] = 1.f;

	indices[0] = 0;
	indices[1] = 3;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 1;
	indices[5] = 0;
	GLfloat texcoords[8];
	if (p.use_tex > 0) {
		glUniform1i(loc_u_tex_flag, 1);
		for(int i = 2; i <= 14; i += 4) {
			position[i] = 1.f;
		}
		if(p.use_tex == 1) {
			texcoords[0] = 0;
			texcoords[1] = 1;
			texcoords[2] = 0;
			texcoords[3] = 0;
			texcoords[4] = 1;
			texcoords[5] = 0;
			texcoords[6] = 1;
			texcoords[7] = 1;
		} else {
			texcoords[0] = 1;
			texcoords[1] = 1;
			texcoords[2] = 1;
			texcoords[3] = 0;
			texcoords[4] = 0;
			texcoords[5] = 0;
			texcoords[6] = 0;
			texcoords[7] = 1;
		}
	}
	else {
		glUniform1i(loc_u_tex_flag, 0);
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texid);
	glUniform1i(loc_u_texid, 1);
	glEnableVertexAttribArray(loc_a_color);
	glEnableVertexAttribArray(loc_a_position);
	glEnableVertexAttribArray(loc_a_texcoord);
	glVertexAttribPointer(loc_a_position, 4, GL_FLOAT, GL_FALSE, 0, position);
	glVertexAttribPointer(loc_a_color, 4, GL_FLOAT, GL_FALSE, 0, color);
	glVertexAttribPointer(loc_a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
	glDisableVertexAttribArray(loc_a_texcoord);
	glDisableVertexAttribArray(loc_a_color);
	glDisableVertexAttribArray(loc_a_position);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitWindowSize(500, 500);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Top View");

	init_shared_memory();

	init();

	glutDisplayFunc(mydisplay);
	glutIdleFunc(myIdle);
	glutMainLoop();

	delete_shared_memory();
	return 0;
}
char splited_data[30][10];

struct car_Position {
	char name[25];
	kmuvcl::math::vec2d position;
	double dist_to_left;
	double dist_raced;
	int driver_intent;
};
car_Position car_positions[10];

void split_data(int index) {
	char *ptr;
	char temp[2048];
	memcpy(temp, *receive_data, sizeof(temp));
	ptr = strtok(temp, "#");
	memcpy(temp, strtok(NULL, "#"), sizeof(temp));
	ptr = strtok(temp, "&");
	for (int i = 0; i < index; i++)
		ptr = strtok(NULL, "&");
	ptr = strtok(ptr, ":");
	memcpy(car_positions[index].name, ptr, sizeof(ptr)+ 10);
	ptr = strtok(NULL, ":");
	memcpy(temp, ptr, sizeof(temp));
	ptr = strtok(temp, ",");
	car_positions[index].position[0] = atoi(ptr);
	ptr = strtok(NULL, ",");
	car_positions[index].position[1] = atoi(ptr);
	memcpy(temp, ptr, sizeof(temp));
	ptr = strtok(temp, "%");
	ptr = strtok(NULL, "%");
	car_positions[index].dist_to_left = atoi(ptr) * 100;
	memcpy(temp, ptr, sizeof(temp));
	ptr = strtok(temp, "@");
	ptr = strtok(NULL, "@");
	car_positions[index].dist_raced = atoi(ptr);
	memcpy(temp, ptr, sizeof(temp));
	ptr = strtok(temp, "$");
	ptr = strtok(NULL, "$");
	car_positions[index].driver_intent = atoi(ptr);
	printf("name : %s (%d, %d) dist to left : %f dist raced : %f driver_intent : %d\n",
			car_positions[index].name, car_positions[index].position(0),
			car_positions[index].position(1), car_positions[index].dist_to_left,
			car_positions[index].dist_raced, car_positions[index].driver_intent);
}

void myIdle() {
	glutPostRedisplay();
}

// GLSL 파일을 읽어서 컴파일한 후 쉐이더 객체를 생성하는 함수
GLuint create_shader_from_file(const std::string& filename,
		GLuint shader_type) {
	GLuint shader = 0;
	shader = glCreateShader(shader_type);
	std::ifstream shader_file(filename.c_str());
	std::string shader_string;
	shader_string.assign((std::istreambuf_iterator<char>(shader_file)),
			std::istreambuf_iterator<char>());

	const GLchar* shader_src = shader_string.c_str();

	glShaderSource(shader, 1, (const GLchar**) &shader_src, NULL);
	glCompileShader(shader);

	return shader;
}

void init() {
	glewInit();
	glShadeModel( GL_SMOOTH);
	// 정점 쉐이더 객체를 파일로부터 생성

	GLuint vertex_shader = create_shader_from_file("./shader/vertex.glsl",
	GL_VERTEX_SHADER);

	// 프래그먼트 쉐이더 객체를 파일로부터 생성
	GLuint fragment_shader = create_shader_from_file("./shader/fragment.glsl",
	GL_FRAGMENT_SHADER);

	// 쉐이더 프로그램 생성 및 컴파일
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	loc_u_M = glGetUniformLocation(program, "u_M");
	loc_u_V = glGetUniformLocation(program, "u_V");
	loc_u_P = glGetUniformLocation(program, "u_P");

	loc_a_position = glGetAttribLocation(program, "a_position");
	loc_a_color = glGetAttribLocation(program, "a_color");

	loc_u_texid        = glGetUniformLocation(program, "u_texid");
	loc_u_tex_flag     = glGetUniformLocation(program, "u_tex_flag");
	loc_a_texcoord     = glGetAttribLocation(program, "a_texcoord");

	set_texture("turn_signal.jpg", &texid);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void mydisplay() {
	char* ptr;
	char temp[1024];
	memcpy(temp, *receive_data, sizeof(temp));
	ptr = strtok(temp, "#");
	int count = atoi(ptr);
	for (int i = 0; i < count; i++)
		split_data(i);
	Particle* particles = new Particle[count];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	using namespace kmuvcl::math;
	mat4x4f M, V, P;
	mat4x4f T = translate(0.0f, 0.0f, 0.0f);
	mat4x4f R = rotate(ang, 0.0f, 1.0f, 0.0f);
	M = T * R;
	V = cam.lookAt();
	P = perspective(cam.getZoom(), 1.0f, 0.001f, 10000.0f);

	glUseProgram(program);

	glUniformMatrix4fv(loc_u_M, 1, false, M);
	glUniformMatrix4fv(loc_u_V, 1, false, V);
	glUniformMatrix4fv(loc_u_P, 1, false, P);
	kmuvcl::math::vec2d my_position;
	double my_dist_to_left;
	double my_dist_raced;
	double theta;
	for (int i = 0; i < count; i++) {
		if (!strcmp("Player", car_positions[i].name)) {
			my_position = car_positions[i].position;
			my_dist_to_left = car_positions[i].dist_to_left;
			my_dist_raced = car_positions[i].dist_raced;
			particles[i].position(0) = 0.f;
			particles[i].position(1) = 0.f;
			particles[i].position(2) = 0.f;
			particles[i].position(3) = 0.f;
			particles[i].color.r = 0;
			particles[i].color.g = 0;
			particles[i].color.b = 1;
			particles[i].color.a = 1;
			particles[i].use_tex = 0;
		}else {
			double d_cos_theta; // d * cos(theta)
			kmuvcl::math::vec2d dist_vec = my_position-car_positions[i].position;
			double dist = sqrt(pow(my_position(0) - car_positions[i].position(0), 2) + pow(my_position(1) - car_positions[i].position(1), 2));
			dist *= 100;
			d_cos_theta = my_dist_to_left - car_positions[i].dist_to_left;
			theta = acos(fabs(d_cos_theta) / dist);

			if(d_cos_theta > 0)  // left
				particles[i].position(0) = dist*cos(theta);
			else  				// right
				particles[i].position(0) = -dist*cos(theta);

			if(my_dist_raced > car_positions[i].dist_raced) // bottom
				particles[i].position(1) = -dist*sin(theta);
			else
				particles[i].position(1) = dist*sin(theta);

			particles[i].position(2) = 0.f;
			particles[i].position(3) = 0.f;

			particles[i].use_tex = car_positions[i].driver_intent;

			particles[i].color.r = 1;
			particles[i].color.g = 0;
			particles[i].color.b = 0;
			particles[i].color.a = 1;
		}
		draw_Particle(particles[i]);
	}

	glUseProgram(0);

	glutSwapBuffers();
}
void set_texture(const char* filename, GLuint* texid_) {
  int width, height, channels;
  unsigned char* image = SOIL_load_image(filename,
    &width, &height, &channels, SOIL_LOAD_RGB);

  glGenTextures(1, texid_);
  glBindTexture(GL_TEXTURE_2D, *texid_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

  SOIL_free_image_data(image);
}
