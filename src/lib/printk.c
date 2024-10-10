#include <stdarg.h>
#include "toolkit.h"
#include "mmkit.h"

semaphore_t visual_lock;

void putchar(u32_t *fb, s32_t Xsize, s32_t x, s32_t y, u32_t FRcolor, u32_t BKcolor, u8_t font)
{
	u32_t i = 0, j = 0;
	u32_t *addr = nullptr;
	u8_t *fontp = nullptr;
	s32_t testval = 0;
	fontp = font_ascii[font];
	semaphore_down(&visual_lock);

	for (i = 0; i < 16; i++)
	{
		addr = fb + Xsize * (y + i) + x;
		testval = 0x100;
		for (j = 0; j < 8; j++)
		{
			testval = testval >> 1;
			if (*fontp & testval)
				*addr = FRcolor;
			else
				*addr = BKcolor;
			addr++;
		}
		fontp++;
	}
	semaphore_up(&visual_lock);
}

s32_t skip_atoi(cstr_t *s)
{
	s32_t i = 0;

	while (is_digit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

static str_t number(str_t str, s64_t num, s32_t base, s32_t size, s32_t precision, s32_t type)
{
	char_t c, sign, tmp[50];
	cstr_t digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	s32_t i;

	if (type & SMALL)
		digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN && num < 0)
	{
		sign = '-';
		num = -num;
	}
	else
		sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
	if (sign)
		size--;
	if (type & SPECIAL) {
		if (base == 16) {
			size -= 2;
		} else if (base == 8) {
			size--;
		}
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = digits[do_div(num, base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL)
	{
		if (base == 8) {
			*str++ = '0';
		} else if (base == 16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}

	if (!(type & LEFT)) {
		while (size-- > 0)
			*str++ = c;
	}
	
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

s32_t vsprintf(str_t buf, cstr_t fmt, va_list args)
{
	char_t *str, *s;
	s32_t flags;
	s32_t field_width;
	s32_t precision;
	s32_t len, i;

	s32_t qualifier; /* 'h', 'l', 'L' or 'Z' for integer fields */

	for (str = buf; *fmt; fmt++)
	{

		if (*fmt != '%')
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;
	repeat:
		fmt++;
		switch (*fmt)
		{
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */

		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			fmt++;
			field_width = va_arg(args, s32_t);
			if (field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */

		precision = -1;
		if (*fmt == '.')
		{
			fmt++;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				fmt++;
				precision = va_arg(args, s32_t);
			}
			if (precision < 0)
				precision = 0;
		}

		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
		{
			qualifier = *fmt;
			fmt++;
		}

		switch (*fmt)
		{
		case 'c':

			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (u8_t)va_arg(args, s32_t);
			while (--field_width > 0)
				*str++ = ' ';
			break;

		case 's':

			s = va_arg(args, char *);
			if (!s)
				s = '\0';
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; i++)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;

		case 'o':

			if (qualifier == 'l')
				str = number(str, va_arg(args, u64_t), 8, field_width, precision, flags);
			else
				str = number(str, va_arg(args, u32_t), 8, field_width, precision, flags);
			break;

		case 'p':

			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}

			str = number(str, (u64_t)va_arg(args, void *), 16, field_width, precision, flags);
			break;

		case 'x':

			flags |= SMALL;

		case 'X':

			if (qualifier == 'l')
				str = number(str, va_arg(args, u64_t), 16, field_width, precision, flags);
			else
				str = number(str, va_arg(args, u32_t), 16, field_width, precision, flags);
			break;

		case 'd':
		case 'i':

			flags |= SIGN;
		case 'u':

			if (qualifier == 'l')
				str = number(str, va_arg(args, s64_t), 10, field_width, precision, flags);
			else
				str = number(str, va_arg(args, s32_t), 10, field_width, precision, flags);
			break;

		case 'n':

			if (qualifier == 'l')
			{
				s64_t *ip = va_arg(args, s64_t *);
				*ip = (str - buf);
			}
			else
			{
				s32_t *ip = va_arg(args, s32_t *);
				*ip = (str - buf);
			}
			break;

		case '%':

			*str++ = '%';
			break;

		default:

			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				fmt--;
			break;
		}
	}
	*str = '\0';
	return str - buf;
}

s32_t sprintf(str_t buf, cstr_t fmt, ...)
{
	s32_t count = 0;
	va_list args;

	va_start(args, fmt);
	count = vsprintf(buf, fmt, args);
	va_end(args);

	return count;
}

s32_t color_printk(u32_t FRcolor, u32_t BKcolor, cstr_t fmt, ...)
{

	s32_t i = 0;
	s32_t count = 0;
	s32_t line = 0;
	va_list args;
	va_start(args, fmt);

	// 通过IF位，来判断是否是中断处理程序，中断处理程序的IF位是0
	// 不是中断处理程序，那么我们就进行自旋锁
	if (get_rflags() & 0x200UL)
		fair_spin_lock(&Pos.printk_lock);

	i = vsprintf(buf, fmt, args);

	va_end(args);
	
	// 这里光标的设置略感丑陋 😋 
	// 删除上次光标位置
	putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, WHITE, BLACK, 0);
	
	for (count = 0; count < i || line; count++)
	{
		////	add \n \b \t
		if (line > 0)
		{
			count--;
			goto Label_tab;
		}
		if ((u8_t)*(buf + count) == '\n')
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		else if ((u8_t)*(buf + count) == '\b')
		{
			Pos.XPosition--;
			if (Pos.XPosition < 0)
			{
				Pos.XPosition = (Pos.XResolution / Pos.XCharSize - 1) * Pos.XCharSize;
				Pos.YPosition--;
				if (Pos.YPosition < 0)
					Pos.YPosition = (Pos.YResolution / Pos.YCharSize - 1) * Pos.YCharSize;
			}
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');
		}
		else if ((u8_t)*(buf + count) == '\t')
		{
			line = ((Pos.XPosition + 8) & ~(8 - 1)) - Pos.XPosition;

		Label_tab:
			line--;
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');
			Pos.XPosition++;
		}
		else
		{
			putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, (u8_t)*(buf + count));
			Pos.XPosition++;
		}

		if (Pos.XPosition >= (Pos.XResolution / Pos.XCharSize))
		{
			Pos.YPosition++;
			Pos.XPosition = 0;
		}
		if (Pos.YPosition >= (Pos.YResolution / Pos.YCharSize))
		{
			Pos.YPosition = 0;
		}
	}

	// 显示光标
	putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, WHITE, BLACK, 255);
	
	if (get_rflags() & 0x200UL)
		fair_spin_unlock(&Pos.printk_lock);
	
	return i;
}



/**
 * @brief 把VBE帧缓存区地址重映射，在页表重新初始化过程中覆盖了VBE帧缓存区, 导致pagetable_init函数无法在屏幕上
 * 显示日志信息，
 *      这里会把VBE帧缓存区地址0xfd000000-映射到线性地址0xffff8000fd000000这个内存空洞
 */
void frame_buffer_init()
{
	/////// re init frame buffer
	u64_t i;
	u64_t *tmp;
	u64_t FB_addr = PAGE_OFFSET + VBE_Phy_address;
	u64_t *virtual = nullptr;

	Global_CR3 = Get_gdt();

    for(i = 0; i < Pos.FB_length; i += PAGE_4K_SIZE) {
		tmp = Phy_To_Virt((u64_t *)((u64_t)Global_CR3 & (~0xfffUL))) + (((FB_addr + i) >> PAGE_GDT_SHIFT) & 0x1ff);
		if (*tmp == 0)
		{
			virtual = knew(PAGE_4K_SIZE, 1); // 申请PDPT内存，填充PML4页表项
			memset(virtual, 0, PAGE_4K_SIZE);

			set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(virtual), PAGE_USER_Dir));
		}
	
		// 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
		tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL))) + (((FB_addr + i) >> PAGE_1G_SHIFT) & 0x1ff);
		if (*tmp == 0)
		{
			virtual = knew(PAGE_4K_SIZE, 1); // 申请PDT内存，填充PDPT页表项
			memset(virtual, 0, PAGE_4K_SIZE);

			set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
		}
	
		// 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
		// 申请用户占用的内存,填充页表, 填充PDT内存
		tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL))) + (((FB_addr + i) >> PAGE_2M_SHIFT) & 0x1ff);
		if (*tmp == 0)
		{	
			virtual = knew(PAGE_4K_SIZE, 1); // 申请page_table 内存，填充page_dirctory页表项
			memset(virtual, 0, PAGE_4K_SIZE);
			set_pdt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
		}

		tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL))) + (((FB_addr + i) >> PAGE_4K_SHIFT) & 0x1ff);

		set_pt(tmp, mk_pdpt(VBE_Phy_address + i, PAGE_USER_Page_4K));
	
    }

	Pos.FB_addr = (u32_t*)Phy_To_Virt(VBE_Phy_address);

	u64_t* old_map = Phy_To_Virt(0x103000  + 23 * 8); 
	memset(old_map, 0, 8 * 9);
	
	flush_tlb();
	return;
}


