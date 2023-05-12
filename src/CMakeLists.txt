# FILE(GLOB MyCPPSources *.cpp)
# FILE(GLOB MyHSources *.h)

# set(PROJ "hycube_simulator")

# #find_package(nlohmann_json CONFIG REQUIRED)

# add_executable(${PROJ} ${MyCPPSources} ${MyHSources})
# add_library(${PROJ} STATIC ${MyCPPSources} ${MyHSources})

# # Use C++11 to compile our pass (i.e., supply -std=c++11).
# target_compile_features(${PROJ} PRIVATE cxx_range_for cxx_auto_type)
# #target_link_libraries(${PROJ} nlohmann_json::nlohmann_json)
FILE(GLOB MyCPPSources *.cpp)
FILE(GLOB MyHSources *.h)

set(PROJ_LIB "hycube_simulator_lib")
set(PROJ_EXE "hycube_simulator")

#find_package(nlohmann_json CONFIG REQUIRED)

add_library(${PROJ_LIB} STATIC ${MyCPPSources} ${MyHSources})
add_executable(${PROJ_EXE} ${MyCPPSources})

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(${PROJ_LIB} PRIVATE cxx_range_for cxx_auto_type)
target_compile_features(${PROJ_EXE} PRIVATE cxx_range_for cxx_auto_type)

# Uncomment the following line if nlohmann_json is needed
#target_link_libraries(${PROJ_LIB} nlohmann_json::nlohmann_json)
#target_link_libraries(${PROJ_EXE} nlohmann_json::nlohmann_json)

