set(subdirs
    adj_diff
    for_each_comp
    for_each_mem
    for_each_mem_sr
    for_each_mem_sr_full
    for_each_comp_sr
    reverse
)

foreach(subdir ${subdirs})
    message("Adding benchmark ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)