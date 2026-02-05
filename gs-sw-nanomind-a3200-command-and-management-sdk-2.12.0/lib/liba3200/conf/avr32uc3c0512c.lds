OUTPUT_FORMAT("elf32-avr32", "elf32-avr32", "elf32-avr32")
OUTPUT_ARCH(avr32:uc)
ENTRY(_start)

_FLASH_DATA_SIZE = (DEFINED(GS_FLASH_DATA_SIZE) ? GS_FLASH_DATA_SIZE : 0K);

MEMORY {
	FLASH (rxai!w) : ORIGIN = 0x80000000, LENGTH = 512K
	CPUSRAM (wxa!ri) : ORIGIN = 0x00000004, LENGTH = 64K - 4
	SDRAM (wxa!ri) : ORIGIN = 0xD0000000, LENGTH = 32M
}

PHDRS {
	text PT_LOAD;
	data PT_LOAD;
	sram PT_NULL;
	flash_data PT_NULL;
}

SECTIONS {

	.text : {
		*(.reset)
		*(.rela.got)
		KEEP(*(.init))
		*(.text)
		*(.text.*)
		KEEP(*(.fini))
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(4);
		__command_start = .;
		*(.commands);
		__command_end = .;
		. = ALIGN(8);
		_etext = .;
	} >FLASH AT>FLASH : text
	
	.scratchpad (NOLOAD) : {
		*(.reset_cause_store)
	} >CPUSRAM : sram

	.data : {
		_data_lma = LOADADDR(.data);
		_data = .;
		KEEP (*crtbegin*.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		KEEP (*crtbegin*.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		__start_log_groups = .;
		*(log_groups);
		__stop_log_groups = .;
		*(.got)
		*(.data)
		*(.data.*)
		. = ALIGN(8);
		_edata = .;
	} >CPUSRAM AT>FLASH : data

	/* Gomspace: Reserved FLASH area for user data (r/w data in Flash) - MUST be last entry for '>FLASH' */
	.flash_data (NOLOAD) : {
		. = ABSOLUTE(ORIGIN(FLASH) + LENGTH(FLASH) - _FLASH_DATA_SIZE);
		. = ALIGN(512);
		_FLASH_DATA_START = .;
		. = . + _FLASH_DATA_SIZE;
		_FLASH_DATA_END = .;
	} >FLASH AT>FLASH : flash_data

	.bss (NOLOAD) : {
		__bss_start = .;
		*(COMMON)
		*(.bss.*)
		*(.bss)
		. = ALIGN(8);
		_end = .;
	} >CPUSRAM : sram

	__heap_start__ = .;
	__heap_end__ = ORIGIN(CPUSRAM) + LENGTH(CPUSRAM) - 1; /* overlap with _estack */
	_estack = ORIGIN(CPUSRAM) + LENGTH(CPUSRAM);          /* once FreeRTOS is started, stack no longer used */

}
