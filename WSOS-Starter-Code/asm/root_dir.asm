; Root Directory Contents
rootDir:
rootdir_fileName           db ".       "
rootdir_extension          db "   "   
rootdir_attributes         db 0x10
rootdir_reserved           dw 0
rootdir_creationTime       dw 0
rootdir_creationDate       dw 0
rootdir_lastAccessDate     dw 0
rootdir_ignored            dw 0
rootdir_lastWriteTime      dw 0
rootdir_lastWriteDate      dw 0
rootdir_startingCluster    dw 0
rootdir_fileSize           dd 512
rootparent_fileName        db "..      "
rootparent_extension       db "   "   
rootparent_attributes      db 0x10
rootparent_reserved        dw 0
rootparent_creationTime    dw 0
rootparent_creationDate    dw 0
rootparent_lastAccessDate  dw 0
rootparent_ignored         dw 0
rootparent_lastWriteTime   dw 0
rootparent_lastWriteDate   dw 0
rootparent_startingCluster dw 0
rootparent_fileSize        dd 512
kernel_fileName            db "kernel  "
kernel_extension           db "bin"   
kernel_attributes          db 0
kernel_reserved            dw 0
kernel_creationTime        dw 0
kernel_creationDate        dw 0
kernel_lastAccessDate      dw 0
kernel_ignored             dw 0
kernel_lastWriteTime       dw 0
kernel_lastWriteDate       dw 0
kernel_startingCluster     dw 2
kernel_fileSize            dd 100*512
times (512 * 1) - ($ - rootDir) db 0