add_executable(json_schema_dbgprint dbgprint.cc)
target_link_libraries(json_schema_dbgprint PRIVATE cli_args)
target_link_libraries(json_schema_dbgprint PRIVATE json_schema)
# FIXME needs platform-specific coding
target_link_libraries(json_schema_dbgprint PRIVATE stdc++fs)
