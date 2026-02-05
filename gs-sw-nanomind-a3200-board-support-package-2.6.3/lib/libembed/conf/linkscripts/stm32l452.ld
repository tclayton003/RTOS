
/* Entry */
ENTRY(Reset_Handler)

/* Minimum heap and stack size */
_Min_Heap_Size = 0x200;
_Min_Stack_Size = 0x400;

_FLASH_SIZE = 512K;
_FLASH_DATA_SIZE = (DEFINED(GS_FLASH_DATA_SIZE) ? GS_FLASH_DATA_SIZE : 0K);
_FLASH_CODE_SIZE = _FLASH_SIZE - _FLASH_DATA_SIZE;

MEMORY
{
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 160K
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = _FLASH_SIZE - _FLASH_DATA_SIZE
  FLASH_DATA (rw) : ORIGIN = 0x08000000 + _FLASH_SIZE - _FLASH_DATA_SIZE, LENGTH = _FLASH_DATA_SIZE
}

/* Highest address of stack (user mode) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* End of heap */
_heap_end = _estack - _Min_Stack_Size - 1;

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(8);
    KEEP(*(.isr_vector))
    . = ALIGN(8);
  } >FLASH

  .text :
  {  
    
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))
    
    /* Gomspace commands */
    __command_start = .;
    *(.commands)
    __command_end = .;
    . = ALIGN(8);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(8);
  } >FLASH

  .ARM.extab   : 
  { 
  . = ALIGN(8);
  *(.ARM.extab* .gnu.linkonce.armextab.*)
  . = ALIGN(8);
  } >FLASH
  .ARM : {
	. = ALIGN(8);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
	. = ALIGN(8);
  } >FLASH

  .preinit_array     :
  {
	. = ALIGN(8);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
	. = ALIGN(8);
  } >FLASH
  
  .init_array :
  {
	. = ALIGN(8);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
	. = ALIGN(8);
  } >FLASH
  .fini_array :
  {
	. = ALIGN(8);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
	. = ALIGN(8);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data : 
  {
    . = ALIGN(8);
    _sdata = .;
    *(.data)
    *(.data*)

    /* Gomspace log_groups */
    __start_log_groups = .;
    *(log_groups);
    __stop_log_groups = .; 
    
    . = ALIGN(8);
    _edata = .;
  } >RAM AT> FLASH

  . = ALIGN(4);
  .bss :
  {
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Gomspace: Reserved FLASH for user data (r/w data in Flash) */
  .flash_data (NOLOAD) : 
  {
    . = ALIGN(2048);
    _FLASH_DATA_START = .;
    . = . + _FLASH_DATA_SIZE;
    _FLASH_DATA_END = .;
  } >FLASH_DATA

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}

