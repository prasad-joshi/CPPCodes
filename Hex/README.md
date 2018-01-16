# Mandatory Packages
1. `cmake`
2. `gtest`
3. `glog`
4. `gflags`

# Instructions to compile code
1. Goto top level source directory
	- `cd ${TOP_LEVEL_DIR}`

2. Create `build` directory
	- `mkdir ${TOP_LEVEL_DIR}/build`

3. Switch to `build` directory
	- `cd ${TOP_LEVEL_DIR}/build`

4. Run `cmake ..`

5. Run `make`

6. Game executable can be found in `${TOP_LEVEL_DIR}/build/src/HexGame`

# Running Game

1. Print help message `${TOP_LEVEL_DIR}/build/src/HexGame --help`

2. Setting board size `${TOP_LEVEL_DIR}/build/src/HexGame -nrows 11`. Default value of `nrows` is `7`.