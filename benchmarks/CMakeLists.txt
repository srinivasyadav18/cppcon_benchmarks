set(subdirs
    adj_diff
    for_each_comp
    for_each_mem
    reverse
)

foreach(subdir ${subdirs})
    message("Adding benchmark ${subdir}")
    add_subdirectory(${subdir})
endforeach(subdir)