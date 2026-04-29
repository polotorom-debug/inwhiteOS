.global switch_page_directory
switch_page_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

.global enable_page_paging
enable_page_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret
