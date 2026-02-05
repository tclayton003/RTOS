/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
#include <gs/util/stdio.h>
#include <gs/util/crc32.h>
#include <avr32/io.h>
#include "wdt.h"
#include <gs/asf/avr32/asf.h>
#include <gs/asf/avr32/sysreg.h>
#include <gs/asf/avr32/ptrace.h>
#include <gs/embed/drivers/sys/reset.h>

static const char * const cpu_modes[8] = {
	"Application", "Supervisor", "Interrupt level 0", "Interrupt level 1",
	"Interrupt level 2", "Interrupt level 3", "Exception", "NMI"
};

struct pt_regs {
	/* These are always saved */
	unsigned long sr;
	unsigned long pc;

	/* These are sometimes saved */
	unsigned long lr;
	unsigned long sp;
	unsigned long r12;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long r7;
	unsigned long r6;
	unsigned long r5;
	unsigned long r4;
	unsigned long r3;
	unsigned long r2;
	unsigned long r1;
	unsigned long r0;

	/* Only saved on system call */
	unsigned long r12_orig;
};

static void dump_mem(const char *str, unsigned long bottom, unsigned long top)
{
	unsigned long p;
	int i;

	printf("%s(0x%08lx to 0x%08lx)\r\n", str, bottom, top);

	for (p = bottom & ~31; p < top; ) {
		printf("%04lx: ", p & 0xffff);

		for (i = 0; i < 8; i++, p += 4) {
			unsigned int val;

			if (p < bottom || p >= top)
				printf("         ");
			else {
				val = *(unsigned long *)p;
				printf("%08x ", val);
			}
		}
		printf("\r\n");
	}
}

extern volatile gs_reset_cause_store_t gs_reset_cause;
void do_unknown_exception(unsigned int ecr, struct pt_regs *regs) {
	unsigned int mode;

	printf("\r\n\r\n");
	printf("!!! EXCEPTION %u !!!\r\n", ecr);

	switch (ecr) {
	case ECR_BUS_ERROR_WRITE:
	case ECR_BUS_ERROR_READ:
		printf("Bus error at address 0x%08x\r\n", sysreg_read(SYSREG_BEAR));
		break;

	case ECR_ADDR_ALIGN_X:
	case ECR_ADDR_ALIGN_R:
	case ECR_ADDR_ALIGN_W:
		printf("Addr align exception at address 0x%08x\r\n", sysreg_read(SYSREG_BEAR));
		break;

	case ECR_PROTECTION_X:
	case ECR_PROTECTION_R:
	case ECR_PROTECTION_W:
	case ECR_DTLB_MODIFIED:
	case ECR_TLB_MULTIPLE:
	case ECR_TLB_MISS_X:
	case ECR_TLB_MISS_R:
	case ECR_TLB_MISS_W:
		printf("MMU exception at address 0x%08x\r\n", sysreg_read(SYSREG_TLBEAR));
		break;

	}

	printf("   pc: %08lx    lr: %08lx    sp: %08lx   r12: %08lx\r\n",
	       regs->pc, regs->lr, regs->sp, regs->r12);
	printf("  r11: %08lx   r10: %08lx    r9: %08lx    r8: %08lx\r\n",
	       regs->r11, regs->r10, regs->r9, regs->r8);
	printf("   r7: %08lx    r6: %08lx    r5: %08lx    r4: %08lx\r\n",
	       regs->r7, regs->r6, regs->r5, regs->r4);
	printf("   r3: %08lx    r2: %08lx    r1: %08lx    r0: %08lx\r\n",
	       regs->r3, regs->r2, regs->r1, regs->r0);
	printf("Flags: %c%c%c%c%c\r\n",
	       (regs->sr & SR_Q) ? 'Q' : 'q',
         (regs->sr & SR_V) ? 'V' : 'v',
         (regs->sr & SR_N) ? 'N' : 'n',
         (regs->sr & SR_Z) ? 'Z' : 'z',
	       (regs->sr & SR_C) ? 'C' : 'c');
	printf("Mode bits: %c%c%c%c%c%c%c%c%c\r\n",
	       (regs->sr & SR_H) ? 'H' : 'h',
	       (regs->sr & SR_R) ? 'R' : 'r',
	       (regs->sr & SR_J) ? 'J' : 'j',
	       (regs->sr & SR_EM) ? 'E' : 'e',
	       (regs->sr & SR_I3M) ? '3' : '.',
	       (regs->sr & SR_I2M) ? '2' : '.',
         (regs->sr & SR_I1M) ? '1' : '.',
         (regs->sr & SR_I0M) ? '0' : '.',
         (regs->sr & SR_GM) ? 'G' : 'g');
	mode = (regs->sr >> SYSREG_M0_OFFSET) & 7;
	printf("CPU Mode: %s\r\n", cpu_modes[mode]);

	/* Avoid exception loops */
	dump_mem("\r\nStack: ", regs->sp, regs->sp + 256);

	printf("\r\n");

	gs_reset_cause.crash_addr = regs->lr;
	gs_reset_cause.crash_addr_crc = gs_crc32((void*)&gs_reset_cause.crash_addr, sizeof(gs_reset_cause.crash_addr));

	gs_sys_reset(GS_SYS_RESET_EXCEPTION);
}
