# Finds all source files in the current directory
# and save the name to the DIR_Infrared_SRCS variable
aux_source_directory(. DIR_Infrared_SRCS)

include_directories(../Config)

# Generate the link library
add_library(Infrared ${DIR_Infrared_SRCS})
target_link_libraries(Infrared PUBLIC Config)