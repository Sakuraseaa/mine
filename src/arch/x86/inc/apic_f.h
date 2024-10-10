#ifndef _APIC_F_H_
#define _APIC_F_H_

void IOAPIC_enable(u64_t irq);
void IOAPIC_disable(u64_t irq);
u64_t IOAPIC_install(u64_t irq, void *arg);
void IOAPIC_uninstall(u64_t irq);
void IOAPIC_level_ack(u64_t irq);
void IOAPIC_edge_ack(u64_t irq);

#endif // _APIC_F_H_