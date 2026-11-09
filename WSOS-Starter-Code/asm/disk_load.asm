disk_load_first_half:
    pusha
    xor ax, ax
    mov es, ax

    ; Track 1: Cyl=0, Head=1, Sector=7, 12 sectors
    mov ah, 0x02
    mov al, 12
    mov dl, [boot_drive]
    mov dh, 1
    mov ch, 0
    mov cl, 7
    int 0x13
    jc .error
    cmp al, 12
    jne .error

    add bx, 0x1800

    ; Track 2: Cyl=1, Head=0, Sector=1, 18 sectors
    mov ah, 0x02
    mov al, 18
    mov dl, [boot_drive]
    mov dh, 0
    mov ch, 1
    mov cl, 1
    int 0x13
    jc .error
    cmp al, 18
    jne .error

    add bx, 0x2400

    ; Track 3: Cyl=1, Head=1, Sector=1, 18 sectors
    mov ah, 0x02
    mov al, 18
    mov dl, [boot_drive]
    mov dh, 1
    mov ch, 1
    mov cl, 1
    int 0x13
    jc .error
    cmp al, 18
    jne .error

    add bx, 0x2400

    ; Track 4: Cyl=2, Head=0, Sector=1, 6 sectors
    mov ah, 0x02
    mov al, 6
    mov dl, [boot_drive]
    mov dh, 0
    mov ch, 2
    mov cl, 1
    int 0x13
    jc .error
    cmp al, 6
    jne .error

    popa
    ret

.error:
    jmp $