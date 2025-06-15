target extended-remote localhost:3333

# monitor-reset-init
document mri
    monitor reset init
end
define mri
    monitor reset init
end

# compile-link-load
document comp
    compile, link and load
end
define comp
    make
    target extended-remote localhost:3333
    load
    mri
end
