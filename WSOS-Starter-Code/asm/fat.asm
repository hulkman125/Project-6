; File Allocation Table (First Copy)
fatCopy0:
times 2                 dw 0
kernelStartingCluster0  dw 3
                        %assign i 4
                        %rep 54-4+1
                            dw i
                            %assign i i+1
                        %endrep
                        dw 0xFFFF
times (512 * 11) - ($ - fatCopy0) db 0

; NOTE: Make sure fatCopy0 and fatCopy1 have identical contents!

; File Allocation Table (Second Copy)
fatCopy1:
times 2                 dw 0
kernelStartingCluster1  dw 3
                        %assign i 4
                        %rep 54-4+1
                            dw i
                            %assign i i+1
                        %endrep
                        dw 0xFFFF
times (512 * 11) - ($ - fatCopy1) db 0