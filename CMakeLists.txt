request_dependency(porthole)

# Set the module name here
set(MODULE_NAME mvi)

# Set module name and source files here
add_library(${MODULE_NAME} MODULE
	mvi.cpp
	AppController.cpp
	AppController.h
	Workspace.cpp
	Workspace.h
	WorkspaceLayout.cpp
	WorkspaceLayout.h
	WorkspaceLibrary.cpp
	WorkspaceLibrary.h
	)

# Set the module library dependencies here
target_link_libraries(${MODULE_NAME}
	omega
	omegaToolkit)
    
add_subdirectory(appmgr)

#------------------------------------------------------------------------------
# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()
