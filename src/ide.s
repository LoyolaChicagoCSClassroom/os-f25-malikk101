.intel_syntax noprefix
.section .text
.code32
.globl ata_lba_read
ata_lba_read:
    push ebp
    mov  ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    push edi

    mov  edx, 0x03F6        # Digital output register
    mov  al,  2             # Disable interrupts
    out  dx,  al

    mov  eax, [ebp+8]       # LBA
    mov  edi, [ebp+12]      # buffer
    mov  ecx, [ebp+16]      # sector count
    and  eax, 0x0FFFFFFF
    mov  ebx, eax           # save LBA

    mov  edx, 0x01F6        # drive/head + LBA[27:24]
    shr  eax, 24
    or   al,  0b11100000    # LBA mode, drive 0
    out  dx,  al

    mov  edx, 0x01F2        # sector count
    mov  al,  cl
    out  dx,  al

    mov  edx, 0x01F3        # LBA[7:0]
    mov  eax, ebx
    out  dx,  al

    mov  edx, 0x01F4        # LBA[15:8]
    mov  eax, ebx
    shr  eax, 8
    out  dx,  al

    mov  edx, 0x01F5        # LBA[23:16]
    mov  eax, ebx
    shr  eax, 16
    out  dx,  al

    mov  edx, 0x01F7        # command
    mov  al,  0x20          # READ SECTORS (with retry)
    out  dx,  al

    # brief initial poll (ignore ERR for first few reads)
    mov  ecx, 4
.lp1:
    in   al,  dx
    test al,  0x80          # BSY?
    jne  .retry
    test al,  0x08          # DRQ?
    jne  .data_rdy
.retry:
    dec  ecx
    jg   .lp1

.pior_l:
    in   al,  dx
    test al,  0x80          # BSY?
    jne  .pior_l
    test al,  0x21          # ERR or DF?
    jne  .fail

.data_rdy:
    mov  edx, 0x01F0        # data port
    mov  cx,  256           # 256 words = 512 bytes
    cld
    rep  insw               # read one sector -> [EDI]

    mov  edx, 0x01F7        # 400ns delay via status reads
    in   al,  dx
    in   al,  dx
    in   al,  dx
    in   al,  dx

    sub  dword ptr [ebp+16], 1   # sectors remaining
    jne  .pior_l

    xor  eax, eax            # success
    jmp  .done

.fail:
    mov  eax, -1

.done:
    pop  edi
    pop  edx
    pop  ecx
    pop  ebx
    leave
    ret

.section .note.GNU-stack,"",@progbits
