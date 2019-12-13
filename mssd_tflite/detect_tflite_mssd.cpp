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
 * Copyright (c) 2019, Open AI Lab
 * Author: haoluo@openailab.com
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>

#include "tengine_operations.h"
#include "tengine_c_api.h"

#define DEF_MODEL "models/mssd_quant_tflite.tmfile"
#define DEF_IMAGE "images/ssd_dog.jpg"
#define DEF_LABEL "models/coco_labels_list.txt"

#define DEF_HEIGHT 300
#define DEF_WIDTH  300

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

void LoadLabelFile(std::vector<std::string>& result, const char* fname)
{
    std::ifstream labels(fname);

    std::string line;
    while(std::getline(labels, line))
        result.push_back(line);
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

void post_process_ssd(const char* label_file, const char* image_file, tensor_t t0, tensor_t t1, tensor_t t2, tensor_t t3)
{
    float* boxes = ( float* )get_tensor_buffer(t0);
    float* classes = ( float* )get_tensor_buffer(t1);
    float* scores = ( float* )get_tensor_buffer(t2);
    float* num = ( float* )get_tensor_buffer(t3);

    std::vector<std::string> labels;
    LoadLabelFile(labels, label_file);
    image im = imread(image_file);
    std::cout<<"image : " << label_file << " , H x W: ( " <<im.h<<" x "<<im.w<<" ) \n"; 

    int max_num = num[0];
    printf("detect num : %d\n", max_num);
    std::cout << "--------------------------------------\n";
    for(int i = 0; i < max_num; i++)
    {
        if(scores[i] > 0.6)
        {
            int class_idx = classes[i];
            int x1 = boxes[i * 4] * im.w;
            int y1 = boxes[i * 4 + 1] * im.h;
            int x2 = boxes[i * 4 + 2] * im.w;
            int y2 = boxes[i * 4 + 3] * im.h;
            draw_box(im, x1, y1, x2, y2, 3, 255, 0, 0);
            put_label(im, labels[class_idx].c_str(), 1, x1, y1, 255, 0, 0);
            printf("%d -- %f -- %d -- box: %f,%f,%f,%f\n", i, scores[i], class_idx, boxes[i * 4],boxes[i * 4+1],boxes[i * 4+2],boxes[i * 4+3]);
            std::cout << "score: " << scores[i] << " ,class: " << class_idx << "  -> " << labels[class_idx]<<"  ";
            std::cout << "  (" << x1 << "," << y1 << ") , (" << x2 << "," << y2 << ")\n";
        }
    }
    save_image(im, "save");
}

int main(int argc, char* argv[])
{
    std::string model_file = DEF_MODEL;
    std::string label_file = DEF_LABEL;
    std::string image_file = DEF_IMAGE;
    int img_h = 0;
    int img_w = 0;
    int warm_count = 1;
    int repeat_count = 1;

    int res;
    std::vector<int> hw;
    while((res = getopt(argc, argv, "m:l:i:g:w:r:h")) != -1)
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
    // init tengine
    init_tengine();
    if(request_tengine_version("1.11") < 0)
        return 1;

    if(!check_file_exist(model_file) || !check_file_exist(label_file) || !check_file_exist(image_file))
    {
        return -1;
    }
    if(img_h == 0 || img_w == 0)
    {
        img_h = DEF_HEIGHT;
        img_w = DEF_WIDTH;
    }
    graph_t graph = create_graph(0, "tengine", model_file.c_str());
    if(graph == nullptr)
    {
        std::cout << "create graph failed!\n";
        return 1;
    }
    std::cout << "create graph done!\n";

    // input
    int img_size = img_h * img_w * 3;
    uint8_t* input_data = ( uint8_t* )malloc(sizeof(uint8_t) * img_size);

    int node_idx = 0;
    int tensor_idx = 0;
    tensor_t input_tensor = get_graph_input_tensor(graph, node_idx, tensor_idx);
    if(!check_tensor_valid(input_tensor))
    {
        printf("Get input node failed : node_idx: %d, tensor_idx: %d\n", node_idx, tensor_idx);
        return 1;
    }

    int dims[] = {1, img_h, img_w, 3};
    set_tensor_shape(input_tensor, dims, 4);
    if(prerun_graph(graph) != 0)
    {
        std::cout << "prerun _graph failed\n";
        return -1;
    }

    // warm up
    get_input_data_uint8(image_file.c_str(), input_data, img_h, img_w);
    set_tensor_buffer(input_tensor, input_data, img_size);
    FILE* pf = fopen("input.bin","wb");
    fwrite(input_data, 3, 300 * 300, pf);
    fclose(pf);
    if(run_graph(graph, 1) != 0)
    {
        std::cout << "run _graph failed\n";
        return -1;
    }

    for(int i = 0; i < warm_count; i++)
    {
        if(run_graph(graph, 1) < 0)
        {
            std::cout<<"run graph failed!\n";
            return -1;
        }
    }
    struct timeval t0, t1;
    float total_time = 0.f;
    float min_time = __DBL_MAX__;
    float max_time = -__DBL_MAX__;
    for(int i = 0; i < repeat_count; i++)
    {
        gettimeofday(&t0, NULL);
        run_graph(graph, 1);

        gettimeofday(&t1, NULL);
        float mytime = ( float )((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
        total_time += mytime;
        min_time = std::min(min_time, mytime);
        max_time = std::max(max_time, mytime);
    }
    std::cout << "--------------------------------------\n";
    std::cout << "\nRepeat " << repeat_count << " times, avg time per run is " << total_time / repeat_count << " ms\n" << "max time is " << max_time << " ms, min time is " << min_time << " ms\n";

    tensor_t boxes = get_graph_output_tensor(graph, 0, 0);
    tensor_t classes = get_graph_output_tensor(graph, 0, 1);
    tensor_t scores = get_graph_output_tensor(graph, 0, 2);
    tensor_t number = get_graph_output_tensor(graph, 0, 3);

    post_process_ssd(label_file.c_str(), image_file.c_str(),boxes, classes, scores, number);

    release_graph_tensor(boxes);
    release_graph_tensor(classes);
    release_graph_tensor(scores);
    release_graph_tensor(number);
    release_graph_tensor(input_tensor);
    free(input_data);

    postrun_graph(graph);
    destroy_graph(graph);

    release_tengine();

    return 0;
}

