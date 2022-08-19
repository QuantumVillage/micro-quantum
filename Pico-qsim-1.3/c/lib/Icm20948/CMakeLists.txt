# Finds all source files in the current directory
# and save the name to the DIR_Icm20948_SRCS variable
aux_source_directory(. DIR_Icm20948_SRCS)

include_directories(../Config)

# Generate the link library
add_library(Icm20948 ${DIR_Icm20948_SRCS})
target_link_libraries(Icm20948 PUBLIC Config)