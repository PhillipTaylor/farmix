#ifndef __SYSTEM_H
#define __SYSTEM_H

#define NULL 0

typedef unsigned int size_t;

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;    
};

/* MAIN.C */
extern unsigned char inportb (unsigned short _port);
extern void outportb (unsigned short _port, char _data);

/* MEM_UTILS.C */
#define OUT_OF_MEMORY -1
extern unsigned int grub_boot_memory_map;
extern unsigned int grub_boot_magic_number;
extern void *memcpy(void *dest, const void *src, size_t count);
extern void *memset(void *dest, char val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);
extern void init_memory();
extern void print_memory_map();
extern void *malloc(unsigned int num_bytes);
extern void free(void *start_address);

/* STR_UTILS.C */

/* kprint and kbprintf supported formats: %s, %x (hex), %i (int), %c, %o (octal), %% */
extern void kprintf(char *format, ...);
extern int kbprintf(char *buffer, int max_size, const char* format, ...); //same as printf. writes to buffer.
extern int strcat(char *buffer, int max_size, char *strcat);
extern int strcpy(char *buffer, int max_size, char *strcat);
extern size_t strlen(const char *str);

/* CONSOLE.C */
extern void init_video(void);
extern void puts(char *text);
extern void putch(char c);
extern void cls();

/* GDT.C */
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_install();

/* IDT.C */
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void idt_install();

/* ISRS.C */
extern void isrs_install();

/* IRQ.C */
extern void irq_install_handler(int irq, void (*handler)(struct regs *r));
extern void irq_uninstall_handler(int irq);
extern void irq_install();

/* TIMER.C */
extern void timer_wait(int ticks);
extern void timer_install();

/* KEYBOARD.C */
extern void keyboard_install();

#endif
