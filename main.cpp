#include <opencv2/opencv.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    cv::Mat image;
    image = cv::imread(argv[1]);
    std::cout << image.size() << std::endl;
    return 0;
}

