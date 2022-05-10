#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model;

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float radian = rotation_angle / 180.0f * MY_PI;
    float sin_angle = std::sin(radian);
    float cos_angle = std::cos(radian);
    model << cos_angle, -sin_angle, 0.0f, 0.0f,
             sin_angle, cos_angle, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection;

    float half_height = std::tan(eye_fov / 2.0f * MY_PI / 180.0f);
    float half_width = aspect_ratio * half_height;
    projection << 1.0f / half_width, 0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f / half_height, 0.0f, 0.0f,
                  0.0f, 0.0f, (zNear + zFar) / (zNear - zFar), 2.0f * zNear * zFar / (zNear - zFar),
                  0.0f, 0.0f, -1.0f, 0.0f;
    
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Vector3f x_axis, y_axis;
    Vector3f z_axis = axis.normalized();
    if(z_axis.x() <= z_axis.y() && z_axis.x() <= z_axis.z()) {
        x_axis = Vector3f(0.f, -z_axis.z(), z_axis.y());
    } else if(z_axis.y() <= z_axis.x() && z_axis.y() <= z_axis.z()) {
        x_axis = Vector3f(-z_axis.z(), 0.f, z_axis.x());
    } else {
        x_axis = Vector3f(-z_axis.y(), z_axis.x(), 0.0f);
    }
    x_axis.normalize();
    y_axis = z_axis.cross(x_axis);

    Eigen::Matrix4f basis;
    basis << x_axis.x(),x_axis.y(), x_axis.z(), 0.0f,
             y_axis.x(),y_axis.y(), y_axis.z(), 0.0f,
             z_axis.x(),z_axis.y(), z_axis.z(), 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f;
    
    float radian = angle / 180.0f * MY_PI;
    float s = std::sin(radian);
    float c = std::cos(radian);
    Eigen::Matrix4f rotation_z;
    rotation_z << c, -s, 0.0f, 0.0f,
                  s, c, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f;

    return basis.transpose() * rotation_z * basis;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
