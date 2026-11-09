[org 0x7C00]

kernel_offset equ 0x1000

jmp short _start
nop

; FAT16 Bios Parameter Block
oem						db "MSWIN4.1"
bytesPerSector			dw 512
sectorsPerCluster		db 1
reservedSectors			dw 1
fatCount				db 2
rootDirectoryEntries	dw 16
sectorCount				dw 2880
mediaDescriptorType		db 0b11111000
sectorsPerFat			dw 11
sectorsPerTrack			dw 18
headCount				dw 2
hiddenSectorCount		db 0
largeSectorCount		dd 0

; Extended Boot Record
driveNumber				db 0
reserved				db 0
signature				db 29h
volumeID				db 00h, 00h, 00h, 00h
volumeLabel				db "BOOT FLOPPY"
systemID				db "FAT16   "

_start:
    mov [boot_drive], dl
    mov bp, 0x8000
    mov sp, bp
    call load_kernel
    call switch
    jmp $

%include "./asm/disk_load.asm"
%include "./asm/gdt.asm"
%include "./asm/switch.asm"

[bits 16]
load_kernel:
    ; Reset disk controller (required by SeaBIOS)
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    ; Load kernel
    mov bx, kernel_offset 
    mov dh, 54
    call disk_load_first_half        

    ; Disable blinking cursor
    mov ah, 0x01
    mov ch, 0x26
    mov cl, 0x07
    int 0x10
    
    ret

[bits 32]
pmode:
	call kernel_offset
	jmp $

boot_drive db 0
times 510 - ($ - $$) db 0
db 0x55, 0xaa
