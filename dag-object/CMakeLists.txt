# CMakeList.txt : CMake project for dag-object, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (dag-object 
	"dag-object.cpp" "dag-object.h"
)

target_link_libraries(dag-object PRIVATE dag)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET dag-object PROPERTY CXX_STANDARD 20)
endif()
