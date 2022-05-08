# Edge Bundling Algorithms Evaluation 

_Inspired by_ [Optical Graph Recognition](https://www.researchgate.net/publication/232651643_Optical_Graph_Recognition)

### Steps

1. Reading image using OpenCV tools
2. Image preprocessing: _opening_ + _closing_
3. Image thinning ([A Sequential Thinning Algorithm For MultiDimensional Binary Patterns](https://arxiv.org/pdf/1710.03025.pdf)) 
4. Create extended matrix for future algorithms of optical graph recognition
5. Detect vertexes from source graph
6. Detect edges (the most complex step)
7. Evaluation some metrics: crossings count, edge bends, edge lengths, ambiguity, etc...

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

### Quick usage start 

Create `build` directory using previous step and run cmake.

```
make sample1            # run algorithm for sample1
make sample1-report     # run only for report with sample1 (quick)
make all                # run algorithm for all samples
```

### Usage

```
 ./build/main --help
Aesthetic metrics evaluation of bundling visualization techniques
Usage: ./build/main [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i,--input TEXT REQUIRED    Input images dir path
  -b,--baseline-name TEXT [baseline] 
                              Input image baseline
  -o,--output TEXT REQUIRED   Output results dir path
  --log-level TEXT [info]  (Env:LOG_LEVEL)
                              Log level: info, debug, none
  --dev-dir TEXT              Path dir to dev steps dump
  --vertex UINT               Run algo only for particular vertex
  --filter TEXT               Run algo only for particular image file
  --dump-intermediate [0]     
  --only-report [0]           Show only report of algo evaluation
[Option Group: Algo params]
  Parameters of ogr algorithm
  Options:
    --curvature FLOAT [20]      Acceptable steps curvature
    --stable-diff FLOAT [15]    Acceptable angle diff threshold between stable edge parts
    --state-diff FLOAT [40]     Acceptable diff angle between consecutive several steps
    --baseline-curvature FLOAT [13] 
                                Acceptable steps curvature (baseline)
    --baseline-stable-diff FLOAT [9] 
                                Acceptable angle diff threshold between stable edge parts (baseline)
    --baseline-state-diff FLOAT [30] 
                                Acceptable diff angle between consecutive several steps (baseline)
    --edges-union TEXT [union]  Union found edges strategy: union, intersection
```

### Algo evaluation results

1. [sample1](./samples/1) => [result1](./results/1/report.txt)
2. [sample2](./samples/2) => [result2](./results/2/report.txt)
3. [sample3](./samples/3) => [result3](./results/3/report.txt)
4. [sample4](./samples/4) => [result4](./results/4/report.txt)
5. [sample5](./samples/5) => [result5](./results/5/report.txt)

Below you can see visualization of algorithm implementation for finding edges from one particular vertex:

![](./results/algo_vis.gif)
![](./results/algo_vis2.gif)
