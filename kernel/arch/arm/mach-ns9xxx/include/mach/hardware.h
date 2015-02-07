
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#define io_p2v(x)	(0xf0000000 \
			 + (((x) & 0xf0000000) >> 4) \
			 + ((x) & 0x00ffffff))

#define io_v2p(x)	((((x) & 0x0f000000) << 4) \
			 + ((x) & 0x00ffffff))

#define __REGSHIFT(mask)	((mask) & (-(mask)))

#define __REGBIT(bit)		((u32)1 << (bit))
#define __REGBITS(hbit, lbit)	((((u32)1 << ((hbit) - (lbit) + 1)) - 1) << (lbit))
#define __REGVAL(mask, value)	(((value) * __REGSHIFT(mask)) & (mask))

#ifndef __ASSEMBLY__

#  define __REG(x)	((void __iomem __force *)io_p2v((x)))
#  define __REG2(x, y)	((void __iomem __force *)(io_p2v((x)) + 4 * (y)))

#  define __REGSET(var, field, value)					\
	((var) = (((var) & ~((field) & ~(value))) | (value)))

#  define REGSET(var, reg, field, value)				\
	__REGSET(var, reg ## _ ## field, reg ## _ ## field ## _ ## value)

#  define REGSET_IDX(var, reg, field, idx, value)			\
	__REGSET(var, reg ## _ ## field((idx)), reg ## _ ## field ## _ ## value((idx)))

#  define REGSETIM(var, reg, field, value)				\
	__REGSET(var, reg ## _ ## field, __REGVAL(reg ## _ ## field, (value)))

#  define REGSETIM_IDX(var, reg, field, idx, value)			\
	__REGSET(var, reg ## _ ## field((idx)), __REGVAL(reg ## _ ## field((idx)), (value)))

#  define __REGGET(var, field)						\
	(((var) & (field)))

#  define REGGET(var, reg, field)					\
	 __REGGET(var, reg ## _ ## field)

#  define REGGET_IDX(var, reg, field, idx)				\
	 __REGGET(var, reg ## _ ## field((idx)))

#  define REGGETIM(var, reg, field)					\
	 __REGGET(var, reg ## _ ## field) / __REGSHIFT(reg ## _ ## field)

#  define REGGETIM_IDX(var, reg, field, idx)				\
	 __REGGET(var, reg ## _ ## field((idx))) /			\
	 __REGSHIFT(reg ## _ ## field((idx)))

#else

#  define __REG(x)	io_p2v(x)
#  define __REG2(x, y)	io_p2v((x) + 4 * (y))

#endif

#endif /* ifndef __ASM_ARCH_HARDWARE_H */