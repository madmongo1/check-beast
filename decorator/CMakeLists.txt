add_executable(decorator main.cpp)
target_link_libraries(decorator PUBLIC Boost::system Boost::thread Threads::Threads)
