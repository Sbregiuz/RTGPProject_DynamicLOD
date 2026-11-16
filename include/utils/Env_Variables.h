#pragma once 

//SCREEN
#include "glm/trigonometric.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <glm/glm.hpp>

const int Screen_Width = 1920;
const int Screen_Height = 1080;

//CAMERA
const float min_camera_distance = 3.0f;
const float max_camera_distance = 200.0f;

//GUI
const float TEXT_SIZE = 1.5f;

//LOD
//Linear Interpolation
const float min_range = 10.0f;
const float max_range = 180.0f;

//Screen Space Error 
const float error_scale = 0.1;

float pixel_tolerance = 1.5;
float last_pixel_tolerance = pixel_tolerance;
const float fov_y = 45.0f;
const float fov_y_radians = glm::radians(fov_y);
// const float screen_resolution = Screen_Height*Screen_Width;
float error_partial = (2*pixel_tolerance*std::tan(fov_y_radians/2))/Screen_Height;

int cut_function_selector;

void recompute_partial_error(){
    error_partial =  (2*pixel_tolerance*std::tan(fov_y_radians/2))/Screen_Height;
    last_pixel_tolerance = pixel_tolerance;
}

const std::vector<std::string> functions_names = {
    "Distance Based - Linear", 
    "Distance Based - NonLinear", 
    "ScreenSpace Coverage", 
    "ScreenSpace Error"
};

// int f1(float a){
//     return 0;
// }
// int f2(float a){
//     return 0;
// }
//
// const std::vector<std::function<int(float)>> cut_functions = {f1,f2};


