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
 * Copyright (c) 2020, Open AI Lab
 * Author: qli@openailab.com
 */

#include <iostream>
#include <iomanip>
#include <sys/time.h>

#include "tengine_c_api.h"
#include "tengine_operations.h"

float show_threshold = 0.8;

struct Box
{
    float x0;
    float y0;
    float x1;
    float y1;
    int class_idx;
    float score;
};

void post_process_ssd(image img, float threshold, float* outdata, int num, const std::string& save_name)
{
    std::vector<Box> boxes;
    printf("--------------------------------------------\n");
    printf("Face id: prob%%\tBOX:( x0 , y0 ),( x1 , y1 )\n");
    printf("--------------------------------------------\n");
    int detected_face_num = 0;
    for(int i = 0; i < num; i++)
    {
        if(outdata[1] >= threshold)
        {
            detected_face_num += 1;
            Box box;
            box.class_idx = outdata[0];
            box.score = outdata[1];
            box.x0 = outdata[2] * img.w;
            box.y0 = outdata[3] * img.h;
            box.x1 = outdata[4] * img.w;
            box.y1 = outdata[5] * img.h;
            boxes.push_back(box);
            printf("Face %d:\t%.0f%%\t", detected_face_num, box.score * 100);
            printf("BOX:( %g , %g ),( %g , %g )\n", box.x0, box.y0, box.x1, box.y1);
        }
        outdata += 6;
    }
    printf("detect faces : %d \n", detected_face_num);
    for(int i = 0; i < ( int )boxes.size(); i++)
    {
        Box box = boxes[i];

        std::ostringstream score_str;
        score_str << box.score * 100;
        std::string labelstr = score_str.str();

        put_label(img, labelstr.c_str(), 0.02, box.x0, box.y0, 255, 255, 125);
        draw_box(img, box.x0, box.y0, box.x1, box.y1, 2, 125, 0, 125);
    }
    save_image(img, "Yu_FaceDetect.jpg");
    std::cout << "======================================\n";
    std::cout << "[DETECTED IMAGE SAVED]:\t"
              << "Yu_FaceDetect"
              << "\n";
    std::cout << "======================================\n";
}

void get_input_data(image img, float* input_data, int img_h, int img_w)
{
    int mean[3] = {104, 117, 123};
    for(int h = 0; h < img_h; h++)
    {
        for(int w = 0; w < img_w; w++)
        {
            for(int c = 0; c < 3; c++)
            {
                int hw = img_w * img_h;
                img.data[c * hw + h * img_w + w] = 0.007843 * (img.data[c * hw + h * img_w + w] - mean[c]);
            }
        }
    }
    memcpy(input_data, img.data, sizeof(float) * 3 * img_w * img_h);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "[Usage]: " << argv[0] << "<modelfile> <jpg> \n";
        return 0;
    }
    std::string mdl_name_ = argv[1];
    std::string image_file = argv[2];

    std::string save_file = "save.jpg";
    image im = imread(image_file.c_str());

    int img_w = 250;
    int img_h = 250;
    image resImage = resize_image(im, img_w, img_h);

    float* input_data = ( float* )malloc(sizeof(float) * img_h * img_w * 3);
    get_input_data(resImage, input_data, img_h, img_w);

    init_tengine();
    if(request_tengine_version("0.9") < 0)
        return 1;

    graph_t graph = create_graph(nullptr, "tengine",  mdl_name_.c_str());
    if(graph == nullptr)
    {
        std::cout << "Create graph0 failed\n";
        std::cout << "errno: " << get_tengine_errno() << "\n";
        return -1;
    }

    /* get input tensor */
    int node_idx = 0;
    int tensor_idx = 0;
    tensor_t input_tensor = get_graph_input_tensor(graph, node_idx, tensor_idx);
    int dims[] = {1, 3, img_h, img_w};
    set_tensor_shape(input_tensor, dims, 4);
    /* setup input buffer */
    if(set_tensor_buffer(input_tensor, input_data, 3 * img_h * img_w * 4) < 0)
    {
        std::printf("Set buffer for tensor failed\n");
        return -1;
    }

    prerun_graph(graph);

    // time run_graph
    int repeat_count = 1;
    const char* repeat = std::getenv("REPEAT_COUNT");
    if(repeat)
        repeat_count = std::strtoul(repeat, NULL, 10);

    struct timeval t0, t1;
    float avg_time = 0.f;
    gettimeofday(&t0, NULL);
    for(int i = 0; i < repeat_count; i++)
        run_graph(graph, 1);
    gettimeofday(&t1, NULL);
    float mytime = ( float )((t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec)) / 1000;
    avg_time += mytime;
    std::cout << "--------------------------------------\n";
    std::cout << "repeat " << repeat_count << " times, avg time per run is " << avg_time / repeat_count << " ms\n";

    // post process
    tensor_t out_tensor = get_graph_output_tensor(graph, 0, 0);    //"detection_out");
    int out_dim[4];
    get_tensor_shape(out_tensor, out_dim, 4);
    float* outdata = ( float* )get_tensor_buffer(out_tensor);
    int num = out_dim[1];

    post_process_ssd(im, show_threshold, outdata, num, save_file.c_str());

    // free
    release_graph_tensor(out_tensor);
    release_graph_tensor(input_tensor);
    postrun_graph(graph);
    destroy_graph(graph);
    free(input_data);
    release_tengine();

    return 0;
}
