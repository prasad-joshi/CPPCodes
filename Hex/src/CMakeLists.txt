include_directories(include)

set(LIBRARY_LIST
	gflags
	glog
	pthread
)

add_library(Hex SHARED
	Hex.cpp
)

add_subdirectory(tests)

add_executable(HexGame
	HexGame.cpp
)

target_link_libraries(HexGame
	${LIBRARY_LIST}
	Hex
)