#include "local.h"
#include <iostream>

int init_camera(void)
{
    app.camera.camera_pos = { 0.0f, 0.0f, 5.0f };
    app.camera.camera_front = { 0.0f, 0.0f, -1.0f };
    app.camera.camera_up = { 0.0f, 1.0f, 0.0f };
    app.camera.camera_right = { 1.0f, 0.0f, 0.0f };

    app.camera.jaw = -90.0f;
    app.camera.pitch = 0.0f;

    app.camera.mouse_sensitivity = 0.25f;

#ifdef __EMSCRIPTEN__
    app.camera.move_velocity = 0.01f;
#else
    app.camera.move_velocity = 0.01f;
#endif
    app.camera.ignore_next_mouse_event = false;
    app.camera.mouse_moving = false;

    return 0;
}

glm::mat4 get_view_matrix() {

    return glm::lookAt(
        app.camera.camera_pos, 
        app.camera.camera_pos + app.camera.camera_front, 
        app.camera.camera_up);
}

void update_camera_vectors() {
    // Calculate front vector
    glm::vec3 front;
    front.x = cos(glm::radians(app.camera.jaw)) * cos(glm::radians(app.camera.pitch));
    front.y = sin(glm::radians(app.camera.pitch));
    front.z = sin(glm::radians(app.camera.jaw)) * cos(glm::radians(app.camera.pitch));

    app.camera.camera_front = glm::normalize(front);
    app.camera.camera_right = glm::normalize(glm::cross(app.camera.camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    app.camera.camera_up = glm::normalize(glm::cross(app.camera.camera_right, app.camera.camera_front));
}

static void process_mouse_movement(double xoffset, double yoffset) {
    bool constrain_pitch = true;

    xoffset *= app.camera.mouse_sensitivity;
    yoffset *= app.camera.mouse_sensitivity;

    app.camera.jaw += xoffset;
    app.camera.pitch += yoffset;

    if (constrain_pitch) {
        if (app.camera.pitch > 45.0f) app.camera.pitch = 45.0f;
        if (app.camera.pitch < -45.0f) app.camera.pitch = -45.0f;
    }

    update_camera_vectors();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (app.camera.ignore_next_mouse_event) {
        app.camera.ignore_next_mouse_event = false;
        app.camera.last_x = xpos;
        app.camera.last_y = ypos;
        return;
    }

    double xoffset = xpos - app.camera.last_x;
    double yoffset = app.camera.last_y - ypos; 
    app.camera.last_x = xpos;
    app.camera.last_y = ypos;

    if (std::abs(xoffset) > 0.01 || std::abs(yoffset) > 0.01) {
        app.camera.mouse_moving = true;
        process_mouse_movement(xoffset, yoffset);
    }
}

void process_keyboard( GLFWwindow* window ) {
    float velocity;

    // Speed up when control key is pressed
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        velocity = app.camera.move_velocity * 30.0f;  // Increase movement speed
    else
        velocity = app.camera.move_velocity;  // Normal movement speed

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        app.camera.camera_pos += app.camera.camera_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        app.camera.camera_pos -= app.camera.camera_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        app.camera.camera_pos -= app.camera.camera_right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        app.camera.camera_pos += app.camera.camera_right * velocity;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        init_camera();
}