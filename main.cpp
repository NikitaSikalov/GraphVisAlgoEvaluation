#include <opencv2/opencv.hpp>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>

int main(int argc, char* argv[]) {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);
    PLOG_INFO << "Ok, log is working";
    PLOG_INFO << "Yet another message";

    cv::Mat image;
    image = cv::imread("images/sample.png");
    return 0;
}

