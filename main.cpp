#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>

#include <filesystem>
#include <string>
#include <unordered_set>
#include <queue>
#include <utility>
#include <array>

#define DUMP(image) \
    auto filepath##image = output / (std::string{#image} + ".png");     \
    cv::imwrite(filepath##image, image); \
    LOG_INFO << "Dump " << #image << " to " << std::filesystem::absolute(filepath##image);

enum class PixelType {
    EMPTY,
    UNKNOWN,
    VERTEX,
    EDGE,
    CROSSING,

    MAX
};

const cv::Vec3b colorsPalette[] = {
        // {Blue, Green, Red}
        cv::Vec3b{0, 0, 0},         // EMPTY
        cv::Vec3b{255, 255, 255},   // UNKNOWN
        cv::Vec3b{255, 0, 0},       // VERTEX
        cv::Vec3b{0, 255, 0},       // EDGE
        cv::Vec3b{0, 0, 255}        // CROSSING
};

template <>
class std::hash<cv::Point> {
public:
    uint64_t operator()(const cv::Point& point) const {
        auto hashFn = std::hash<int>{};
        uint64_t hash1 = hashFn(point.x);
        uint64_t hash2 = hashFn(point.y);
        return hash1 ^ hash2;
    }
};

template <class TItem, size_t MaxSize>
class StackVector {
public:
    class Iterator;
public:
    StackVector() = default;

    void PushBack(const TItem& item) {
        assert(size_ < MaxSize);

        array_[size_] = item;
        size_++;
    }

    Iterator begin() const {
        // FIXME: Create ConstIterator
        TItem* front = const_cast<TItem*>(&array_[0]);
        return Iterator(front);
    }

    Iterator end() const {
        // FIXME: Create ConstIterator
        TItem* front = const_cast<TItem*>(&array_[0]);
        return Iterator(front + size_);
    }

    bool Contains(const TItem& other) const {
        for (const auto& item : *this) {
            if (item == other) {
                return true;
            }
        }

        return false;
    }

    size_t Size() const {
        return size_;
    }

public:
    class Iterator {
    public:
        Iterator(TItem* pitem) : pitem_(pitem) {}

        Iterator& operator++() {
            pitem_++;
            return *this;
        }

        bool operator!=(const Iterator& rhs) {
            return pitem_ != rhs.pitem_;
        }

        TItem& operator*() {
            return *pitem_;
        }

    private:
        TItem* pitem_;
    };

private:
    size_t size_{0};
    std::array<TItem, MaxSize> array_;
};

StackVector<cv::Point, 8> Get8Neighborhood(const cv::Mat& image, const cv::Point& pixel) {
    StackVector<cv::Point, 8> result;
    std::array<cv::Point, 8> allNeighbours {
            cv::Point{pixel.x - 1, pixel.y},
            cv::Point{pixel.x, pixel.y - 1},
            cv::Point{pixel.x, pixel.y + 1},
            cv::Point{pixel.x + 1, pixel.y},
            cv::Point{pixel.x - 1, pixel.y - 1},
            cv::Point{pixel.x + 1, pixel.y + 1},
            cv::Point{pixel.x + 1, pixel.y - 1},
            cv::Point{pixel.x - 1, pixel.y + 1}
    };

    for (const auto& point : allNeighbours) {
        if (point.x < 0 || point.y < 0) {
            continue;
        }
        if (point.x > image.cols || point.y > image.rows) {
            continue;
        }

        auto type = static_cast<PixelType>(image.at<uchar>(point));
        if (type == PixelType::EMPTY) {
            continue;
        }

        result.PushBack(point);
    }

    return result;
}

class GraphCrawler {
public:
    GraphCrawler(cv::Mat* image, std::unordered_set<cv::Point>* used, cv::Point start) : image_(image), used_(used), prev_(start) {
        pixelsQueue_.push(start);
    }

    bool Step(cv::Point* pointForNextCrawler) {
        cv::Point current = pixelsQueue_.front();
        pixelsQueue_.pop();

        auto allNeighbours = Get8Neighborhood(*image_, current);
        if (!allNeighbours.Contains(prev_) && current != prev_) {
            image_->at<uchar>(current) = static_cast<uchar>(PixelType::CROSSING);
            *pointForNextCrawler = current;
            return false;
        }

        if (current != prev_) {
            image_->at<uchar>(current) = static_cast<uchar>(PixelType::EDGE);
        }

        auto notTraversedPoints = FilterFromUsed(allNeighbours);
        for (const auto& next : notTraversedPoints) {
            used_->insert(next);
            pixelsQueue_.push(next);
        }

        prev_ = current;
        return true;
    }

    bool Empty() const {
        return pixelsQueue_.empty();
    }

private:
    cv::Mat* image_;
    std::unordered_set<cv::Point>* used_;
    cv::Point prev_;
    std::queue<cv::Point> pixelsQueue_;

private:
    template<class VectorContainer>
    VectorContainer FilterFromUsed(const VectorContainer& container) {
        VectorContainer filtered;
        for (const cv::Point& point : container) {
            if (used_->contains(point)) {
                continue;
            }

            auto type = static_cast<PixelType>(image_->at<uchar>(point));
            if (type != PixelType::UNKNOWN) {
                continue;
            }
            filtered.PushBack(point);
        }

        return filtered;
    }

};

class PixelClassifier {
public:
    PixelClassifier(cv::Mat* image) : image_(image) {
        TryAddCrawler();
    }

    void Run() {
        while (!crawlers_.empty()) {
            while (!crawlers_.empty()) {
                RunCrawler(&crawlers_.front());
                crawlers_.pop();
            }

            TryAddCrawler();
        }
    }

    void OptimizeCrossings() {
        LOG_INFO << "Optimize crossing points, initial size = " << crossings_.size();

        std::unordered_set<cv::Point> crossingsCopy = crossings_;
        for (const cv::Point& crossing : crossingsCopy) {
            image_->at<uchar>(crossing) = static_cast<uchar>(PixelType::EDGE);
            if (!crossings_.contains(crossing)) {
                continue;
            }

            OptimizeCrossing(crossing);
        }

        for (const cv::Point& crossing : crossings_) {
            image_->at<uchar>(crossing) = static_cast<uchar>(PixelType::CROSSING);
        }

        LOG_INFO << "Crossing points optimized, actual crossings cnt = " << crossings_.size();
    }

private:
    std::unordered_set<cv::Point> used_;
    std::queue<GraphCrawler> crawlers_;
    cv::Mat* image_;
    std::unordered_set<cv::Point> crossings_;

private:
    void TryAddCrawler() {
        for (int y = 0; y < image_->rows; ++y) {
            for (int x = 0; x < image_->cols; ++x) {
                cv::Point point{x, y};
                if (GetPointType(point) == PixelType::VERTEX) {
                    for (const cv::Point& neighbour : Get8Neighborhood(*image_, point)) {
                        if (GetPointType(neighbour) == PixelType::UNKNOWN) {
                            LOG_DEBUG << "Explicitly add crawler with start point " << point;

                            crawlers_.push(GraphCrawler(image_, &used_, point));
                            return;
                        }
                    }
                }
            }
        }

        LOG_DEBUG << "No new crawlers found, exit";
    }

    void RunCrawler(GraphCrawler* crawler) {
        while (!crawler->Empty()) {
            cv::Point crossingPoint;
            if (!crawler->Step(&crossingPoint)) {
                LOG_DEBUG << "Found crossing point " << crossingPoint;
                LOG_DEBUG << "Add new crawler with start point " << crossingPoint;
                crossings_.insert(crossingPoint);
                crawlers_.push(GraphCrawler(image_, &used_, crossingPoint));
            }
        }
    }

    void OptimizeCrossing(const cv::Point& crossing) {
        StackVector<cv::Point, 8> localCrossings;
        crossings_.erase(crossing);
        for (const cv::Point& neighbour : Get8Neighborhood(*image_, crossing)) {
            if (GetPointType(neighbour) == PixelType::CROSSING) {
                crossings_.erase(neighbour);
                localCrossings.PushBack(neighbour);
            }
        }

        cv::Point pointWithMaxNeighbours = crossing;
        size_t maxNeighboursCnt = 0;
        for (const cv::Point& crossingPoint : localCrossings) {
            for (const cv::Point& neighbour : Get8Neighborhood(*image_, crossingPoint)) {
                const auto neighbours = Get8Neighborhood(*image_, neighbour);
                if (neighbours.Size() > maxNeighboursCnt) {
                    maxNeighboursCnt = neighbours.Size();
                    pointWithMaxNeighbours = neighbour;
                }
            }
        }

        crossings_.insert(pointWithMaxNeighbours);
    }

    PixelType GetPointType(const cv::Point& point) const {
        return static_cast<PixelType>(image_->at<uchar>(point));
    }
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
    cv::Mat output(image.size(), CV_8UC3);

    for (int row = 0; row < image.rows; ++row) {
        for (int col = 0; col < image.cols; ++col) {
            const uchar ptype = image.at<uchar>(row, col);
            assert(ptype < static_cast<uchar>(PixelType::MAX));

            output.at<cv::Vec3b>(row, col) = colorsPalette[ptype];
        }
    }

    return output;
}

int main(int argc, char* argv[]) {
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    std::filesystem::path input(argv[1]);
    std::filesystem::path output(argv[2]);
    output /= "sample5";
    if (!std::filesystem::exists(output)) {
        std::filesystem::create_directory(output);
    }

    std::filesystem::path imgPath = input / "fdeb5.png";
    assert(std::filesystem::exists(imgPath));

    LOG_INFO << "Read input image: " << imgPath;

    cv::Mat sourceImage;
    sourceImage = cv::imread(imgPath, cv::ImreadModes::IMREAD_GRAYSCALE);
    cv::Mat coloredImage;
    coloredImage = cv::imread(imgPath, cv::ImreadModes::IMREAD_COLOR);

    cv::Mat skeleton = GetReducedImage(sourceImage);
    DUMP(skeleton);

    cv::Mat vertexesMask = GetVertexesMask(coloredImage, {0, 0, 0}, 100.);

    cv::Mat typedSkel = MarkPixels(skeleton, vertexesMask, PixelType::VERTEX);

    LOG_INFO << "Run pixels classifier";
    PixelClassifier classifier(&typedSkel);
    classifier.Run();
    classifier.OptimizeCrossings();

    cv::Mat pixelsRecognition = ColorizeTypedPixels(typedSkel);
    DUMP(pixelsRecognition);

    return 0;
}

