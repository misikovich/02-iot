include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(mcu_default_library_list )

# Handle files with suffix s, for group default-XC16
if(mcu_default_default_XC16_FILE_TYPE_assemble)
add_library(mcu_default_default_XC16_assemble OBJECT ${mcu_default_default_XC16_FILE_TYPE_assemble})
    mcu_default_default_XC16_assemble_rule(mcu_default_default_XC16_assemble)
    list(APPEND mcu_default_library_list "$<TARGET_OBJECTS:mcu_default_default_XC16_assemble>")

endif()

# Handle files with suffix S, for group default-XC16
if(mcu_default_default_XC16_FILE_TYPE_assemblePreproc)
add_library(mcu_default_default_XC16_assemblePreproc OBJECT ${mcu_default_default_XC16_FILE_TYPE_assemblePreproc})
    mcu_default_default_XC16_assemblePreproc_rule(mcu_default_default_XC16_assemblePreproc)
    list(APPEND mcu_default_library_list "$<TARGET_OBJECTS:mcu_default_default_XC16_assemblePreproc>")

endif()

# Handle files with suffix c, for group default-XC16
if(mcu_default_default_XC16_FILE_TYPE_compile)
add_library(mcu_default_default_XC16_compile OBJECT ${mcu_default_default_XC16_FILE_TYPE_compile})
    mcu_default_default_XC16_compile_rule(mcu_default_default_XC16_compile)
    list(APPEND mcu_default_library_list "$<TARGET_OBJECTS:mcu_default_default_XC16_compile>")

endif()

# Handle files with suffix s, for group default-XC16
if(mcu_default_default_XC16_FILE_TYPE_dependentObject)
add_library(mcu_default_default_XC16_dependentObject OBJECT ${mcu_default_default_XC16_FILE_TYPE_dependentObject})
    mcu_default_default_XC16_dependentObject_rule(mcu_default_default_XC16_dependentObject)
    list(APPEND mcu_default_library_list "$<TARGET_OBJECTS:mcu_default_default_XC16_dependentObject>")

endif()

# Handle files with suffix elf, for group default-XC16
if(mcu_default_default_XC16_FILE_TYPE_bin2hex)
add_library(mcu_default_default_XC16_bin2hex OBJECT ${mcu_default_default_XC16_FILE_TYPE_bin2hex})
    mcu_default_default_XC16_bin2hex_rule(mcu_default_default_XC16_bin2hex)
    list(APPEND mcu_default_library_list "$<TARGET_OBJECTS:mcu_default_default_XC16_bin2hex>")

endif()


# Main target for this project
add_executable(mcu_default_image_igA9NkaQ ${mcu_default_library_list})

set_target_properties(mcu_default_image_igA9NkaQ PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${mcu_default_output_dir}")
target_link_libraries(mcu_default_image_igA9NkaQ PRIVATE ${mcu_default_default_XC16_FILE_TYPE_link})

# Add the link options from the rule file.
mcu_default_link_rule( mcu_default_image_igA9NkaQ)

# Call bin2hex function from the rule file
mcu_default_bin2hex_rule(mcu_default_image_igA9NkaQ)

