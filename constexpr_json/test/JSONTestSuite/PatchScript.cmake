if (NOT EXISTS "${INPUT_FILE}")
  message(FATAL_ERROR "Couldn't find script to patch: ${INPUT_FILE}")
endif()
file(READ "${INPUT_FILE}" SCRIPT_CONTENT)

string(REPLACE
# Match String
",
   \"C++ nlohmann JSON"
# Replace String
",
   \"C++ constexpr json\":
       {
           \"url\":\"https://github.com/suluke/monobo/tree/master/constexpr_json\",
           \"commands\":[\"${CMAKE_CURRENT_BINARY_DIR}/jsontestsuite_driver\"]
       },
   \"C++ nlohmann JSON"
# Result Variable
  SCRIPT_CONTENT_PATCHED
# Content String
  "${SCRIPT_CONTENT}"
)
file(WRITE "${OUTPUT_FILE}" "${SCRIPT_CONTENT_PATCHED}")
