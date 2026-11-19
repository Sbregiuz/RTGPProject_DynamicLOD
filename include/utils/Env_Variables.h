#pragma once 

#include "glm/trigonometric.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <glm/glm.hpp>

//SCREEN
const int Screen_Width = 1920;
const int Screen_Height = 1080;

//CAMERA
const float min_camera_distance = 3.0f;
const float max_camera_distance = 200.0f;

const float fov_y = 45.0f;
const float fov_y_radians = glm::radians(fov_y);

//GUI
const float TEXT_SIZE = 1.5f;

//LOD
int cut_function_selector;
const std::vector<std::string> functions_names = {
    "Distance Based - Linear", 
    "Distance Based - NonLinear", 
    "Screen Coverage", 
    "ScreenSpace Error"
};

//Linear Interpolation
const float min_range = 10.0f;
const float max_range = 180.0f;

// Non-Linear Interpolation 
float exponent = 0.4f;

const float min_exponent = 0.1f;
const float max_exponent = 1.0f;
float last_exponent = exponent;

//Screen Coverage
int const range_pixels_min = 10;
int const range_pixels_max = 1000;

int min_pixels = 100;
int max_pixels = 900;

int last_min_pixels = min_pixels;
int last_max_pixels = max_pixels;

float ssc_error_partial = Screen_Height/tan(fov_y_radians/2);

//Screen Space Error 
const float error_scale = 0.1;
float pixel_tolerance = 1.5;
float last_pixel_tolerance = pixel_tolerance;
float sse_error_partial = (2*pixel_tolerance*std::tan(fov_y_radians/2))/Screen_Height;

void recompute_partial_error(){
    sse_error_partial =  (2*pixel_tolerance*std::tan(fov_y_radians/2))/Screen_Height;
    last_pixel_tolerance = pixel_tolerance;
}


