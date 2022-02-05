# Edge Bundling Algorithms Evaluation (WIP)

## Part 1: Evaluation Aesthetic Graph Metrics

1. [Metrics for Graph Drawing Aesthetics](https://www.researchgate.net/publication/222546202_Metrics_for_Graph_Drawing_Aesthetics)
2. [Optical Graph Recognition](https://www.researchgate.net/publication/232651643_Optical_Graph_Recognition)

### Work Steps

1. Image skeletonization ([A Sequential Thinning Algorithm For MultiDimensional Binary Patterns](https://arxiv.org/pdf/1710.03025.pdf))
See [sample](images/sample1.jpeg) and [thinning image](./cv_results/sample_skel.jpeg). 
   
2. Pixels classification [cv results](./cv_results). Vertex pixels classified by color from source image.
Pixel type to color mapping:
```c++
const cv::Vec3b colorsPalette[] = {
        // {Blue, Green, Red}
        cv::Vec3b{0, 0, 0},         // EMPTY
        cv::Vec3b{255, 255, 255},   // UNKNOWN
        cv::Vec3b{255, 0, 0},       // VERTEX
        cv::Vec3b{0, 255, 0},       // EDGE
        cv::Vec3b{0, 0, 255}        // CROSSING
};
```

## Usage

### Dependencies

1. [OpenCV](https://opencv.org/) –– open source CV library
2. [plog](https://github.com/SergiusTheBest/plog) –– lightweight logging library

### Git submodules initialization
```
git submodule update --init --recursive
```

### Building
To build source code use cmake:
```
mkdir build
cd build
cmake ..
make 
```

### Running
```
cd build
./main
```

