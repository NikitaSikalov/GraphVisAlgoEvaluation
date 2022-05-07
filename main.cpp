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


using Fpath = std::filesystem::path;
namespace FS = std::filesystem;

struct OgrParams {
    double curvature;
    double stable_diff;
    double state_diff;
};

struct InputCliParams {
    Fpath input_dir;
    std::string baseline_name;
    Fpath output_dir;
    std::string log_level;
    std::optional<ogr::VertexId> vertex;
    std::optional<std::string> filter;
    std::string union_strategy;
    bool only_report;

    OgrParams ogr_baseline_params;
    OgrParams ogr_algo_params;
};

ogr::OpticalGraphRecognition ProcessImage(const std::filesystem::path& input_img, const OgrParams& ogr_params, const InputCliParams& input_params) {
    // Step 1: Read image
    LOG_INFO << "Read input image: " << input_img;

    cv::Mat colored_image = cv::imread(input_img, cv::ImreadModes::IMREAD_COLOR);
    cv::Mat grayscale_image = cv::imread(input_img, cv::ImreadModes::IMREAD_GRAYSCALE);

    LOG_INFO << "Image successfully read";

    // Step 2: Preprocess image and get thinning graph representation
    cv::Mat thinning_image = ogr::opencv::GetThinningImage(grayscale_image);

    LOG_INFO << "Image was thinned for morphological parsing";

    // Step 3.0: Morphological parsing graph image
    // Prepare ogr algo params
    ogr::kStableStateAngleDiffLocalThreshold = ogr_params.curvature;
    ogr::kStableStateAngleDiffThreshold = ogr_params.stable_diff;
    ogr::kAngleDiffThreshold = ogr_params.state_diff;

    ogr::OpticalGraphRecognition ogr_algo{thinning_image};
    ogr_algo.UpdateIncUsage(colored_image);

    LOG_INFO << "Optical graph recognition initialized";

    // Step 3.1: Prepare vertex detecting strategy and use this strategy for detect vertexes in source image
    ogr::vertex::VertexPointsDetectorByColor vertex_detector(colored_image, {0, 0, 0});
    ogr_algo.DetectVertexes(vertex_detector);

    LOG_INFO << "All graph vertexes were detected from image";

    // Step 3.2: Detecting edges
    // Run algorithm of edges detecting
    ogr_algo.DetectEdges(input_params.vertex);

    if (input_params.union_strategy == "union") {
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

    if (!input_params.only_report) {
        Fpath output_dir = input_params.output_dir / input_img.stem();
        if (!FS::exists(output_dir)) {
            FS::create_directory(output_dir);
        }

        ogr_algo.DumpResultImages(output_dir, input_params.vertex);
    }

    return ogr_algo;
}


int main(int argc, char* argv[]) {
    CLI::App app{"Aesthetic metrics evaluation of bundling visualization techniques"};

    InputCliParams cli_params;

    auto check_path = [](const std::string& path) {
        std::string error_msg;

        if (!FS::exists(path)) {
            error_msg = "File does not exist, check input path";
        }

        return error_msg;
    };

    // General required params
    app.add_option("-i,--input", cli_params.input_dir, "Input images dir path")
        ->required()
        ->check(check_path);
    app.add_option("-b,--baseline-name", cli_params.baseline_name, "Input image baseline")
        ->default_val("baseline");
    app.add_option("-o,--output", cli_params.output_dir, "Output results dir path")
        ->required()
        ->check(check_path);

    // Infra/Dev params
    app.add_option("--log-level", cli_params.log_level, "Log level: info, debug, none")
        ->envname("LOG_LEVEL")
        ->default_val("info");
    app.add_option("--dev-dir", ogr::debug::DevDirPath, "Path dir to dev steps dump")
        ->check(check_path);
    app.add_option("--vertex", cli_params.vertex, "Run algo only for particular vertex")
        ->default_val(std::nullopt);
    app.add_option("--filter", cli_params.filter, "Run algo only for particular image file")
        ->default_val(std::nullopt);
    app.add_flag("--dump-intermediate", ogr::debug::DumpIntermediateResults)
        ->default_val(false);
    app.add_flag("--only-report", cli_params.only_report, "Show only report of algo evaluation")
        ->default_val(false);

    // Ogr algo params
    auto* algo_input_params = app.add_option_group("Algo params", "Parameters of ogr algorithm");

    // Add ogr algo params for bundling algorithms (with curved edges; more flexible)
    algo_input_params->add_option("--curvature", cli_params.ogr_algo_params.curvature, "Acceptable steps curvature")
        ->default_val(20.0);
    algo_input_params->add_option("--stable-diff", cli_params.ogr_algo_params.stable_diff, "Acceptable angle diff threshold between stable edge parts")
        ->default_val(15.0);
    algo_input_params->add_option("--state-diff", cli_params.ogr_algo_params.state_diff, "Acceptable diff angle between consecutive several steps")
        ->default_val(40.0);

    // Add ogr params for baseline visualisation (with straight edges; more strict)
    algo_input_params->add_option("--baseline-curvature", cli_params.ogr_baseline_params.curvature, "Acceptable steps curvature (baseline)")
        ->default_val(13.0);
    algo_input_params->add_option("--baseline-stable-diff", cli_params.ogr_baseline_params.stable_diff, "Acceptable angle diff threshold between stable edge parts (baseline)")
        ->default_val(9.0);
    algo_input_params->add_option("--baseline-state-diff", cli_params.ogr_baseline_params.state_diff, "Acceptable diff angle between consecutive several steps (baseline)")
        ->default_val(30.0);

    // Edges union/intersection strategy
    algo_input_params->add_option("--edges-union", cli_params.union_strategy, "Union found edges strategy: union, intersection")
        ->default_val("union");

    CLI11_PARSE(app, argc, argv);

    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender{plog::OutputStream::streamStdErr};

    if (cli_params.log_level == "debug") {
        plog::init(plog::debug, &consoleAppender);
    } else if (cli_params.log_level == "info") {
        plog::init(plog::info, &consoleAppender);
    } else {
        plog::init(plog::none, &consoleAppender);
    }

    std::vector<Fpath> algo_images_paths;
    Fpath baseline_path;
    for (const auto file : FS::directory_iterator{cli_params.input_dir}) {
        Fpath file_path = file.path();
        const std::string stemmed_name = file_path.stem();

        if (cli_params.filter.has_value() && stemmed_name == *cli_params.filter) {
            OgrParams ogr_params = *cli_params.filter == cli_params.baseline_name ? cli_params.ogr_baseline_params : cli_params.ogr_algo_params;
            ProcessImage(file_path, ogr_params, cli_params);
            return 0;
        }

        if (stemmed_name == cli_params.baseline_name) {
            baseline_path = std::move(file_path);
            continue;
        }

        algo_images_paths.emplace_back(std::move(file_path));
    }

    if (!FS::exists(baseline_path)) {
        throw std::runtime_error{"Baseline not valid path"};
    }

    ogr::OpticalGraphRecognition baseline_algo = ProcessImage(baseline_path, cli_params.ogr_baseline_params, cli_params);

    std::vector<ogr::OpticalGraphRecognition> evaluated_algos;
    for (const auto& algo_image_path : algo_images_paths) {
        evaluated_algos.emplace_back(ProcessImage(algo_image_path, cli_params.ogr_algo_params, cli_params));
    }

    ogr::MakeReport(baseline_algo, evaluated_algos);

    return 0;
}
