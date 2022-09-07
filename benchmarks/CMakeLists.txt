set(subdirs
    adj_diff
    for_each_comp
    for_each_mem
    for_each_mem_sr
    reverse
)

foreach(subdir ${subdirs})
    message("Adding benchmark ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)