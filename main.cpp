#include <optical_graph_recognition/algo_params/params.h>
#include <optical_graph_recognition/optical_graph_recognition.h>
#include <optical_graph_recognition/reporter.h>
#include <optical_graph_recognition/utils/opencv_utils.h>
#include <optical_graph_recognition/vertex/detectors.h>
#include <optical_graph_recognition/utils/debug.h>

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include <filesystem>
#include <optional>
#include <thread>
#include <mutex>


ogr::OpticalGraphRecognition ProcessImage(
        const std::filesystem::path& input_img,
        std::optional<ogr::VertexId> vertex,
        std::string union_strategy,
        const std::filesystem::path& common_output_dir
) {
    LOG_INFO << "Read input image: " << input_img;

    // Step 1: Read image
    cv::Mat colored_image = cv::imread(input_img, cv::ImreadModes::IMREAD_COLOR);
    cv::Mat grayscale_image = cv::imread(input_img, cv::ImreadModes::IMREAD_GRAYSCALE);

    LOG_INFO << "Image successfully read";

    // Step 2: Preprocess image and get thinning graph representation
    cv::Mat thinning_image = ogr::opencv::GetThinningImage(grayscale_image);

    LOG_INFO << "Image was thinned for morphological parsing";

    // Step 3.0: Morphological parsing graph image
    ogr::OpticalGraphRecognition ogr_algo(thinning_image);

    LOG_INFO << "Optical graph recognition initialized";

    // Step 3.1: Prepare vertex detecting strategy and use this strategy for detect vertexes in source image
    ogr::vertex::VertexPointsDetectorByColor vertex_detector(colored_image, {0, 0, 0});
    ogr_algo.DetectVertexes(vertex_detector);

    LOG_INFO << "All graph vertexes were detected from image";

    // Step 3.2: Detecting edges
    // Run algorithm of edges detecting
    ogr_algo.DetectEdges(vertex);

    if (union_strategy == "union") {
        ogr_algo.UnionFoundEdges();
    } else {
        ogr_algo.IntersectFoundEdges();
    }

    // Step 4: Detect bundling
    ogr_algo.BuildEdgeBundlingMap();

    // Step 5: Aesthetics evaluation
    ogr_algo.MarkCrossingsPoints();

    // Step 5: Print results

    // Dump algo results
    //    if (!ogr::debug::DevDirPath.empty()) {
    //        output_dir = std::filesystem::path(ogr::debug::DevDirPath);
    //    }
    std::filesystem::path output_dir = common_output_dir / input_img.stem();
    ogr_algo.DumpResultImages(output_dir, vertex);

    return ogr_algo;
}


int main(int argc, char* argv[]) {
    using Fpath = std::filesystem::path;
    namespace FS = std::filesystem;

    CLI::App app{"Aesthetic metrics evaluation of bundling visualization techniques"};

    Fpath input_dir;
    std::string baseline_name;
    Fpath output_dir;
    std::string log_level;
    std::optional<ogr::VertexId> vertex;
    std::string union_strategy;

    auto check_path = [](const std::string& path) {
        std::string error_msg;

        if (!FS::exists(path)) {
            error_msg = "File does not exist, check input path";
        }

        return error_msg;
    };

    app.add_option("-i,--input", input_dir, "Input images dir path")
        ->required()
        ->check(check_path);
    app.add_option("-b,--baseline-name", baseline_name, "Input image baseline")
        ->default_val("baseline.png");
    app.add_option("-o,--output", output_dir, "Output results dir path")
        ->required()
        ->check(check_path);
    app.add_option("--log-level", log_level, "Log level: info, debug, none")
        ->envname("LOG_LEVEL")
        ->default_val("info");
    app.add_option("--dev-dir", ogr::debug::DevDirPath, "Path dir to dev steps dump")
        ->check(check_path);
    app.add_option("--vertex", vertex, "Run algo only for particular vertex")
        ->default_val(std::nullopt);
    app.add_flag("--dump-intermediate", ogr::debug::DumpIntermediateResults)
        ->default_val(false);
    auto* algo_params = app.add_option_group("Algo params", "Parameters of ogr algorithm");
    algo_params->add_option("--curvature", ogr::kStableStateAngleDiffLocalThreshold, "Acceptable steps curvature")
        ->default_val(13.0);
    algo_params->add_option("--stable-diff", ogr::kStableStateAngleDiffThreshold, "Acceptable angle diff threshold between stable edge parts")
        ->default_val(20.0);
    algo_params->add_option("--state-diff", ogr::kAngleDiffThreshold, "Acceptable diff angle between consecutive several steps")
        ->default_val(25.0);
    algo_params->add_option("--edges-union", union_strategy, "Union found edges strategy: union, intersection")
        ->default_val("union");

    CLI11_PARSE(app, argc, argv);

    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;

    if (log_level == "debug") {
        plog::init(plog::debug, &consoleAppender);
    } else if (log_level == "info") {
        plog::init(plog::info, &consoleAppender);
    } else {
        plog::init(plog::none, &consoleAppender);
    }

    Fpath baseline_path{input_dir / baseline_name};
    if (!FS::exists(baseline_path)) {
        throw std::runtime_error{"Baseline not valid path"};
    }

    std::vector<Fpath> algo_images_paths;
    for (const auto file : std::filesystem::directory_iterator{input_dir}) {
        Fpath file_path = file.path();
        if (file_path.filename() == baseline_name) {
            continue;
        }

        algo_images_paths.emplace_back(std::move(file_path));
    }

    ogr::OpticalGraphRecognition baseline_algo = ProcessImage(baseline_path, vertex, union_strategy, output_dir);
    std::vector<ogr::OpticalGraphRecognition> evaluated_algos;
    for (const auto& algo_image_path : algo_images_paths) {
        evaluated_algos.emplace_back(ProcessImage(algo_image_path, vertex, union_strategy, output_dir));
    }

    ogr::MakeReport(baseline_algo, evaluated_algos);

    return 0;
}
