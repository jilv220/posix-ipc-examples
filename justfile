cc := 'gcc'
libs := '-lrt -lpthread'

default: build-all

get-srcs:
    @find *.c

build src:
    #!/usr/bin/env bash
    s={{src}}
    {{cc}} {{src}} -o "${s%.*}.out" {{libs}}

run src *args: (build src)
    #!/usr/bin/env bash 
    s={{src}}
    ./"${s%.*}.out" {{args}}

clean-all:
    #!/usr/bin/env bash
    rm *.o
    rm *.out

build-all: 
    #!/usr/bin/env bash
    for src in $(just get-srcs); do $(just build $src); done

