# Finds all source files in the current directory
# and save the name to the DIR_examples_SRCS variable
aux_source_directory(. DIR_examples_SRCS)


include_directories(../lib/Config)
include_directories(../lib/GUI)
include_directories(../lib/Infrared)
include_directories(../lib/LCD)
include_directories(../lib/Icm20948)
include_directories(.)

# Generate the link library
add_library(qsim ${DIR_examples_SRCS})
target_link_libraries(qsim PUBLIC Config LCD Infrared Icm20948)