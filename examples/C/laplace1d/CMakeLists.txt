include_directories(${OP2_SOURCE_DIR} ${OP2_SOURCE_DIR}/op2 ${OP2_INCLUDE_DIRECTORIES})
add_executable(laplace1d_seq laplace.c)
target_link_libraries(laplace1d_seq op2 m)
ADD_CUSTOM_TARGET( laplace1d_example DEPENDS laplace1d_seq COMMENT "Build laplace1d examples" )
