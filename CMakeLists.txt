cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(ClientServerMessages VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

find_package(Qt5 COMPONENTS REQUIRED 
	Core
	Gui
	Widgets
	network
	)
	
set(SERVER_SRC
	src/server.cpp
	src/Connections.cpp
	)

qt5_wrap_cpp(SERVER_SRC
	include/server.h
	)

qt5_wrap_ui(server_ui
	${CMAKE_CURRENT_SOURCE_DIR}/ui/serverdialog.ui
	)

add_executable(Server
	include/server.h
	${SERVER_SRC}
	${server_ui}
	src/ServerMain.cpp
	)
	
target_link_libraries(Server
	PRIVATE
		Qt5::Core
		Qt5::Gui
		Qt5::Widgets
		Qt5::Network
	)
	
target_include_directories(Server
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
		
		${CMAKE_CURRENT_BINARY_DIR}
	)
	
# TODO: add copying qwindows.dll
if(WIN32)
add_custom_target(COPY_QT_DLLS ALL
	COMMENT "Deploying Qt Dlls..."
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:Server>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:Server>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:Server>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Network> $<TARGET_FILE_DIR:Server>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:Server>
	)
endif()