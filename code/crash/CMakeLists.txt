casa_add_executable (NONE CrashReportPoster apps/reporter/CrashReportPoster.cc apps/reporter/http_upload.cc)
target_link_libraries (CrashReportPoster -ldl)

