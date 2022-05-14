//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) 
    {
        auto u_img = u * width;
        auto v_img = (1.0f - v) * height;
        auto floor_u_img = floor(u_img);
        auto floor_v_img = floor(v_img);
        auto weight_u = u_img - floor_u_img;
        auto weight_v = v_img - floor_v_img;

        auto c1 = image_data.at<cv::Vec3b>(floor_v_img, floor_u_img);
        auto c2 = image_data.at<cv::Vec3b>(floor_v_img, floor_u_img + 1.0f);
        auto c_h1 = c1 * (1.0f - weight_u) + c2 * weight_u;

        auto c3 = image_data.at<cv::Vec3b>(floor_v_img + 1.0f, floor_u_img);
        auto c4 = image_data.at<cv::Vec3b>(floor_v_img + 1.0f, floor_u_img + 1.0f);
        auto c_h2 = c3 * (1.0f - weight_u) + c4 * weight_u;

        auto color = c_h1 * (1.0f - weight_v) + c_h2 * weight_v;
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
};
#endif //RASTERIZER_TEXTURE_H
