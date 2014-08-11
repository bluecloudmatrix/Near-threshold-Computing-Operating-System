/* initialize GDT/IDT */

#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT; //#define ADR_GDT			0x00270000
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;
	
	/* initialize GDT */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092); // it represents all the memory that CPU can manage. limit = 4G 
														   // 0x4092   0100 0000 10010010 -> 32-bit mode, system special, read and write, not execute
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); // 512K, from  0x00280000, prepared for bootpack.hrb
														   // 0x409a -> 32-bit mode, system special(ring0), execute and read, not write
	load_gdtr(0xffff, ADR_GDT); // with the help of assembly
	
	/* initialize IDT */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	
	load_idtr(0x7ff, ADR_IDT); // with the help of assembly
	
	/* setting IDT */
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	
	return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f)|((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
 }
 
 void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
 {
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
 }