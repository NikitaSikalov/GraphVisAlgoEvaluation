#include <optical_graph_recognition/optical_graph_recognition.h>
#include <optical_graph_recognition/utils/opencv_utils.h>
#include <optical_graph_recognition/vertex/detectors.h>

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <filesystem>


int main(int argc, char* argv[]) {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);

    const std::filesystem::path input_dir(argv[1]);
    const std::filesystem::path output_dir(argv[2]);
    const std::filesystem::path sample_dir("sample3");

    const std::string file_name = "bundling";
    const std::string input_ext = ".png";
    const std::string output_ext = ".png";
    const std::filesystem::path input_img = input_dir / sample_dir / (file_name + input_ext);
    const std::string result_file_name = file_name + "_ogr_result" + output_ext;
    const std::filesystem::path output_img = output_dir / sample_dir / result_file_name;

    assert(std::filesystem::exists(input_img));

    LOG_INFO << "Read input image: " << input_img;

    // Step 1: Read image
    cv::Mat colored_image = cv::imread(input_img, cv::ImreadModes::IMREAD_COLOR);
    cv::Mat grayscale_image = cv::imread(input_img, cv::ImreadModes::IMREAD_GRAYSCALE);

    LOG_INFO << "Image successfully read";

    // Step 2: Preprocess image and get thinning graph representation
    cv::Mat thinning_image = ogr::opencv::GetThinningImage(grayscale_image);

    LOG_INFO << "Image was thinned for morphological parsing";

    // Step 3: Morphological parsing graph image
    ogr::OpticalGraphRecognition ogr_algo(thinning_image);

    LOG_INFO << "Optical graph recognition initialized";

    // Step 3.1: Prepare vertex detecting strategy and use this strategy for detect vertexes in source image
    ogr::vertex::VertexPointsDetectorByColor vertex_detector(colored_image, {0, 0, 0});
    ogr_algo.DetectVertexes(vertex_detector);

    LOG_INFO << "All graph vertexes were detected from image";

    // Step 3.2: Parsing edges ... (WIP)

    // Output results
    LOG_INFO << "Number of vertexes is " << ogr_algo.GetVertexes().size();
    LOG_INFO << "Number of edges is " << ogr_algo.GetEdges().size();

    return 0;
}
