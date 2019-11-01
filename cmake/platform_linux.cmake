

# function(init_vars_linux)
#     set(WHOLE_LIBRARY_FLAG "-Wl,--whole-archive" CACHE INTERNAL "")
#     set(NO_WHOLE_LIBRARY_FLAG "-Wl,--no-whole-archive" CACHE INTERNAL "")
# endfunction()


function(init_vars_linux)
    if(UNIX AND NOT APPLE)
        set(WHOLE_LIBRARY_FLAG "-Wl,--whole-archive" CACHE INTERNAL "")
        set(NO_WHOLE_LIBRARY_FLAG "-Wl,--no-whole-archive" CACHE INTERNAL "")
    else()
        set(WHOLE_LIBRARY_FLAG "-Wl" CACHE INTERNAL "")
        set(NO_WHOLE_LIBRARY_FLAG "-Wl" CACHE INTERNAL "")
    endif()
endfunction()