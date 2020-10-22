message("-- Linking Assimp")
add_library(assimp SHARED "./assimp-vc142-mtd.lib")
set_target_properties(assimp PROPERTIES LINKER_LANGUAGE C)
