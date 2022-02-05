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
    LOG_INFO << "Dump skel image to " << std::filesystem::absolute(filepath##image);

enum class PixelType {
    EMPTY,
    UNKNOWN,
    VERTEX,
    EDGE,
    CROSSING,
    PORTAL,

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
            cv::Point{pixel.x - 1, pixel.y - 1},
            cv::Point{pixel.x + 1, pixel.y},
            cv::Point{pixel.x + 1, pixel.y + 1},
            cv::Point{pixel.x, pixel.y + 1},
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
            LOG_DEBUG << "Found crossing point " << current;
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

private:
    std::unordered_set<cv::Point> used_;
    std::queue<GraphCrawler> crawlers_;
    cv::Mat* image_;

private:
    void TryAddCrawler() {
        for (int row = 0; row < image_->rows; ++row) {
            for (int col = 0; col < image_->cols; ++col) {
                cv::Point point{row, col};
                PixelType type = static_cast<PixelType>(image_->at<uchar>(point));
                if (type == PixelType::VERTEX) {
                    for (const cv::Point& neighbour : Get8Neighborhood(*image_, point)) {
                        PixelType ntype = static_cast<PixelType>(image_->at<uchar>(neighbour));
                        if (ntype == PixelType::UNKNOWN) {
                            LOG_DEBUG << "Add explicitly crawler with start point " << point;

                            crawlers_.push(GraphCrawler(image_, &used_, point));
                            return;
                        }
                    }
                }
            }
        }
    }

    void RunCrawler(GraphCrawler* crawler) {
        while (!crawler->Empty()) {
            cv::Point startPointForNextCrawler;
            if (!crawler->Step(&startPointForNextCrawler)) {
                LOG_DEBUG << "Add new crawler with start point " << startPointForNextCrawler;
                crawlers_.push(GraphCrawler(image_, &used_, startPointForNextCrawler));
            }
        }
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

    PixelClassifier classifier(&typedSkel);
    classifier.Run();

    cv::Mat coloredSkel = ColorizeTypedPixels(typedSkel);
    DUMP(coloredSkel);

    return 0;
}

