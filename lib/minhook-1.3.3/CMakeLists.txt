project(minhook)
add_library(minhook STATIC
    src/hook.c
    src/trampoline.c
    src/buffer.c
    src/hde/hde32.c
    src/hde/hde64.c
)

