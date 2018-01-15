add_executable(GameTest
	TestMain.cpp
	GameTest.cpp
)

target_link_libraries(GameTest
	gtest
	Hex
	${LIBRARY_LIST}
)

ADD_TEST(GameTest GameTest ${PROJECT_SOURCE_DIR})