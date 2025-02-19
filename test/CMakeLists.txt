#---------------------------------------------------------------------------#
# Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
#
# Distributed under the Boost Software License, Version 1.0
# See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt
#---------------------------------------------------------------------------#

include(CMTest)

cm_find_package(Boost REQUIRED COMPONENTS unit_test_framework)

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}

                       ${CMAKE_WORKSPACE_NAME}::algebra
                       ${CMAKE_WORKSPACE_NAME}::math
                       ${CMAKE_WORKSPACE_NAME}::multiprecision
                       ${CMAKE_WORKSPACE_NAME}::random

                       marshalling::core
                       marshalling::crypto3_multiprecision
                       marshalling::crypto3_algebra
                       marshalling::crypto3_zk

                       ${Boost_LIBRARIES})

option(ZK_PLACEHOLDER_PROFILING_ENABLED "Build with placeholder profiling" FALSE)

if(ZK_PLACEHOLDER_PROFILING)
    add_definitions(-DZK_PLACEHOLDER_PROFILING_ENABLED)
endif()

macro(define_zk_test test)
    get_filename_component(name ${test} NAME)
    string(REPLACE "/" "_" full_test_name ${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}_${test}_test)
    cm_test(NAME ${full_test_name} SOURCES ${test}.cpp)

    target_include_directories(${full_test_name} PRIVATE
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                               "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"

                               ${Boost_INCLUDE_DIRS})

    set_target_properties(${full_test_name} PROPERTIES CXX_STANDARD 17)

    get_target_property(target_type Boost::unit_test_framework TYPE)
    if(target_type STREQUAL "SHARED_LIB")
        target_compile_definitions(${full_test_name} PRIVATE BOOST_TEST_DYN_LINK)
    elseif(target_type STREQUAL "STATIC_LIB")

    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${full_test_name} PRIVATE "-fconstexpr-steps=2147483647")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${full_test_name} PRIVATE "-fconstexpr-ops-limit=4294967295")
    endif()

endmacro()

set(TESTS_NAMES
    "commitment/lpc"
    "commitment/fri"
    "commitment/fold_polynomial"
    "commitment/lpc_performance"
    "commitment/pedersen"
    "commitment/proof_of_knowledge"
    "commitment/powers_of_tau"
    "commitment/r1cs_gg_ppzksnark_mpc"
    "commitment/type_traits"
    "commitment/kimchi_pedersen"

    "math/expression"

    "routing_algorithms/test_routing_algorithms"

#    "relations/numeric/qap"
#    "relations/numeric/sap"
#    "relations/numeric/ssp"

    "systems/plonk/pickles/pickles"
    "systems/plonk/pickles/kimchi"
    "systems/plonk/pickles/oracles"
    "systems/plonk/pickles/to_field"
    "systems/plonk/pickles/to_group"
    
    "systems/plonk/placeholder/placeholder"

#    "systems/pcd/r1cs_pcd/r1cs_mp_ppzkpcd/r1cs_mp_ppzkpcd"
#    "systems/pcd/r1cs_pcd/r1cs_sp_ppzkpcd/r1cs_sp_ppzkpcd"

#    "systems/ppzksnark/bacs_ppzksnark/bacs_ppzksnark"
    "systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark"
    "systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark_marshalling"
    "systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark_tvm_marshalling"
    "systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark"
#    "systems/ppzksnark/r1cs_se_ppzksnark/r1cs_se_ppzksnark"
#    "systems/ppzksnark/ram_ppzksnark/ram_ppzksnark"
#    "systems/ppzksnark/tbcs_ppzksnark/tbcs_ppzksnark"
#    "systems/ppzksnark/uscs_ppzksnark/uscs_ppzksnark"
    "systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark_aggregation_conformity"

    "transcript/transcript"
    "transcript/kimchi_transcript"

    "systems/plonk/plonk_constraint")

foreach(TEST_NAME ${TESTS_NAMES})
    define_zk_test(${TEST_NAME})
endforeach()

string(CONCAT TEST_DATA ${CMAKE_CURRENT_SOURCE_DIR} "/systems/plonk/pickles/data/kimchi")
target_compile_definitions(crypto3_zk_systems_plonk_pickles_kimchi_test PRIVATE TEST_DATA="${TEST_DATA}")
