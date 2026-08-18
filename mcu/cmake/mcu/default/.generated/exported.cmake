set(DEPENDENT_MP_BIN2HEXmcu_default_igA9NkaQ "/opt/microchip/xc16/v2.10/bin/xc16-bin2hex")
set(DEPENDENT_DEPENDENT_TARGET_ELFmcu_default_igA9NkaQ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/mcu/default.elf)
set(DEPENDENT_TARGET_DIRmcu_default_igA9NkaQ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/mcu)
set(DEPENDENT_BYPRODUCTSmcu_default_igA9NkaQ ${DEPENDENT_TARGET_DIRmcu_default_igA9NkaQ}/${sourceFileNamemcu_default_igA9NkaQ}.s)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRmcu_default_igA9NkaQ}/${sourceFileNamemcu_default_igA9NkaQ}.s
    COMMAND ${DEPENDENT_MP_BIN2HEXmcu_default_igA9NkaQ} ${DEPENDENT_DEPENDENT_TARGET_ELFmcu_default_igA9NkaQ} --image ${sourceFileNamemcu_default_igA9NkaQ} ${addressmcu_default_igA9NkaQ} ${modemcu_default_igA9NkaQ} -mdfp=/home/ss1/.mchp_packs/Microchip/dsPIC33E-GM-GP-MC-GU-MU_DFP/1.7.401/xc16 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRmcu_default_igA9NkaQ}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFmcu_default_igA9NkaQ})
add_custom_target(
    dependent_produced_source_artifactmcu_default_igA9NkaQ 
    DEPENDS ${DEPENDENT_TARGET_DIRmcu_default_igA9NkaQ}/${sourceFileNamemcu_default_igA9NkaQ}.s
    )
