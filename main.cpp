#include "OpticalGraphRecognition/pixel_classifier.h"

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <filesystem>


int main(int argc, char* argv[]) {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    const std::filesystem::path input_dir(argv[1]);
    const std::filesystem::path output_dir(argv[2]);
    const std::filesystem::path sample_dir("sample1");

    const std::filesystem::path input_img = input_dir / sample_dir / "bundling.png";
    const std::filesystem::path output_img = output_dir / sample_dir / "classified.png";

    assert(std::filesystem::exists(input_img));

    LOG_INFO << "Read input image: " << input_img;

    cv::Mat source_image;
    source_image = cv::imread(input_img, cv::ImreadModes::IMREAD_COLOR);

    NOgr::PixelClassifier classifier(source_image);
    classifier.FindVertexes();
    classifier.ClassifyEdgePixels();

    const NOgr::OgrMat& classified_image = classifier.GetClassifiedImage();
    const cv::Mat colored_classified_image = classified_image.GetColoredImage();

    LOG_INFO << "Dump image to: " << output_img;

    cv::imwrite(output_img, colored_classified_image);

    return 0;
}

