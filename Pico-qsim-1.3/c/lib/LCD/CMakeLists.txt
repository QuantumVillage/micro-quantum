# Finds all source files in the current directory
# and save the name to the DIR_ePaper_SRCS variable
aux_source_directory(. DIR_LCD_SRCS)

include_directories(../Config)

# Generate the link library
add_library(LCD ${DIR_LCD_SRCS})
target_link_libraries(LCD PUBLIC Config)