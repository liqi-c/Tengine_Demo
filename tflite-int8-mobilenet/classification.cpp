/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * License); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*
 * Copyright (c) 2018, Open AI Lab
 * Author: haoluo@openailab.com
 */
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <iomanip>
#include "tengine_operations.h"

#include "tengine_c_api.h"

#define DEFAULT_MODEL_NAME "mobilenet_v2_quant_tflite.tmfile"
#define DEFAULT_IMAGE_FILE "images/cat.jpg"
#define DEFAULT_LABEL_FILE "models/synset_words.txt"
#define DEFAULT_IMG_H 224
#define DEFAULT_IMG_W 224
#define DEFAULT_REPEAT_CNT 1

template <typename T> static std::vector<T> ParseString(const std::string str)
{
    typedef std::string::size_type pos;
    const char delim_ch = ',';
    std::string str_tmp = str;
    std::vector<T> result;
    T t;

    pos delim_pos = str_tmp.find(delim_ch);
    while(delim_pos != std::string::npos)
    {
        std::istringstream ist(str_tmp.substr(0, delim_pos));
        ist >> t;
        result.push_back(t);
        str_tmp.replace(0, delim_pos + 1, "");
        delim_pos = str_tmp.find(delim_ch);
    }
    if(str_tmp.size() > 0)
    {
        std::istringstream ist(str_tmp);
        ist >> t;
        result.push_back(t);
    }

    return result;
}
bool check_file_exist(const std::string file_name)
{
    FILE* fp = fopen(file_name.c_str(), "r");
    if(!fp)
    {
        std::cerr << "Input file not existed: " << file_name << "\n";
        return false;
    }
    fclose(fp);
    return true;
}

void get_input_data_uint8(const char* image_file, uint8_t* input_data, int img_h, int img_w)
{
    image im = imread(image_file);

    image resImg = resize_image(im, img_w, img_h);

    int index = 0;
    for(int h = 0; h < img_h; h++)
        for(int w = 0; w < img_w; w++)
            for(int c = 0; c < 3; c++)
                input_data[index++] = ( uint8_t )resImg.data[c * img_h * img_w + h * img_w + w];
    free_image(im);
    free_image(resImg);    

}

void LoadLabelFile(std::vector<std::string>& result, const char* fname)
{
    std::ifstream labels(fname);

    std::string line;
    while(std::getline(labels, line))
        result.push_back(line);
}

static inline bool PairCompare(const std::pair<float, int>& lhs, const std::pair<float, int>& rhs)
{
    return lhs.first > rhs.first;
}

