# ani executable

add_executable(ani
	main.c
	util/log.c
	util/version.c
	util/str.c
	util/time.c
	util/fs.c
	net/http.c
	json/json_wrap.c
	models/model.c
	providers/jikan.c
	providers/anilist.c
	providers/mangadex.c
	cli/args.c
	cli/output.c
)

target_include_directories(ani PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(ani PRIVATE
	CURL::libcurl
	yyjson
)