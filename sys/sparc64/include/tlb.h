/*-
 * Copyright (c) 2001 Jake Burkholder.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef	_MACHINE_TLB_H_
#define	_MACHINE_TLB_H_

#define	TLB_SLOT_COUNT			64

#define	TLB_SLOT_TSB_KERNEL_MIN		60	/* XXX */
#define	TLB_SLOT_TSB_USER_PRIMARY	61
#define	TLB_SLOT_TSB_USER_SECONDARY	62
#define	TLB_SLOT_KERNEL			63

#define	TLB_DAR_SLOT_SHIFT		(3)
#define	TLB_DAR_SLOT(slot)		((slot) << TLB_DAR_SLOT_SHIFT)

#define	TLB_TAR_VA(va)			((va) & ~PAGE_MASK)
#define	TLB_TAR_CTX(ctx)		((ctx) & PAGE_MASK)

#define	TLB_DEMAP_ID_SHIFT		(4)
#define	TLB_DEMAP_ID_PRIMARY		(0)
#define	TLB_DEMAP_ID_SECONDARY		(1)
#define	TLB_DEMAP_ID_NUCLEUS		(2)

#define	TLB_DEMAP_TYPE_SHIFT		(6)
#define	TLB_DEMAP_TYPE_PAGE		(0)
#define	TLB_DEMAP_TYPE_CONTEXT		(1)

#define	TLB_DEMAP_VA(va)		((va) & ~PAGE_MASK)
#define	TLB_DEMAP_ID(id)		((id) << TLB_DEMAP_ID_SHIFT)
#define	TLB_DEMAP_TYPE(type)		((type) << TLB_DEMAP_TYPE_SHIFT)

#define	TLB_DEMAP_PAGE			(TLB_DEMAP_TYPE(TLB_DEMAP_TYPE_PAGE))
#define	TLB_DEMAP_CONTEXT		(TLB_DEMAP_TYPE(TLB_DEMAP_TYPE_CONTEXT))

#define	TLB_DEMAP_PRIMARY		(TLB_DEMAP_ID(TLB_DEMAP_ID_PRIMARY))
#define	TLB_DEMAP_SECONDARY		(TLB_DEMAP_ID(TLB_DEMAP_ID_SECONDARY))
#define	TLB_DEMAP_NUCLEUS		(TLB_DEMAP_ID(TLB_DEMAP_ID_NUCLEUS))

#define	TLB_CTX_KERNEL			(0)

#define	TLB_DTLB			(1 << 0)
#define	TLB_ITLB			(1 << 1)

#define	MMU_SFSR_ASI_SHIFT		(16)
#define	MMU_SFSR_FT_SHIFT		(7)
#define	MMU_SFSR_E_SHIFT		(6)
#define	MMU_SFSR_CT_SHIFT		(4)
#define	MMU_SFSR_PR_SHIFT		(3)
#define	MMU_SFSR_W_SHIFT		(2)
#define	MMU_SFSR_OW_SHIFT		(1)
#define	MMU_SFSR_FV_SHIFT		(0)

#define	MMU_SFSR_ASI_SIZE		(8)
#define	MMU_SFSR_FT_SIZE		(6)
#define	MMU_SFSR_CT_SIZE		(2)

#define	MMU_SFSR_W			(1L << MMU_SFSR_W_SHIFT)

static __inline void
tlb_dtlb_page_demap(u_long ctx, vm_offset_t va)
{
	if (ctx == TLB_CTX_KERNEL) {
		stxa(TLB_DEMAP_VA(va) | TLB_DEMAP_NUCLEUS | TLB_DEMAP_PAGE,
		    ASI_DMMU_DEMAP, 0);
		membar(Sync);
	} else {
		stxa(AA_DMMU_SCXR, ASI_DMMU, ctx);
		membar(Sync);
		stxa(TLB_DEMAP_VA(va) | TLB_DEMAP_SECONDARY | TLB_DEMAP_PAGE,
		    ASI_DMMU_DEMAP, 0);
		stxa(AA_DMMU_SCXR, ASI_DMMU, 0);
		membar(Sync);
	}
}

static __inline void
tlb_dtlb_store(vm_offset_t va, u_long ctx, struct tte tte)
{
	stxa(AA_DMMU_TAR, ASI_DMMU,
	    TLB_TAR_VA(va) | TLB_TAR_CTX(ctx));
	stxa(0, ASI_DTLB_DATA_IN_REG, tte.tte_data);
	membar(Sync);
}

static __inline void
tlb_dtlb_store_slot(vm_offset_t va, u_long ctx, struct tte tte, int slot)
{
	stxa(AA_DMMU_TAR, ASI_DMMU, TLB_TAR_VA(va) | TLB_TAR_CTX(ctx));
	stxa(TLB_DAR_SLOT(slot), ASI_DTLB_DATA_ACCESS_REG, tte.tte_data);
	membar(Sync);
}

static __inline void
tlb_itlb_page_demap(u_long ctx, vm_offset_t va)
{
	if (ctx == TLB_CTX_KERNEL) {
		stxa(TLB_DEMAP_VA(va) | TLB_DEMAP_NUCLEUS | TLB_DEMAP_PAGE,
		    ASI_IMMU_DEMAP, 0);
		flush(KERNBASE);
	} else {
		stxa(AA_DMMU_SCXR, ASI_DMMU, ctx);
		membar(Sync);
		stxa(TLB_DEMAP_VA(va) | TLB_DEMAP_SECONDARY | TLB_DEMAP_PAGE,
		    ASI_IMMU_DEMAP, 0);
		stxa(AA_DMMU_SCXR, ASI_DMMU, 0);
		/* flush probably not needed. */
		membar(Sync);
	}
}

static __inline void
tlb_itlb_store(vm_offset_t va, u_long ctx, struct tte tte)
{
	stxa(AA_IMMU_TAR, ASI_IMMU, TLB_TAR_VA(va) | TLB_TAR_CTX(ctx));
	stxa(0, ASI_ITLB_DATA_IN_REG, tte.tte_data);
	if (ctx == TLB_CTX_KERNEL)
		flush(va);
	else {
		/*
		 * flush probably not needed and impossible here, no access to
		 * user page.
		 */
		membar(Sync);
	}
}

static __inline void
tlb_itlb_store_slot(vm_offset_t va, u_long ctx, struct tte tte, int slot)
{
	stxa(AA_IMMU_TAR, ASI_IMMU, TLB_TAR_VA(va) | TLB_TAR_CTX(ctx));
	stxa(TLB_DAR_SLOT(slot), ASI_ITLB_DATA_ACCESS_REG, tte.tte_data);
	flush(va);
}

static __inline void
tlb_page_demap(u_int tlb, u_int ctx, vm_offset_t va)
{
	if (tlb & TLB_DTLB)
		tlb_dtlb_page_demap(ctx, va);
	if (tlb & TLB_ITLB)
		tlb_itlb_page_demap(ctx, va);
}

static __inline void
tlb_store(u_int tlb, vm_offset_t va, u_long ctx, struct tte tte)
{
	if (tlb & TLB_DTLB)
		tlb_dtlb_store(va, ctx, tte);
	if (tlb & TLB_ITLB)
		tlb_itlb_store(va, ctx, tte);
}

static __inline void
tlb_store_slot(u_int tlb, vm_offset_t va, u_long ctx, struct tte tte, int slot)
{
	if (tlb & TLB_DTLB)
		tlb_dtlb_store_slot(va, ctx, tte, slot);
	if (tlb & TLB_ITLB)
		tlb_itlb_store_slot(va, ctx, tte, slot);
}

#endif /* !_MACHINE_TLB_H_ */