static inline std::vector<int> Argmax(const std::vector<float>& v, int N)
{
    std::vector<std::pair<float, int>> pairs;
    for(size_t i = 0; i < v.size(); ++i)
        pairs.push_back(std::make_pair(v[i], i));
    std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

    std::vector<int> result;
    for(int i = 0; i < N; ++i)
        result.push_back(pairs[i].second);
    return result;
}
void PrintTopLabels_uint8(const char* label_file, uint8_t* data, int data_size, float scale, int zero_point)
{
    std::vector<std::string> labels;
    LoadLabelFile(labels, label_file);

    std::vector<float> result;
    for(int i = 0; i < data_size; i++)
        result.push_back((data[i] - zero_point) * scale);

    std::vector<int> top_N = Argmax(result, 5);

    for(unsigned int i = 0; i < top_N.size(); i++)
    {
        int idx = top_N[i];
        std::cout << std::fixed << std::setprecision(4) << result[idx] << " - \"" << labels[idx] << "\"\n";
    }
}
bool run_tengine_library(const char* model_file, const char* label_file, const char* image_file,
                         int img_h, int img_w, int warm_count, int repeat_count)
{
    // init
    init_tengine();
    std::cout << "tengine library version: " << get_tengine_version() << "\n";
    if(request_tengine_version("1.2") < 0)
        return false;

    // create graph
    graph_t graph = create_graph(nullptr, "tengine", model_file);
    if(graph == nullptr)
    {
        std::cerr << "Create graph failed.\n";
        std::cerr << "errno: " << get_tengine_errno() << "\n";
        return false;
    }

    // set input shape
    int img_size = img_h * img_w * 3;
    int dims[4] = {1, img_h, img_w, 3};
    uint8_t* input_data_tflite = ( uint8_t* )malloc(img_size);

    tensor_t input_tensor = get_graph_input_tensor(graph, 0, 0);
    if(input_tensor == nullptr)
    {
        std::cerr << "Get input tensor failed\n";
        return false;
    }
    set_tensor_shape(input_tensor, dims, 4);

    // prerun
    if(prerun_graph(graph) < 0)
    {
        std::cerr << "Prerun graph failed\n";
        return false;
    }
    //dump_graph(graph);
    
    struct timeval t0, t1;
    float avg_time = 0.f;
    float min_time = __DBL_MAX__;
    float max_time = -__DBL_MAX__;
    get_input_data_uint8(image_file, input_data_tflite, img_h, img_w);
        
    set_tensor_buffer(input_tensor, input_data_tflite, img_size * 4);
    
    if(run_graph(graph, 1) < 0)
    {
        std::cerr << "Run graph failed\n";
        return false;
    }
    //warm up
    for(int i = 0; i < warm_count; i++)
    {
        run_graph(graph, 1);
    }

    for(int i = 0; i < repeat_count; i++)
    {
        gettimeofday(&t0, NULL);
        run_graph(graph, 1);
        gettimeofday(&t1, NULL);

        float mytime = ( float )((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
        avg_time += mytime;
        min_time = std::min(min_time, mytime);
        max_time = std::max(max_time, mytime);
    }
    std::cout << "\nModel name : " << model_file << "\n"
              << "label file : " << label_file << "\n"
              << "image file : " << image_file << "\n"
              << "img_h, imag_w, scale, mean[3] : " << img_h << " " << img_w << "\n";
    std::cout << "\nRepeat " << repeat_count << " times, avg time per run is " << avg_time / repeat_count << " ms\n" << "max time is " << max_time << " ms, min time is " << min_time << " ms\n";
    std::cout << "--------------------------------------\n";

    // print output
    tensor_t output_tensor = get_graph_output_tensor(graph, 0 , 0);
    uint8_t* data = ( uint8_t* )get_tensor_buffer(output_tensor);
    int data_size = get_tensor_buffer_size(output_tensor);
    float output_scale = 0.0f;
    int zero_point = 0;
    get_tensor_quant_param(output_tensor, &output_scale, &zero_point, 1);
    PrintTopLabels_uint8(label_file, data, data_size, output_scale, zero_point);
    std::cout << "--------------------------------------\n";

    free(input_data_tflite);
    release_graph_tensor(input_tensor);
    release_graph_tensor(output_tensor);
    postrun_graph(graph);
    destroy_graph(graph);

    release_tengine();

    return true;
}

int main(int argc, char* argv[])
{
    int repeat_count = DEFAULT_REPEAT_CNT;
    std::string model_file = DEFAULT_MODEL_NAME;
    std::string label_file = DEFAULT_LABEL_FILE;
    std::string image_file = DEFAULT_IMAGE_FILE;
    std::vector<int> hw;
    int img_h = 0;
    int img_w = 0;
    int warm_count = 1;

    int res;
    while((res = getopt(argc, argv, "m:t:l:i:g:s:w:r:h")) != -1)
    {
        switch(res)
        {
            case 'm':
                model_file = optarg;
                break;
            case 'l':
                label_file = optarg;
                break;
            case 'i':
                image_file = optarg;
                break;
            case 'g':
                hw = ParseString<int>(optarg);
                if(hw.size() != 2)
                {
                    std::cerr << "Error -g parameter.\n";
                    return -1;
                }
                img_h = hw[0];
                img_w = hw[1];
                break;
            case 'w':
                warm_count = std::strtoul(optarg, NULL, 10);
                break;
            case 'r':
                repeat_count = std::strtoul(optarg, NULL, 10);
                break;
            case 'h':
                std::cout << "[Usage]: " << argv[0] << " [-h]\n"
                          << "    [-m model_file] [-l label_file] [-i image_file]\n"
                          << "    [-g img_h,img_w] [-w warm_count] [-r repeat_count]\n";
                return 0;
            default:
                break;
        }
    }

    if(img_h == 0)
        img_h = DEFAULT_IMG_H;
    if(img_w == 0)
        img_w = DEFAULT_IMG_W;

    // check input files
    if(!check_file_exist(model_file) || !check_file_exist(label_file) || !check_file_exist(image_file))
        return -1;

    // start to run
    if(!run_tengine_library(model_file.c_str(), label_file.c_str(), image_file.c_str(), img_h, img_w,
                            warm_count, repeat_count))
        return -1;

    std::cout << "ALL TEST DONE\n";

    return 0;
}
