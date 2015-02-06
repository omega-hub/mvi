if(OMEGA_OS_WIN)
    add_executable(appmgr appmgr.cpp icon.rc)
else()
    add_executable(appmgr appmgr.cpp appmgr.h)
endif()

set_target_properties(appmgr PROPERTIES FOLDER apps)
target_link_libraries(appmgr omega)