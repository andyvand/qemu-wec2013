    INCLUDE kxarm.h
    TEXTAREA CONFIG=32

    EXPORT _wfi
    LEAF_ENTRY _wfi

    mrs r0, cpsr
    cpsid i
    wfi
    msr cpsr, r0
    bx lr
    
    ENDP

    END
