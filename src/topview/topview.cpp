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
#include "vec.hpp"
#include "mat.hpp"
#include "transform.hpp"
#include "Camera.hpp"
#include "cstring"

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

GLuint program; // 쉐이더 프로그램 객체의 레퍼런스 값
GLint loc_a_position;
GLint loc_a_color;
GLint loc_u_M;
GLint loc_u_V;
GLint loc_u_P;
float ang = 0.0f;
float eyex = 0.0f, eyey = 0.0f, eyez = 100.0f, centerx = 0.0f, centery = 0.0f,
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
	GLfloat position[12];
	GLfloat color[12];
	GLuint indices[3];
	for (int i = 0; i < 12; i += 4) {
		color[i] = p.color.r;
		color[i + 1] = p.color.g;
		color[i + 2] = p.color.b;
		color[i + 3] = p.color.a;
	}
	position[0] = p.position(0) - 1.f;
	position[1] = p.position(1);
	position[2] = 0.f;
	position[3] = 1.f;

	position[4] = p.position(0) + 1.f;
	position[5] = p.position(1);
	position[6] = 0.f;
	position[7] = 1.f;

	position[8] = p.position(0);
	position[9] = p.position(1) + 1.f;
	position[10] = 0.f;
	position[11] = 1.f;

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	glEnableVertexAttribArray(loc_a_color);
	glEnableVertexAttribArray(loc_a_position);
	glVertexAttribPointer(loc_a_position, 4, GL_FLOAT, GL_FALSE, 0, position);
	glVertexAttribPointer(loc_a_color, 4, GL_FLOAT, GL_FALSE, 0, color);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, indices);
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
	char name[15];
	kmuvcl::math::vec2i position;
};
car_Position car_positions[10];

void split_data(int index) {
	char *ptr;
	char temp[1024];
	memcpy(temp, *receive_data, sizeof(temp));
	ptr = strtok(temp, "#");
	memcpy(temp, strtok(NULL, "#"), sizeof(temp));
	ptr = strtok(temp, "&");
	for (int i = 0; i < index; i++)
		ptr = strtok(NULL, "&");
	ptr = strtok(ptr, ":");
	memcpy(car_positions[index].name, ptr, sizeof(ptr));
	ptr = strtok(NULL, ":");
	memcpy(temp, ptr, sizeof(temp));
	ptr = strtok(temp, ",");
	car_positions[index].position[0] = atoi(ptr);
	ptr = strtok(NULL, ",");
	car_positions[index].position[1] = atoi(ptr);
	//printf("name : %s (%d, %d)\n", car_positions[index].name, car_positions[index].position(0), car_positions[index].position(1));
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
	P = perspective(cam.getZoom(), 1.0f, 0.001f, 1000.0f);

	glUseProgram(program);

	glUniformMatrix4fv(loc_u_M, 1, false, M);
	glUniformMatrix4fv(loc_u_V, 1, false, V);
	glUniformMatrix4fv(loc_u_P, 1, false, P);
	for (int i = 0; i < count; i++) {
		particles[i].position = car_positions[i].position;
		if (!strcmp("Player", car_positions[i].name)) {
			particles[i].color.r = 0;
			particles[i].color.g = 0;
			particles[i].color.b = 1;
			particles[i].color.a = 1;
			eyex = (float) particles[i].position(0);
			eyey = (float) particles[i].position(1);
			eyez = 10.f;
			centerx = eyex;
			centery = eyey;
			centerz = 0;
			upx = 0.f;
			upy = 1.f;
			upz = 0.f;
			cam = kmuvcl::Camera(eyex, eyey, eyez, centerx, centery, centerz,
					upx, upy, upz);
		} else {
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

