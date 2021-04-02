ENTRY(_start)

SECTIONS {
    . = 0;

    .text : {
        *(.init)
    }
}