// void frame_buffer_init()
// {
// 	/////// re init frame buffer
// 	u64_t i;
// 	u64_t *tmp;
// 	u64_t *tmp1;
// 	u32_t *FB_addr = (u32_t *)Phy_To_Virt(VBE_Phy_address);
// 	u64_t *vir_address;
// 	Global_CR3 = Get_gdt();

// 	// 获取该虚拟地址对应的PML(page map level 4, 4级页表)中的页表项指针
// 	tmp = Phy_To_Virt((u64_t *)((u64_t)Global_CR3 & (~0xfffUL))) +
// 		  (((u64_t)FB_addr >> PAGE_GDT_SHIFT) & 0x1ff);
// 	if (*tmp == 0)
// 	{ // 页表项为空，则分配4kbPDPT页表,填充该表项
// 		vir_address = kmalloc(PAGE_4K_SIZE, 0);
// 		set_mpl4t(tmp, mk_mpl4t(Virt_To_Phy(vir_address), PAGE_KERNEL_GDT));
// 	}
// 	//=======================================================================================

// 	// 获取该虚拟地址对应的PDPT(page directory point table)中的页表项指针
// 	tmp = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL))) + (((u64_t)FB_addr >> PAGE_1G_SHIFT) & 0x1ff);
// 	if (*tmp == 0)
// 	{
// 		vir_address = kmalloc(PAGE_4K_SIZE, 0);
// 		set_pdpt(tmp, mk_pdpt(Virt_To_Phy(vir_address), PAGE_KERNEL_Dir));
// 	}
// 	//============================================================
// 	for (i = 0; i < Pos.FB_length; i += PAGE_2M_SIZE)
// 	{ // 获取该虚拟地址对应的PDT(page directory table)中的页表项指针
// 		tmp1 = Phy_To_Virt((u64_t *)(*tmp & (~0xfffUL))) + (((u64_t)FB_addr + i >> PAGE_2M_SHIFT) & 0x1ff);
// 		// 填写该表项
// 		u64_t phy = VBE_Phy_address + i;
// 		set_pdt(tmp1, mk_pdpt(phy, PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD));
// 	}

// 	Pos.FB_addr = (u32_t *)Phy_To_Virt(VBE_Phy_address);

// 	flush_tlb();
// }