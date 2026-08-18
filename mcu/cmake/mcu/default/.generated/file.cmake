# The following variables contains the files used by the different stages of the build process.
set(mcu_default_default_XC16_FILE_TYPE_assemble "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/where_was_i.s")
set_source_files_properties(${mcu_default_default_XC16_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${mcu_default_default_XC16_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(mcu_default_default_XC16_FILE_TYPE_assemblePreproc)
set_source_files_properties(${mcu_default_default_XC16_FILE_TYPE_assemblePreproc} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${mcu_default_default_XC16_FILE_TYPE_assemblePreproc})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(mcu_default_default_XC16_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../blinker.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../hid.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../led.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../log.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/ext_int.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/i2c1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/interrupt_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/mcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/oc1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/oc2.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/oc3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/oc4.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/pin_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/reset.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/traps.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/uart1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/uart2.c")
set_source_files_properties(${mcu_default_default_XC16_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(mcu_default_default_XC16_FILE_TYPE_link)
set(mcu_default_default_XC16_FILE_TYPE_bin2hex)
set(mcu_default_image_name "default.elf")
set(mcu_default_image_base_name "default")

# The output directory of the final image.
set(mcu_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/mcu")

# The full path to the final image.
set(mcu_default_full_path_to_image ${mcu_default_output_dir}/${mcu_default_image_name})
