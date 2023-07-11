# Donde toolkits

It's a group of AI/CV related toolkits, including face feature extraction, feature searching and clustering service. Built with openvino and faiss[todo] [wip].

Currently developed with macOS, llvm-clang


## Features

- Openvino inference
- Full pipeline detect/landmarks/align/extract
- Feature search service with simple store
- Feature searching with faiss
- Feature clustering with faiss
- Master/Workers architecture, Poco messaging
- Conan to manage dependencies
- Tests coverage

## Dependencies

openvino, you need to install(download from openvino website) it first. other depencencies are managed by `conan/conanfile.txt`.

### Models

| name                          | type      | desc                 | urldesc                                                                                                                  |
|-------------------------------|-----------|----------------------|--------------------------------------------------------------------------------------------------------------------------|
| face-detection-adas-0001      | detect    | detect face          | https://github.com/openvinotoolkit/open_model_zoo/blob/master/models/intel/face-detection-adas-0001/README.md  |
| facial-landmarks-35-adas-0002 | landmarks | 70 points landmarks  | https://github.com/openvinotoolkit/open_model_zoo/blob/master/models/intel/facial-landmarks-35-adas-0002/README.md       |
| Sphereface.xml                | feature   | extract face feature | https://github.com/openvinotoolkit/open_model_zoo/blob/master/models/public/Sphereface/README.md                         |



## Project layout


* `include` contains main api headers, define what `DondeToolkits` is. include common abstract definitions for feature extract, search, video process etc.

* `src` contains implementations for api headers, and contains related header definition for those implementations.

* `test` contains tests, they test the `DondeToolkits` library. tests depend to library.


## Usage


### Build and run

#### Install openvino distribution

https://www.intel.com/content/www/us/en/developer/tools/openvino-toolkit/download.html

##### For mac m1
for mac m1 users, please refer to `README_mac_m1.md` for instructions on how to install openvino with an arm device.

```
#double check your openvino install location, modify CMakeLists.txt if needed

set(OpenVINO_DIR "/opt/intel/openvino_2022/runtime/cmake")
find_package(OpenVINO REQUIRED COMPONENTS Runtime)
```

activate openvino env

```
python3.8 -m venv venv3.8
source venv3.8/bin/activate
source /opt/intel/openvino_2022/setupvars.sh
```


#### Install llvm clang (deprecated)

The project can be built use llvm clang (version 14), not use apple clang

```
brew install llvm

/usr/local/opt/llvm/bin/clang --version
Homebrew clang version 14.0.6

#explict set CC and CXX, for sure
export CC=/usr/local/opt/llvm/bin/clang ; export CXX=/usr/local/opt/llvm/bin/clang++
```

#### Build conan dependency packages

```bash
mkdir -p build
conan install --build=missing --profile conan/conanprofile.m1  -if build ./conan
```

#### Build toolkits with testings

```bash

cmake -B build -DDondeToolkits_ENABLE_UNIT_TESTING=true
cmake --build build -- -j 8

# run tests
./build/bin/DondeToolkitsTests
```

## Mac M1

[README mac m1](./README_mac_m1.md)
