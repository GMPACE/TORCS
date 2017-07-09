#include "vec.hpp"
#include "mat.hpp"
#include "operator.hpp"
#include <GL/freeglut.h>
namespace kmuvcl {
class Camera {
private:
	kmuvcl::math::vec3f x_axis; // right
	kmuvcl::math::vec3f y_axis; // up
	kmuvcl::math::vec3f z_axis; // view direction
	kmuvcl::math::vec3f origin; //중심의 위치 eye's position
	kmuvcl::math::vec2f oldMousePosition;
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	float zoom = 100.0f;
	kmuvcl::math::mat4x4f V;
	kmuvcl::math::vec3f init_x_axis;
	kmuvcl::math::vec3f init_y_axis;
	kmuvcl::math::vec3f init_z_axis;
	kmuvcl::math::vec3f init_origin;
	float init_zoom;
public:
	Camera() {
		x_axis = kmuvcl::math::vec3f(1, 0, 0);
		y_axis = kmuvcl::math::vec3f(0, 1, 0);
		z_axis = kmuvcl::math::vec3f(0, 0, -1);
		origin = kmuvcl::math::vec3f(0, 0, 0);
	}
	Camera(float eyex, float eyey, float eyez, float viewx, float viewy,
			float viewz, float upx, float upy, float upz) {
		// set Position
		origin = kmuvcl::math::vec3f(eyex, eyey, eyez);
		// set z_axis
		z_axis = kmuvcl::math::vec3f(viewx, viewy, viewz);
		z_axis -= origin;
		z_axis = z_axis.normalize();
		// set x_axis
		kmuvcl::math::vec3f up(upx, upy, upz);
		x_axis = cross(up, z_axis);
		x_axis = x_axis.normalize();
		// set y_axis
		y_axis = cross(z_axis, x_axis);
		y_axis = y_axis.normalize();
		init_x_axis = x_axis;
		init_y_axis = y_axis;
		init_z_axis = z_axis;
		init_origin = origin;
		init_zoom = zoom;
	}
	kmuvcl::math::mat4x4f lookAt() {
		kmuvcl::math::mat4x4f m;
		m(0, 0) = 1.f;
		m(1, 0) = 0.f;
		m(2, 0) = 0.f;
		m(3, 0) = 0.f;

		m(0, 1) = 0.f;
		m(1, 1) = 1.f;
		m(2, 1) = 0.f;
		m(3, 1) = 0.f;

		m(0, 2) = 0.f;
		m(1, 2) = 0.f;
		m(2, 2) = 1.f;
		m(3, 2) = 0.f;

		m(0, 3) = -origin(0);
		m(1, 3) = -origin(1);
		m(2, 3) = -origin(2);
		m(3, 3) = 1.f;

		V(0, 0) = x_axis(0);
		V(1, 0) = y_axis(0);
		V(2, 0) = -z_axis(0);
		V(3, 0) = 0.f;
		V(0, 1) = x_axis(1);
		V(1, 1) = y_axis(1);
		V(2, 1) = -z_axis(1);
		V(3, 1) = 0.f;
		V(0, 2) = x_axis(2);
		V(1, 2) = y_axis(2);
		V(2, 2) = -z_axis(2);
		V(3, 2) = 0.f;
		V(0, 3) = 0.f;
		V(1, 3) = 0.f;
		V(2, 3) = 0.f;
		V(3, 3) = 1.f;
		return V * m;
	}
	kmuvcl::math::vec3f position() {
		return origin;
	}
	kmuvcl::math::vec3f viewDirection() {
		return z_axis;
	}
	kmuvcl::math::vec3f right() {
		return x_axis;
	}
	kmuvcl::math::vec3f up() {
		return y_axis;
	}
	float getZoom() {
		return zoom;
	}
	void keyboard(unsigned char key, int x, int y);
	void mouseUpdate(const kmuvcl::math::vec2f& mousePosition);
};

void Camera::keyboard(unsigned char key, int x, int y) {
	const float MOVE_CONSTANT = 1.0f;
	kmuvcl::math::vec3f temp;
	switch (key) {
	case GLUT_KEY_LEFT:
		temp = -MOVE_CONSTANT * x_axis;
		break;
	case GLUT_KEY_RIGHT:
		temp = MOVE_CONSTANT * x_axis;
		break;
	case GLUT_KEY_UP:
		temp = MOVE_CONSTANT * y_axis;
		break;
	case GLUT_KEY_DOWN:
		temp = -MOVE_CONSTANT * y_axis;
		break;
	case '+':
		zoom -= 1.f;
		if(zoom < 0.f) zoom = 0.f;
		break;
	case '-':
		zoom += 1.f;
		if(zoom > 180.f) zoom = 180.f;
		break;
	case GLUT_KEY_HOME :
		x_axis = init_x_axis;
		y_axis = init_y_axis;
		z_axis = init_z_axis;
		origin = init_origin;
		zoom = init_zoom;
		break;
	default:
		break;
	}
	origin += temp;

}
void Camera::mouseUpdate(const kmuvcl::math::vec2f& mousePosition) {
//	kmuvcl::math::vec2f mouseDelta;
//	mouseDelta(0) = mousePosition(0) - oldMousePosition(0);
//	mouseDelta(1) = mousePosition(1) - oldMousePosition(1);
//	printf("Delta : %f\n", mouseDelta(0));
//	float angle_yaw = -mouseDelta(0) * 0.01f;
	float angle_yaw;
	float angle_pitch = 0.f;
	if(mousePosition(0) > 0) angle_yaw = -0.5f;
	else angle_yaw = 0.5f;
	kmuvcl::math::mat4x4f rotate_mat_yaw;
	rotate_mat_yaw = kmuvcl::math::rotate(angle_yaw, y_axis(0), y_axis(1),
			y_axis(2));
	kmuvcl::math::mat4x4f trans_T = kmuvcl::math::translate(0.f, 0.f, 0.f);
	kmuvcl::math::mat4x4f trans_T_i = kmuvcl::math::translate(origin(0), origin(1), origin(2));
	kmuvcl::math::mat4x4f rotate_mat_pitch = kmuvcl::math::rotate(angle_pitch,
			x_axis(0), x_axis(1), x_axis(2));
	kmuvcl::math::vec4f temp_z(z_axis(0), z_axis(1), z_axis(2), 0);
	kmuvcl::math::vec4f temp_y(y_axis(0), y_axis(1), y_axis(2), 0);
	temp_z = trans_T * temp_z;
	temp_z = rotate_mat_pitch * temp_z;
	temp_z = trans_T_i * temp_z;
	z_axis(0) = temp_z(0);
	z_axis(1) = temp_z(1);
	z_axis(2) = temp_z(2);
	//temp_y = rotate_mat_pitch * temp_y;
	z_axis = z_axis.normalize();
//	y_axis(0) = temp_y(0);
//	y_axis(1) = temp_y(1);
//	y_axis(2) = temp_y(2);
//	printf("p(%f, %f, %f)\n", origin(0), origin(1), origin(2));
//	printf("z(%f, %f, %f)\n", z_axis(0), z_axis(1), z_axis(2));
//	printf("y(%f, %f, %f)\n", y_axis(0), y_axis(1), y_axis(2));
//	printf("x(%f, %f, %f)\n", x_axis(0), x_axis(1), x_axis(2));
	if (mousePosition(0) != 0)
		oldMousePosition = mousePosition;
}
}
