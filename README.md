# Edge Bundling Algorithms Evaluation (WIP)

_Inspired by_ [Optical Graph Recognition](https://www.researchgate.net/publication/232651643_Optical_Graph_Recognition)

### Steps

1. Reading image using OpenCV tools
2. Image preprocessing: _opening_ + _closing_
3. Image thinning ([A Sequential Thinning Algorithm For MultiDimensional Binary Patterns](https://arxiv.org/pdf/1710.03025.pdf)) 
4. Create extended matrix for future algorithms of optical graph recognition
5. Detect vertexes from source graph
6. Detect edges (the most complex step)
7. Evaluation some metrics: crossings count, edge bends, edge lengths, ambiguity

### Dependencies

1. [OpenCV](https://opencv.org/) — open source CV library
2. [plog](https://github.com/SergiusTheBest/plog/tree/a6b5e5189e6d4de15cbb7fddc7dedfb187c8bdc3) — lightweight logging library
3. [CLI11](https://github.com/CLIUtils/CLI11/tree/e1cef53f91044be9263e24eff1325eff94ef7f8b) — CLI parser lib

### Git submodules initialization
```
git submodule update --init --recursive
```

### Building

To build source code use cmake:
```
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make
```

### Usage

```
 ./build/main --help
Aesthetic metrics evaluation of bundling visualization techniques
Usage: ./build/main [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--input TEXT REQUIRED    Input image path
  -o,--output TEXT REQUIRED   Output results dir path
  --log-level TEXT [info]  (Env:LOG_LEVEL)
                              Log level: info, debug, none
  --dev-dir TEXT              Path dir to dev steps dump
  --vertex UINT               Run algo only for particular vertex
[Option Group: Algo params]
  Parameters of ogr algorithm
  Options:
    --curvature FLOAT [13]      Acceptable Steps curvature
    --stable-diff FLOAT [20]    Acceptable angle diff threshold between stable edge parts
    --state-diff FLOAT [25]     Acceptable diff angle between consecutive several steps
    --edges-union TEXT [union]  Union found edges strategy: union, intersection
```

### Results

1. [sample1](./samples/1) => [result1](./results/1)
2. [sample2](./samples/2) => [result2](./results/2)
3. [sample3](./samples/3) => [result3](./results/3)

Below you can see visualization of algorithm implementation for finding edges from one particular vertex:

![](./results/algo_vis.gif)
