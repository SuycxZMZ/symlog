set(TEST_LIBRARIES symlog pthread)

# 添加测试
enable_testing()

# 定义测试列表
set(TESTS
    test_asynclog
)

# 循环添加测试
foreach(TEST ${TESTS})
    add_executable(${TEST} ${TEST}.cc)
    target_link_libraries(${TEST} ${TEST_LIBRARIES})
    add_test(NAME ${TEST} COMMAND ${TEST})
endforeach()