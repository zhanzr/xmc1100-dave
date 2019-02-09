# xmc1100-dave

Fault and Fault Handler test.

Faults are a subset of exceptions, see Exception model on page 2-19. All faults result in
the HardFault exception being taken or cause lockup if they occur in the NMI or
HardFault handler. The faults are:
• execution of an SVC instruction at a priority equal or higher than SVCall
• execution of a BKPT instruction without a debugger attached
• a system-generated bus error on a load or store
• execution of an instruction from an XN memory address
• execution of an instruction from a location for which the system generates a bus fault
• a system-generated bus error on a vector fetch
• execution of an Undefined instruction
• execution of an instruction when not in Thumb-State as a result of the T-bit being 
  previously cleared to 0
• an attempted load or store to an unaligned address.

Note
Only Reset and NMI can preempt the fixed priority HardFault handler. A HardFault can
preempt any exception other than Reset, NMI, or another HardFault.
