include_directories(${OP2_SOURCE_DIR} ${OP2_SOURCE_DIR}/op2 ${OP2_INCLUDE_DIRECTORIES})
add_executable(airfoil_seq airfoil.c)
target_link_libraries(airfoil_seq op2 m)
ADD_CUSTOM_TARGET( airfoil_example DEPENDS airfoil_seq COMMENT "Build airfoil examples" )
