// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Eigen::Vector3f p = Eigen::Vector3f(x, y, 0.0f);
    Eigen::Vector3f p1 = Eigen::Vector3f(_v[0].x(), _v[0].y(), 0.0f);
    Eigen::Vector3f p2 = Eigen::Vector3f(_v[1].x(), _v[1].y(), 0.0f);
    Eigen::Vector3f p3 = Eigen::Vector3f(_v[2].x(), _v[2].y(), 0.0f);
    return ((p - p1).cross(p2 - p1)).z() <= 0.0 &&
            ((p - p2).cross(p3 - p2)).z() <= 0.0 &&
            ((p - p3).cross(p1 - p3)).z() <= 0.0;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

std::vector<Eigen::Vector3f>& rst::rasterizer::frame_buffer() 
{ 
    int _x;
    Eigen::Vector3f temp;
    int offset1 = 0;
    int offset2 = width << 1;
    int target_i = 0;
    for(auto y = 0; y < height; ++y) {
        _x = 0;
        for(auto x = 0; x < width; ++x) {
            temp = super_sampling_frame_buf[offset1 + _x];
            temp += super_sampling_frame_buf[offset1 + _x + 1];
            temp += super_sampling_frame_buf[offset2 + _x];
            temp += super_sampling_frame_buf[offset2 + _x + 1];
            temp /= 4.0f;
            _x += 2;
            frame_buf[target_i++] = temp;
        }
        offset1 += width << 2;
        offset2 += width << 2;
    }
    return frame_buf; 
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*2*(vert.x()+1.0);
            vert.y() = 0.5*height*2*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

    Eigen::Vector2f min(width * 2, height * 2);
    Eigen::Vector2f max(0, 0);

    for(auto& vertex: v) {
        if(vertex.x() < min.x()) {
            min.x() = vertex.x();
        }
        if(vertex.x() > max.x()) {
            max.x() = vertex.x();
        }
        if(vertex.y() < min.y()) {
            min.y() = vertex.y();
        }
        if(vertex.y() > max.y()) {
            max.y() = vertex.y();
        }
    }

    for(int y = int(min.y()); y < max.y(); ++y) {
        for(int x = int(min.x()); x < max.x(); ++x) {
            /*
            if(!insideTriangle(x, y, t.v)){
                continue;
            }
            set_pixel(Vector3f(x, y, 0.0f), Eigen::Vector3f(255.0f, 0.0f, 0.0f));
            */
            auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            if(alpha < 0.0f || beta < 0.0f || gamma < 0.0f) {
                continue;
            }
            float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;
            int index = get_index(x, y);
            if(z_interpolated > super_sampling_depth_buf[index]) {
                continue;
            }
            super_sampling_depth_buf[index] = z_interpolated;
            Eigen::Vector3f color = w_reciprocal * (alpha * t.color[0] / v[0].w() + beta * t.color[1] / v[1].w() + gamma * t.color[2] / v[2].w());
            set_pixel(Vector3f(x, y, 0.0f), color * 255.0);
        }
    }

}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(super_sampling_frame_buf.begin(), super_sampling_frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(super_sampling_depth_buf.begin(), super_sampling_depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    super_sampling_frame_buf.resize(w * h * 4);
    super_sampling_depth_buf.resize(w * h * 4);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height * 2 -1-y)*width * 2 + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height*2-1-point.y())*width*2 + point.x();
    super_sampling_frame_buf[ind] = color;

}

// clang-format on