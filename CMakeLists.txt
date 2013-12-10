# Set the module name here
SET(MODULE_NAME mvi)

# Set module name and source files here
add_library(${MODULE_NAME} MODULE
	mvi.cpp
	WandInputFilter.cpp
	WandInputFilter.h
	WandPointerSwitcher.cpp
	WandPointerSwitcher.h
	)

# Set the module library dependencies here
target_link_libraries(${MODULE_NAME}
	omega
	omegaToolkit)

#------------------------------------------------------------------------------
# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()
