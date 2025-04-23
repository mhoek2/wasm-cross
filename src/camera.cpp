#include "local.h"
#include <iostream>

int init_camera(void)
{
    camera.camera_pos = { 6.0f, 1.0f, 15.0f };
    camera.camera_front = { 0.0f, 0.0f, -1.0f };
    camera.camera_up = { 0.0f, 1.0f, 0.0f };
    camera.camera_right = { 1.0f, 0.0f, 0.0f };

    camera.jaw = -90.0f;
    camera.pitch = 0.0f;

    camera.mouse_sensitivity = 0.25f;
    camera.move_velocity = 0.0001f;

    camera.ignore_next_mouse_event = false;
    camera.mouse_moving = false;

    return 0;
}

glm::mat4 get_view_matrix() {

    return glm::lookAt(
        camera.camera_pos, camera.
        camera_pos + camera.camera_front, 
        camera.camera_up);
}

void update_camera_vectors() {
    // Calculate front vector
    glm::vec3 front;
    front.x = cos(glm::radians(camera.jaw)) * cos(glm::radians(camera.pitch));
    front.y = sin(glm::radians(camera.pitch));
    front.z = sin(glm::radians(camera.jaw)) * cos(glm::radians(camera.pitch));

    camera.camera_front = glm::normalize(front);
    camera.camera_right = glm::normalize(glm::cross(camera.camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    camera.camera_up = glm::normalize(glm::cross(camera.camera_right, camera.camera_front));
}

static void process_mouse_movement(double xoffset, double yoffset) {
    // Apply sensitivity
    bool constrain_pitch = true;

    xoffset *= camera.mouse_sensitivity;
    yoffset *= camera.mouse_sensitivity;

    // Update yaw and pitch
    camera.jaw += xoffset;
    camera.pitch += yoffset;

    // Constrain pitch
    if (constrain_pitch) {
        if (camera.pitch > 45.0f) camera.pitch = 45.0f;
        if (camera.pitch < -45.0f) camera.pitch = -45.0f;
    }

    update_camera_vectors();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (camera.ignore_next_mouse_event) {
        camera.ignore_next_mouse_event = false;
        camera.last_x = xpos;
        camera.last_y = ypos;
        return;
    }

    double xoffset = xpos - camera.last_x;
    double yoffset = camera.last_y - ypos;  // Invert the y-axis because GLFW has the origin at the bottom-left
    camera.last_x = xpos;
    camera.last_y = ypos;

    if (std::abs(xoffset) > 0.01 || std::abs(yoffset) > 0.01) {
        camera.mouse_moving = true;
        process_mouse_movement(xoffset, yoffset);
    }

   // std::cout << "offsetx: " << xoffset << " offsety: " << yoffset << "\n";
}

void process_keyboard( GLFWwindow* window ) {
    float velocity;

    // Speed up when control key is pressed
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        velocity = camera.move_velocity * 30.0f;  // Increase movement speed
    else
        velocity = camera.move_velocity;  // Normal movement speed

    // Handle keyboard input for movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.camera_pos += camera.camera_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.camera_pos -= camera.camera_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.camera_pos -= camera.camera_right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.camera_pos += camera.camera_right * velocity;

}