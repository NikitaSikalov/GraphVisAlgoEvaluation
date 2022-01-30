#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <filesystem>
#include <string>


#define DUMP(image) \
    cv::imwrite(output / (std::string{#image} + ".jpeg"), image)

int main() {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    std::filesystem::path input("images");
    std::filesystem::path output("output");

    std::filesystem::path input_image = input / "sample.jpeg";

    LOG_INFO << "Read input image: " << input_image;

    cv::Mat image;
    image = cv::imread(input_image, cv::ImreadModes::IMREAD_GRAYSCALE);
    assert(!image.empty());

    LOG_INFO << "Preprocess image";

    LOG_INFO << "Apply gaussian blur";

    cv::Mat blurred_image;
    cv::GaussianBlur(image, blurred_image, {5, 5}, 0);
    DUMP(blurred_image);

    LOG_INFO << "Apply binarization to image";

    cv::Mat binary_image;
    cv::threshold(blurred_image, binary_image, 250, 255, cv::THRESH_BINARY_INV);
    DUMP(binary_image);

    LOG_INFO << "Remove some salt from image";

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, {5, 5});
    cv::Mat preprocessed_image;
    morphologyEx(binary_image, preprocessed_image, cv::MORPH_OPEN, kernel);
    DUMP(preprocessed_image);

    LOG_INFO << "Image successfully preprocessed";

    cv::Mat skel;
    cv::ximgproc::thinning(preprocessed_image, skel);
    DUMP(skel);

    LOG_INFO << "Skeletonization successfully applied";
    LOG_INFO << "See result in " << std::filesystem::absolute(output / "skel.jpeg");

    return 0;
}

