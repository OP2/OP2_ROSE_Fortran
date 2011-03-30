include_directories(${OP2_SOURCE_DIR} ${OP2_SOURCE_DIR}/op2 ${OP2_INCLUDE_DIRECTORIES})
add_executable(jac1_seq jac.c)
target_link_libraries(jac1_seq op2 m)
