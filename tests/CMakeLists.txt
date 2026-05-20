add_executable(unit_tests 
    test_main.cpp 
    test_cloudapi.cpp
    test_localfolder.cpp
    test_syncmanager.cpp
    test_fileinfo.cpp
)

target_link_libraries(unit_tests 
    PRIVATE 
    cloudsync_lib 
    gtest_main
    gmock
)

include(GoogleTest)
gtest_discover_tests(unit_tests)