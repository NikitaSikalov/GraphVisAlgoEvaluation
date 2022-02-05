#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <filesystem>
#include <string>

#define DUMP(image) \
    auto filepath##image = output / (std::string{#image} + ".jpeg");     \
    cv::imwrite(filepath##image, image); \
    LOG_INFO << "Dump skel image to " << std::filesystem::absolute(filepath##image);

enum class PixelType {
    EMPTY,
    UNKNOWN,
    VERTEX,
    EDGE,
    PORTAL,
    CROSSING,

    MAX
};

const cv::Scalar colorsPalette[] = {
        CV_RGB(0,0, 0),
        CV_RGB(255, 255, 255),
        CV_RGB(255, 0, 0),
        CV_RGB(0, 0, 255),
        CV_RGB(112, 48, 162),
        CV_RGB(1, 112, 193),
        CV_RGB(255, 102, 0),
        CV_RGB(255, 255, 1),
        CV_RGB(0, 175, 82),
};

double Distance(const cv::Vec3b& pixel1, const cv::Vec3b& pixel2) {
    double res = 0;
    constexpr size_t rgb_channels = 3;

    for (int i = 0; i < rgb_channels; ++i) {
        double x = static_cast<double>(pixel1[i]) - pixel2[i];
        res += x * x;
    }

    return sqrt(res);
}

cv::Mat GetVertexesMask(const cv::Mat& sourceImage, const cv::Vec3b& vertexColor = {0, 0, 0}, double threshold = 80.0) {
    cv::Mat vertexesMask(sourceImage.size(), CV_8UC1);

    for (int row = 0; row < sourceImage.rows; ++row) {
        for (int col = 0; col < sourceImage.cols; ++col) {
            const auto& pixel = sourceImage.at<cv::Vec3b>(row, col);

            if (Distance(pixel, vertexColor) <= threshold) {
                vertexesMask.at<uchar>(row, col) = 1;
            } else {
                vertexesMask.at<uchar>(row, col) = 0;
            }
        }
    }

    return vertexesMask;
}

cv::Mat GetReducedImage(const cv::Mat& sourceImage) {
    cv::Mat blurredImage;
    cv::GaussianBlur(sourceImage, blurredImage, {5, 5}, 0);

    cv::Mat binaryImage;
    cv::threshold(blurredImage, binaryImage, 250, 255, cv::THRESH_BINARY_INV);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, {5, 5});
    cv::Mat preprocessedImage;
    morphologyEx(binaryImage, preprocessedImage, cv::MORPH_OPEN, kernel);

    cv::Mat skel;
    cv::ximgproc::thinning(preprocessedImage, skel);

    return skel;
}

cv::Mat MarkPixels(const cv::Mat& image, const cv::Mat& mask, PixelType type) {
    cv::Mat output;
    image.copyTo(output);

    for (int row = 0; row < image.rows; ++row) {
        for (int col = 0; col < image.cols; ++col) {
            uchar& outputValue = output.at<uchar>(row, col);
            if (mask.at<uchar>(row, col) && image.at<uchar>(row, col)) {
                outputValue = static_cast<uchar>(type);
            }
            if (outputValue >= static_cast<uchar>(PixelType::MAX)) {
                outputValue = static_cast<uchar>(PixelType::UNKNOWN);
            }
        }
    }

    return output;
}

cv::Mat ColorizeTypedPixels(const cv::Mat& image) {
    cv::Mat output(image.size(), CV_64FC4);

    for (int row = 0; row < image.rows; ++row) {
        for (int col = 0; col < image.cols; ++col) {
            const uchar ptype = image.at<uchar>(row, col);
            assert(ptype < static_cast<uchar>(PixelType::MAX));

            output.at<cv::Scalar>(row, col) = colorsPalette[ptype];
        }
    }

    return output;
}

int main() {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    std::filesystem::path input("images");
    std::filesystem::path output("output");

    std::filesystem::path imgPath = input / "sample.jpeg";
    assert(std::filesystem::exists(imgPath));

    LOG_INFO << "Read input image: " << imgPath;

    cv::Mat sourceImage;
    sourceImage = cv::imread(imgPath, cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat coloredImage;
    coloredImage = cv::imread(imgPath, cv::ImreadModes::IMREAD_COLOR);

    cv::Mat skel = GetReducedImage(sourceImage);
    DUMP(skel);

    cv::Mat vertexesMask = GetVertexesMask(coloredImage);

    cv::Mat typedSkel = MarkPixels(skel, vertexesMask, PixelType::VERTEX);
    cv::Mat coloredSkel = ColorizeTypedPixels(typedSkel);
    DUMP(coloredSkel);

    return 0;
}

