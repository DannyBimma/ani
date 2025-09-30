# ani executable

add_executable(ani
	main.c
	util/log.c
	util/version.c
)

target_include_directories(ani PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ani PRIVATE
	CURL::libcurl
	yyjson
)