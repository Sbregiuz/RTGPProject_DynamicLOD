#pragma once 

//SCREEN
#include <functional>
#include <string>
#include <vector>
const int Screen_Width = 1920;
const int Screen_Height = 1080;

//CAMERA
const float min_camera_distance = 3.0f;
const float max_camera_distance = 200.0f;

//GUI
const float TEXT_SIZE = 1.5f;

//LOD
const float min_range = 10.0f;
const float max_range = 180.0f;

int cut_function_selector;

const std::vector<std::string> functions_names = {"Distance Based - Linear", "Distance Based - NonLinear", "ScreenSpace Coverage", "ScreenSpace Error"};
// int f1(float a){
//     return 0;
// }
// int f2(float a){
//     return 0;
// }
//
// const std::vector<std::function<int(float)>> cut_functions = {f1,f2};


