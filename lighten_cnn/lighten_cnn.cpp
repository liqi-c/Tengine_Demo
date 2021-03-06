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
 * Author: chunyinglv@openailab.com
 */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sys/time.h>
#include <math.h>
#include "tengine_c_api.h"
//#include "common.hpp"

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

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

void get_data(void* buffer, int datasize, const char* fname)
{
    // read data
    FILE* data_fp = fopen(fname, "rb");
    if(!data_fp)
        printf("data can not be open\n");

    size_t n = fread(buffer, sizeof(float), datasize, data_fp);
    if(( int )n < datasize)
        printf("data read error\n");

    fclose(data_fp);
}

void maxerr(float* pred, float* gt, int size)
{
    float maxError = 0.f;
    for(int i = 0; i < size; i++)
    {
        maxError = MAX(( float )fabs(gt[i] - *(pred + i)), maxError);
    }
    printf("====================================\n");
    printf("maxError is %f\n", maxError);
    printf("====================================\n");
}

int repeat_count = 1;

int main(int argc, char* argv[])
{
    int ret = -1;
    std::string model_dir = "./models";

    if(argc == 1)
    {
        std::cout << "[Usage]: " << argv[0] << " <model_dir>\n";
    }

    if(argc > 1)
        model_dir = argv[1];

    // init tengine
    if(init_tengine() < 0)
    {
        std::cout << " init tengine failed\n";
        return 1;
    }
    if(request_tengine_version("0.9") != 1)
    {
        std::cout << " request tengine version failed\n";
        return 1;
    }

    // load model
    std::string proto_name_ = model_dir + "/LightenedCNN_B.prototxt";
    std::string mdl_name_ = model_dir + "/LightenedCNN_B.caffemodel";
    if(!check_file_exist(proto_name_) or (!check_file_exist(mdl_name_)))
    {
        return 1;
    }
    // create graph
    graph_t graph = create_graph(nullptr, "caffe", proto_name_.c_str(), mdl_name_.c_str());

    if(graph == nullptr)
    {
        std::cout << "create graph0 failed\n";
        return 1;
    }
    std::cout << "create graph done!\n";

    // input
    int img_h = 128;
    int img_w = 128;
    int img_size = img_h * img_w;
    float* input_data = ( float* )malloc(sizeof(float) * img_size);
    // for(int i=0;i<img_size;i++) input_data[i]=(1%128)/255.f;

    std::string input_fname = model_dir + "/data_16384";
    if(!check_file_exist(input_fname))
    {
        return 1;
    }
    get_data(input_data, img_size, input_fname.c_str());

    tensor_t input_tensor = get_graph_input_tensor(graph, 0, 0);
    int dims[] = {1, 1, img_h, img_w};
    set_tensor_shape(input_tensor, dims, 4);
    if(set_tensor_buffer(input_tensor, input_data, img_size * 4) < 0)
    {
        std::printf("set buffer for input tensor failed\n");
        return -1;
    }

    ret = prerun_graph(graph);
    if(ret != 0)
    {
        std::cout << "Prerun graph failed, errno: " << get_tengine_errno() << "\n";
        return 1;
    }

    int repeat_count = 1;
    const char* repeat = std::getenv("REPEAT_COUNT");
    if(repeat)
        repeat_count = std::strtoul(repeat, NULL, 10);

    struct timeval t0, t1;
    float total_time = 0.f;
    for(int i = 0; i < repeat_count; i++)
    {
        gettimeofday(&t0, NULL);
        ret = run_graph(graph, 1);
        if(ret != 0)
        {
            std::cout << "Run graph failed, errno: " << get_tengine_errno() << "\n";
            return 1;
        }
        gettimeofday(&t1, NULL);

        float mytime = ( float )((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
        total_time += mytime;
    }
    std::cout << "--------------------------------------\n";
    std::cout << "repeat " << repeat_count << " times, avg time per run is " << total_time / repeat_count << " ms\n";

    free(input_data);

    int size1 = 256;
    tensor_t mytensor1 = get_graph_tensor(graph, "eltwise_fc1");
    float* data1 = ( float* )get_tensor_buffer(mytensor1);
    float* out1 = ( float* )malloc(sizeof(float) * size1);

    std::string out_data_file = model_dir + "/eltwise_fc1_256";
    if(!check_file_exist(out_data_file))
    {
        return 1;
    }
    get_data(out1, size1, out_data_file.c_str());
    maxerr(data1, out1, size1);

    free(out1);
    release_graph_tensor(mytensor1);
    ret = postrun_graph(graph);
    if(ret != 0)
    {
        std::cout << "Postrun graph failed, errno: " << get_tengine_errno() << "\n";
        return 1;
    }

    destroy_graph(graph);
    release_tengine();

    return 0;
}
