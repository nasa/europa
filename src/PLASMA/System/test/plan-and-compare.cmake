execute_process(COMMAND ${exec_plan} ${model} ${configFile} ${language}
  OUTPUT_FILE ${output_file}
  ERROR_FILE ${output_file}
  RESULT_VARIABLE result)
if(NOT ${result} EQUAL 0)
  message(FATAL_ERROR ${result})
endif(NOT ${result} EQUAL 0)

if(plan_compare AND (${compare} STREQUAL "true"))
  execute_process(COMMAND ${plan_compare} ${gold_file} ${output_file}
    OUTPUT_FILE ${diff_file}
    RESULT_VARIABLE diff_result)
  if(NOT ${diff_result} EQUAL 0)
    message(FATAL_ERROR ${diff_result})
  endif(NOT ${diff_result} EQUAL 0)
endif(plan_compare AND (${compare} STREQUAL "true"))