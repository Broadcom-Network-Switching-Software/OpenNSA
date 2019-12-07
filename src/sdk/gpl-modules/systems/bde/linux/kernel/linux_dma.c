/*
 * Copyright 2017 Broadcom
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 */
/*
 * Linux Kernel BDE DMA memory allocation
 *
 *
 * DMA memory allocation modes
 * ===========================
 *
 * 1. Using private pool in kernel memory
 * --------------------------------------
 * In this mode the BDE module will try to assemble a physically contiguous
 * of memory using the kernel page allocator. This memory block is then
 * administered by the mpool allocation functions. Note that once a system
 * has been running for a while, the memory fragmentation may prevent the
 * allocator from assembling a contiguous memory block, however, if the
 * module is loaded shortly after system startup, it is very unlikely to
 * fail.
 *
 * This allocation method is used by default.
 *
 * 2. Using private pool in high memory
 * ------------------------------------
 * In this mode the BDE module will assume that unused physical memory is
 * present at the high_memory address, i.e. memory not managed by the Linux
 * memory manager. This memory block is mapped into kernel space and
 * administered by the mpool allocation functions. High memory must be
 * reserved using either the mem=xxx kernel parameter (recommended), or by
 * hardcoding the memory limit in the kernel image.
 *
 * The module parameter himem=1 enables this allocation mode.
 *
 * 3. Using kernel allocators (kmalloc, __get_free_pages)
 * ------------------------------------------------------
 * In this mode all DMA memory is allocated from the kernel on the fly, i.e.
 * no private DMA memory pool will be created. If large memory blocks are
 * only allocated at system startup (or not at all), this allocation method
 * is the most flexible and memory-efficient, however, it is not recommended
 * for non-coherent memory platforms due to an overall system performance
 * degradation arising from the use of cache flush/invalidate instructions.
 *
 * The module parameter dmasize=0M enables this allocation mode, however if
 * DMA memory is requested from a user mode application, a private memory
 * pool will be created and used irrespectively.
 */

#include <gmodule.h>
#include <linux-bde.h>
#include <linux_dma.h>
#include <mpool.h>
#include <sdk_config.h>

#ifdef BCM_PLX9656_LOCAL_BUS
#include <asm/cacheflush.h>
#endif

/* allocation types/methods for the DMA memory pool */
#define ALLOC_TYPE_CHUNK 0 /* use small allocations and join them */
#define ALLOC_TYPE_API 1 /* use one allocation */
#if _SIMPLE_MEMORY_ALLOCATION_
#include <linux/dma-mapping.h>
#if defined(IPROC_CMICD) && defined(CONFIG_CMA) && defined(CONFIG_CMA_SIZE_MBYTES)
#define DMA_MAX_ALLOC_SIZE (CONFIG_CMA_SIZE_MBYTES * 1024 * 1024)
#else
#define DMA_MAX_ALLOC_SIZE (1 << (MAX_ORDER - 1 + PAGE_SHIFT)) /* Maximum size the kernel can allocate in one allocation */
#endif
#endif /* _SIMPLE_MEMORY_ALLOCATION_ */

#if _SIMPLE_MEMORY_ALLOCATION_ == 1
#define ALLOC_METHOD_DEFAULT ALLOC_TYPE_API
#if defined(__arm__)
#define USE_DMA_MMAP_COHERENT
#define _PGPROT_NONCACHED(x) x = pgprot_noncached((x))
#elif defined(__aarch64__ )
#define USE_DMA_MMAP_COHERENT
#define _PGPROT_NONCACHED(x) x = pgprot_writecombine((x))
#endif
#else
#define ALLOC_METHOD_DEFAULT ALLOC_TYPE_CHUNK
#endif

#ifndef _PGPROT_NONCACHED
#ifdef REMAP_DMA_NONCACHED
#define _PGPROT_NONCACHED(x) x = pgprot_noncached((x))
#else
#define _PGPROT_NONCACHED(x)
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/slab.h>
#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#define VIRT_TO_PAGE(p)     virt_to_page((void*)(p))
#else
#define VIRT_TO_PAGE(p)     virt_to_page((p))
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
#define DMA_MAPPING_ERROR(d, p)     dma_mapping_error((d),(p))
#else
#define DMA_MAPPING_ERROR(d, p)     dma_mapping_error((p))
#endif

#ifndef KMALLOC_MAX_SIZE
#define KMALLOC_MAX_SIZE (1UL << (MAX_ORDER - 1 + PAGE_SHIFT))
#endif

/* Compatibility */
#ifdef LKM_2_4
#define MEM_MAP_RESERVE mem_map_reserve
#define MEM_MAP_UNRESERVE mem_map_unreserve
#else /* LKM_2_6 */
#define MEM_MAP_RESERVE SetPageReserved
#define MEM_MAP_UNRESERVE ClearPageReserved
#endif /* LKM_2_x */

#ifndef GFP_DMA32
#define GFP_DMA32 0
#endif

/* Flags for memory allocations */
#ifdef SAL_BDE_XLP
static int mem_flags = GFP_ATOMIC | GFP_KERNEL | GFP_DMA;
#else
#if defined(CONFIG_ZONE_DMA32)
static int mem_flags = GFP_ATOMIC | GFP_DMA32;
#else
static int mem_flags = GFP_ATOMIC | GFP_DMA;
#endif
#endif

/* Debug output */
static int dma_debug = 0;
module_param(dma_debug, int, 0);
MODULE_PARM_DESC(dma_debug,
"DMA debug output enable (default 0).");

/* DMA memory pool size */
static char *dmasize;
LKM_MOD_PARAM(dmasize, "s", charp, 0);
MODULE_PARM_DESC(dmasize,
"Specify DMA memory size (default 4MB)");

/* Select DMA memory pool allocation method */
static int dmaalloc = ALLOC_METHOD_DEFAULT;
LKM_MOD_PARAM(dmaalloc, "i", int, 0);
MODULE_PARM_DESC(dmaalloc, "Select DMA memory allocation method");

/* Use high memory for DMA */
static char *himem;
LKM_MOD_PARAM(himem, "s", charp, 0);
MODULE_PARM_DESC(himem,
"Use high memory for DMA (default no)");

/* Physical high memory address to use for DMA */
static char *himemaddr = 0;
LKM_MOD_PARAM(himemaddr, "s", charp, 0);
MODULE_PARM_DESC(himemaddr,
"Physical address to use for high memory DMA");

/* DMA memory allocation */

#define ONE_KB 1024
#define ONE_MB (1024*1024)
#define ONE_GB (1024*1024*1024)

/* Default DMA memory size */
#ifdef SAL_BDE_DMA_MEM_DEFAULT
#define DMA_MEM_DEFAULT (SAL_BDE_DMA_MEM_DEFAULT * ONE_MB)
#else
#define DMA_MEM_DEFAULT (8 * ONE_MB)
#endif

/* We try to assemble a contiguous segment from chunks of this size */
#define DMA_BLOCK_SIZE (512 * ONE_KB)

typedef struct _dma_segment {
    struct list_head list;
    unsigned long req_size;     /* Requested DMA segment size */
    unsigned long blk_size;     /* DMA block size */
    unsigned long blk_order;    /* DMA block size in alternate format */
    unsigned long seg_size;     /* Current DMA segment size */
    unsigned long seg_begin;    /* Logical address of segment */
    unsigned long seg_end;      /* Logical end address of segment */
    unsigned long *blk_ptr;     /* Array of logical DMA block addresses */
    int blk_cnt_max;            /* Maximum number of block to allocate */
    int blk_cnt;                /* Current number of blocks allocated */
} dma_segment_t;

static unsigned int _dma_mem_size = DMA_MEM_DEFAULT;
static mpool_handle_t _dma_pool = NULL;
static void __iomem *_dma_vbase = NULL;
/* cpu physical address for mmap */
static phys_addr_t _cpu_pbase = 0;
/*
 * DMA bus address, it is either identical to cpu physical address
 * or another address(IOVA) translated by IOMMU.
 */
static phys_addr_t _dma_pbase = 0;
static int _use_himem = 0;
static unsigned long _himemaddr = 0;
static int _use_dma_mapping = 0;
static LIST_HEAD(_dma_seg);

#define DMA_DEV_INDEX      0    /* Device index to allocate memory pool */
#define DMA_DEV(n)         lkbde_get_dma_dev(n)
#define BDE_NUM_DEVICES(t) lkbde_get_num_devices(t)

/*
 * Function: _find_largest_segment
 *
 * Purpose:
 *    Find largest contiguous segment from a pool of DMA blocks.
 * Parameters:
 *    dseg - DMA segment descriptor
 * Returns:
 *    0 on success, < 0 on error.
 * Notes:
 *    Assembly stops if a segment of the requested segment size
 *    has been obtained.
 *
 *    Lower address bits of the DMA blocks are used as follows:
 *       0: Untagged
 *       1: Discarded block
 *       2: Part of largest contiguous segment
 *       3: Part of current contiguous segment
 */
static int
_find_largest_segment(dma_segment_t *dseg)
{
    int i, j, blks, found;
    unsigned long b, e, a;

    blks = dseg->blk_cnt;
    /* Clear all block tags */
    for (i = 0; i < blks; i++) {
        dseg->blk_ptr[i] &= ~3;
    }
    for (i = 0; i < blks && dseg->seg_size < dseg->req_size; i++) {
        /* First block must be an untagged block */
        if ((dseg->blk_ptr[i] & 3) == 0) {
            /* Initial segment size is the block size */
            b = dseg->blk_ptr[i];
            e = b + dseg->blk_size;
            dseg->blk_ptr[i] |= 3;
            /* Loop looking for adjacent blocks */
            do {
                found = 0;
                for (j = i + 1; j < blks && (e - b) < dseg->req_size; j++) {
                    a = dseg->blk_ptr[j];
                    /* Check untagged blocks only */
                    if ((a & 3) == 0) {
                        if (a == (b - dseg->blk_size)) {
                            /* Found adjacent block below current segment */
                            dseg->blk_ptr[j] |= 3;
                            b = a;
                            found = 1;
                        } else if (a == e) {
                            /* Found adjacent block above current segment */
                            dseg->blk_ptr[j] |= 3;
                            e += dseg->blk_size;
                            found = 1;
                        }
                    }
                }
            } while (found);
            if ((e - b) > dseg->seg_size) {
                /* The current block is largest so far */
                dseg->seg_begin = b;
                dseg->seg_end = e;
                dseg->seg_size = e - b;
                /* Re-tag current and previous largest segment */
                for (j = 0; j < blks; j++) {
                    if ((dseg->blk_ptr[j] & 3) == 3) {
                        /* Tag current segment as the largest */
                        dseg->blk_ptr[j] &= ~1;
                    } else if ((dseg->blk_ptr[j] & 3) == 2) {
                        /* Discard previous largest segment */
                        dseg->blk_ptr[j] ^= 3;
                    }
                }
            } else {
                /* Discard all blocks in current segment */
                for (j = 0; j < blks; j++) {
                    if ((dseg->blk_ptr[j] & 3) == 3) {
                        dseg->blk_ptr[j] &= ~2;
                    }
                }
            }
        }
    }
    return 0;
}

/*
 * Function: _alloc_dma_blocks
 *
 * Purpose:
 *    Allocate DMA blocks and add them to the pool.
 * Parameters:
 *    dseg - DMA segment descriptor
 *    blks - number of DMA blocks to allocate
 * Returns:
 *    0 on success, < 0 on error.
 * Notes:
 *    DMA blocks are allocated using the page allocator.
 */
static int
_alloc_dma_blocks(dma_segment_t *dseg, int blks)
{
    int i, start;
    unsigned long addr;

    if (dseg->blk_cnt + blks > dseg->blk_cnt_max) {
        gprintk("No more DMA blocks\n");
        return -1;
    }
    start = dseg->blk_cnt;
    for (i = 0; i < blks; i++) {
        /*
         * Note that we cannot use pci_alloc_consistent when we
         * want to be able to map DMA memory to user space.
         *
         * The GFP_DMA flag is omitted as this imposes the ISA
         * addressing limitations on x86 platforms. As long as
         * we have less than 1GB of memory, we can do PCI DMA
         * to all physical RAM locations.
         */
        addr = __get_free_pages(mem_flags, dseg->blk_order);
        if (addr) {
            dseg->blk_ptr[start + i] = addr;
            ++dseg->blk_cnt;
        } else {
            gprintk("DMA allocation failed: allocated %d of %d "
                    "requested blocks\n", i, blks);
            return -1;
        }
    }
    return 0;
}

/*
 * Function: _dma_segment_alloc
 *
 * Purpose:
 *    Allocate large physically contiguous DMA segment.
 * Parameters:
 *    size - requested DMA segment size
 *    blk_size - assemble segment from blocks of this size
 * Returns:
 *    DMA segment descriptor.
 * Notes:
 *    Since we cannot allocate large blocks of contiguous
 *    memory from the kernel, we simply keep allocating
 *    smaller chunks until we can assemble a contiguous
 *    block of the desired size.
 *
 *    When system allowed maximum bytes of memory has been allocated
 *    without a successful assembly of a contiguous DMA
 *    segment, the allocation function will return the
 *    largest contiguous segment found so far. It is up
 *    to the calling function to decide whether this
 *    amount is sufficient to proceed.
 */
static dma_segment_t *
_dma_segment_alloc(size_t size, size_t blk_size)
{
    dma_segment_t *dseg;
    int i, blk_ptr_size;
    unsigned long page_addr;
    struct sysinfo si;

    /* Sanity check */
    if (size == 0 || blk_size == 0) {
        return NULL;
    }
    /* Allocate an initialize DMA segment descriptor */
    if ((dseg = kmalloc(sizeof(dma_segment_t), GFP_KERNEL)) == NULL) {
        return NULL;
    }
    memset(dseg, 0, sizeof(dma_segment_t));
    dseg->req_size = size;
    dseg->blk_size = PAGE_ALIGN(blk_size);
    while ((PAGE_SIZE << dseg->blk_order) < dseg->blk_size) {
        dseg->blk_order++;
    }

    si_meminfo(&si);
    dseg->blk_cnt_max = (si.totalram << PAGE_SHIFT) / dseg->blk_size;
    blk_ptr_size = dseg->blk_cnt_max * sizeof(unsigned long);
    if (blk_ptr_size > KMALLOC_MAX_SIZE) {
        blk_ptr_size = KMALLOC_MAX_SIZE;
        dseg->blk_cnt_max = KMALLOC_MAX_SIZE / sizeof(unsigned long);
    }
    /* Allocate an initialize DMA block pool */
    dseg->blk_ptr = KMALLOC(blk_ptr_size, GFP_KERNEL);
    if (dseg->blk_ptr == NULL) {
        kfree(dseg);
        return NULL;
    }
    memset(dseg->blk_ptr, 0, blk_ptr_size);
    /* Allocate minimum number of blocks */
    if (_alloc_dma_blocks(dseg, dseg->req_size / dseg->blk_size) != 0) {
        gprintk("Failed to allocate minimum number of DMA blocks\n");
        /*
         * _alloc_dma_blocks() returns -1 if it fails to allocate the requested
         * number of blocks, but it may still have allocated something.  Fall
         * through and return dseg filled in with as much memory as we could
         * allocate.
         */
    }
    /* Allocate more blocks until we have a complete segment */
    do {
        _find_largest_segment(dseg);
        if (dseg->seg_size >= dseg->req_size) {
            break;
        }
    } while (_alloc_dma_blocks(dseg, 8) == 0);
    /* Reserve all pages in the DMA segment and free unused blocks */
    for (i = 0; i < dseg->blk_cnt; i++) {
        if ((dseg->blk_ptr[i] & 3) == 2) {
            dseg->blk_ptr[i] &= ~3;
            for (page_addr = dseg->blk_ptr[i];
                 page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                 page_addr += PAGE_SIZE) {
                MEM_MAP_RESERVE(VIRT_TO_PAGE(page_addr));
            }
        } else if (dseg->blk_ptr[i]) {
            dseg->blk_ptr[i] &= ~3;
            free_pages(dseg->blk_ptr[i], dseg->blk_order);
            dseg->blk_ptr[i] = 0;
        }
    }
    return dseg;
}

/*
 * Function: _dma_segment_free
 *
 * Purpose:
 *    Release resources used by DMA segment.
 * Parameters:
 *    dseg - DMA segment descriptor
 * Returns:
 *    Nothing.
 */
static void
_dma_segment_free(dma_segment_t *dseg)
{
    int i;
    unsigned long page_addr;

    if (dseg->blk_ptr) {
        for (i = 0; i < dseg->blk_cnt; i++) {
            if (dseg->blk_ptr[i]) {
                for (page_addr = dseg->blk_ptr[i];
                     page_addr < dseg->blk_ptr[i] + dseg->blk_size;
                     page_addr += PAGE_SIZE) {
                    MEM_MAP_UNRESERVE(VIRT_TO_PAGE(page_addr));
                }
                free_pages(dseg->blk_ptr[i], dseg->blk_order);
            }
        }
        kfree(dseg->blk_ptr);
        kfree(dseg);
    }
}

/*
 * Function: _pgalloc
 *
 * Purpose:
 *    Allocate DMA memory using page allocator
 * Parameters:
 *    size - number of bytes to allocate
 * Returns:
 *    Pointer to allocated DMA memory or NULL if failure.
 * Notes:
 *    For any sizes less than DMA_BLOCK_SIZE, we ask the page
 *    allocator for the entire memory block, otherwise we try
 *    to assemble a contiguous segment ourselves.
 */
static void *
_pgalloc(size_t size)
{
    dma_segment_t *dseg;
    size_t blk_size;

    blk_size = (size < DMA_BLOCK_SIZE) ? size : DMA_BLOCK_SIZE;
    if ((dseg = _dma_segment_alloc(size, blk_size)) == NULL) {
        return NULL;
    }
    if (dseg->seg_size < size) {
        /* If we didn't get the full size then forget it */
        gprintk("_pgalloc() failed to get requested size %zu: "
                "only got %lu contiguous across %d blocks\n",
                size, dseg->seg_size, dseg->blk_cnt);
        _dma_segment_free(dseg);
        return NULL;
    }
    list_add(&dseg->list, &_dma_seg);
    return (void *)dseg->seg_begin;
}

/*
 * Function: _pgfree
 *
 * Purpose:
 *    Free memory allocated by _pgalloc
 * Parameters:
 *    ptr - pointer returned by _pgalloc
 * Returns:
 *    0 if succesfully freed, otherwise -1.
 */
static int
_pgfree(void *ptr)
{
    struct list_head *pos;
    list_for_each(pos, &_dma_seg) {
        dma_segment_t *dseg = list_entry(pos, dma_segment_t, list);
        if (ptr == (void *)dseg->seg_begin) {
            list_del(&dseg->list);
            _dma_segment_free(dseg);
            return 0;
        }
    }
    return -1;
}

/*
 * Function: _pgcleanup
 *
 * Purpose:
 *    Free all memory allocated by _pgalloc
 * Parameters:
 *    None
 * Returns:
 *    Nothing.
 */
static void
_pgcleanup(void)
{
    switch (dmaalloc) {
#if _SIMPLE_MEMORY_ALLOCATION_
      case ALLOC_TYPE_API:
        if (_dma_vbase) {
            if (dma_debug >= 1) gprintk("freeing v=%p p=0x%lx size=0x%lx\n", _dma_vbase,(unsigned long) _dma_pbase, (unsigned long)_dma_mem_size);
            dma_free_coherent(DMA_DEV(DMA_DEV_INDEX), _dma_mem_size, _dma_vbase, _dma_pbase);
        }
        break;
#endif /* _SIMPLE_MEMORY_ALLOCATION_ */

      case ALLOC_TYPE_CHUNK: {
        struct list_head *pos, *tmp;
        int i, ndevices;
        if (_use_dma_mapping) {
            ndevices = BDE_NUM_DEVICES(BDE_SWITCH_DEVICES);
            for (i = 0; i < ndevices && DMA_DEV(i); i ++) {
                dma_unmap_single(DMA_DEV(i), (dma_addr_t)_dma_pbase, _dma_mem_size, DMA_BIDIRECTIONAL);
            }
            _use_dma_mapping = 0;
        }
        list_for_each_safe(pos, tmp, &_dma_seg) {
            dma_segment_t *dseg = list_entry(pos, dma_segment_t, list);
            list_del(&dseg->list);
            _dma_segment_free(dseg);
        }
        break;
      }

      default:
        gprintk("DMA memory allocation method dmaalloc=%d is not supported\n", dmaalloc);
    }
}

/*
 * Function: _alloc_mpool
 *
 * Purpose:
 *    Allocate DMA memory pool
 * Parameters:
 *    size - size of DMA memory pool
 * Returns:
 *    Nothing.
 * Notes:
 *    If set up to use high memory, we simply map the memory into
 *    kernel space.
 *    It is assumed there is only one pool.
 */
static void
_alloc_mpool(size_t size)
{
    unsigned long pbase = 0;
#if defined(__arm__) && !defined(CONFIG_HIGHMEM)
    if (_use_himem) {
        gprintk("DMA in high memory requires CONFIG_HIGHMEM on ARM CPUs.\n");
        return;
    }
#endif

    if (_use_himem) {
        /* Use high memory for DMA */
        if (_himemaddr) {
            pbase = _himemaddr;
        } else {
            pbase = virt_to_bus(high_memory);
        }
        if (((pbase + (size - 1)) >> 16) > DMA_BIT_MASK(16)) {
            gprintk("DMA in high memory at 0x%lx size 0x%lx is beyond the 4GB limit and not supported.\n", pbase, (unsigned long)size);
            return;
        }
        _cpu_pbase = _dma_pbase = pbase;
        _dma_vbase = IOREMAP(_dma_pbase, size);
    } else {
        /* Get DMA memory from kernel */
        if (dma_debug >= 1) {
            gprintk("Allocating DMA memory using method dmaalloc=%d\n", dmaalloc);
        }
        switch (dmaalloc) {
#if _SIMPLE_MEMORY_ALLOCATION_
          case ALLOC_TYPE_API: {
            size_t alloc_size = size; /* size of memory allocated in current iteration */
            if (alloc_size > DMA_MAX_ALLOC_SIZE) {
                alloc_size = DMA_MAX_ALLOC_SIZE;
            }
            /* get a memory allocation from the kernel */
            {
                dma_addr_t dma_handle;
                if (!(_dma_vbase = dma_alloc_coherent(DMA_DEV(DMA_DEV_INDEX),
                        alloc_size, &dma_handle, GFP_KERNEL)) || !dma_handle) {
                    gprintk("Failed to allocate coherent memory pool of size 0x%lx\n", (unsigned long)alloc_size);
                    return;
                }
                _cpu_pbase = pbase = dma_handle;
            }

            if (alloc_size != size) {
                gprintk("allocated 0x%lx bytes instead of 0x%lx bytes.\n",
                        (unsigned long)alloc_size, (unsigned long)size);
            }
            size = _dma_mem_size = alloc_size;
            break;
          }
#endif /* _SIMPLE_MEMORY_ALLOCATION_ */

          case ALLOC_TYPE_CHUNK:
            _dma_vbase = _pgalloc(size);
            if (!_dma_vbase) {
                gprintk("Failed to allocate memory pool of size 0x%lx\n", (unsigned long)size);
                return;
            }
            _cpu_pbase = virt_to_bus(_dma_vbase);
            /* Use dma_map_single to obtain DMA bus address or IOVA if iommu is present. */
            if (DMA_DEV(DMA_DEV_INDEX)) {
                pbase = dma_map_single(DMA_DEV(DMA_DEV_INDEX), _dma_vbase, size, DMA_BIDIRECTIONAL);
                if (DMA_MAPPING_ERROR(DMA_DEV(DMA_DEV_INDEX), pbase)) {
                    gprintk("Failed to map memory at %p\n", _dma_vbase);
                    _pgcleanup();
                    _dma_vbase = NULL;
                    return;
                }
                _use_dma_mapping = 1;
            } else {
                pbase = _cpu_pbase;
            }
            break;
          default:
            _dma_vbase = NULL;
            gprintk("DMA memory allocation method dmaalloc=%d is not supported\n", dmaalloc);
            return;
        }

        if (((pbase + (size - 1)) >> 16) > DMA_BIT_MASK(16)) {
            gprintk("DMA memory allocated at 0x%lx size 0x%lx is beyond the 4GB limit and not supported.\n", pbase, (unsigned long)size);
            _pgcleanup();
            _dma_vbase = NULL;
            _dma_pbase = 0;
            return;
        }

        _dma_pbase = pbase;
#ifdef REMAP_DMA_NONCACHED
        _dma_vbase = IOREMAP(_dma_pbase, size);
#endif
        if (dma_debug >= 1) {
            gprintk("_use_dma_mapping:%d _dma_vbase:%p _dma_pbase:%lx _cpu_pbase:%lx allocated:%lx dmaalloc:%d\n",
                     _use_dma_mapping, _dma_vbase, (unsigned long)_dma_pbase,
                     (unsigned long)_cpu_pbase, (unsigned long)size, dmaalloc);
        }
    }
}

/*
 * Function: _dma_cleanup
 *
 * Purpose:
 *    DMA cleanup function.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
int
_dma_cleanup(void)
{
    if (_dma_vbase) {
        mpool_destroy(_dma_pool);
        if (_use_himem) {
            iounmap(_dma_vbase);
        } else {
#ifdef REMAP_DMA_NONCACHED
            iounmap(_dma_vbase);
#endif
            _pgcleanup();
        }
        _dma_vbase = NULL;
        _dma_pbase = 0;
        _cpu_pbase = 0;
    }
    return 0;
}

void _dma_init(int dev_index)
{
    unsigned long pbase;

    if (dev_index > DMA_DEV_INDEX) {
        if (_use_dma_mapping && DMA_DEV(dev_index) && _dma_vbase) {
            pbase = dma_map_single(DMA_DEV(dev_index), _dma_vbase, _dma_mem_size, DMA_BIDIRECTIONAL);
            if (DMA_MAPPING_ERROR(DMA_DEV(dev_index), pbase)) {
                gprintk("Failed to map memory for device %d at %p\n", dev_index, _dma_vbase);
                return;
            }
            if (pbase != (unsigned long)_dma_pbase) {
                /* Bus address/IOVA must be identical for all devices. */
                gprintk("Device %d has different pbase: %lx (should be %lx)\n",
                        dev_index, pbase, (unsigned long)_dma_pbase);
            }
        }
        return;
    }

    /* DMA Setup */
    if (dmasize) {
        if ((dmasize[strlen(dmasize)-1] & ~0x20) == 'M') {
            _dma_mem_size = simple_strtoul(dmasize, NULL, 0);
            _dma_mem_size *= ONE_MB;
        } else {
            gprintk("DMA memory size must be specified as e.g. dmasize=8M\n");
        }
        if (_dma_mem_size & (_dma_mem_size-1)) {
            gprintk("dmasize must be a power of 2 (1M, 2M, 4M, 8M etc.)\n");
            _dma_mem_size = 0;
        }
    }

    if (himem) {
        if ((himem[0] & ~0x20) == 'Y' || himem[0] == '1') {
            _use_himem = 1;
        } else if ((himem[0] & ~0x20) == 'N' || himem[0] == '0') {
            _use_himem = 0;
        }
    }

    if (himemaddr && strlen(himemaddr) > 0) {
        char suffix = (himemaddr[strlen(himemaddr)-1] & ~0x20);
        _himemaddr = simple_strtoul(himemaddr, NULL, 0);
        if (suffix == 'M') {
            _himemaddr *= ONE_MB;
        } else if (suffix == 'G') {
            _himemaddr *= ONE_GB;
        } else {
            gprintk("DMA high memory address must be specified as e.g. himemaddr=8[MG]\n");
        }
    }

    if (_dma_mem_size) {
        _alloc_mpool(_dma_mem_size);
        if (_dma_vbase == NULL) {
            gprintk("no DMA memory available\n");
        } else {
            mpool_init();
            _dma_pool = mpool_create(_dma_vbase, _dma_mem_size);
        }
    }
}

/*
 * Some kernels are configured to prevent mapping of kernel RAM memory
 * into user space via the /dev/mem device.
 *
 * The function below provides a backdoor to mapping the DMA pool to
 * user space via the BDE device file.
 */
int _dma_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long phys_addr = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long size = vma->vm_end - vma->vm_start;

    if (phys_addr < (unsigned long )_cpu_pbase ||
        (phys_addr + size) > ((unsigned long )_cpu_pbase + _dma_mem_size)) {
        gprintk("range 0x%lx-0x%lx outside DMA pool 0x%lx-0x%lx\n",
                phys_addr, phys_addr + size, (unsigned long )_cpu_pbase,
                (unsigned long )_cpu_pbase + _dma_mem_size);
        return -EINVAL;
    }

#ifdef USE_DMA_MMAP_COHERENT
    if (dmaalloc == ALLOC_TYPE_API) {
        vma->vm_pgoff = 0;
        return dma_mmap_coherent(DMA_DEV(DMA_DEV_INDEX), vma, (void *)_dma_vbase, phys_addr, size);
    }
#endif

    _PGPROT_NONCACHED(vma->vm_page_prot);

    if (remap_pfn_range(vma,
                        vma->vm_start,
                        vma->vm_pgoff,
                        size,
                        vma->vm_page_prot)) {
        gprintk("Failed to mmap phys range 0x%lx-0x%lx to 0x%lx-0x%lx\n",
                phys_addr, phys_addr + size, vma->vm_start,vma->vm_end);
        return -EAGAIN;
    }
    return 0;
}

/*
 * Function: _dma_pool_allocated
 *
 * Purpose:
 *    Check if DMA pool has been allocated.
 * Parameters:
 *    None
 * Returns:
 *    0 : not allocated
 *    1 : allocated
 */
int
_dma_pool_allocated(void)
{
    return (_dma_vbase) ? 1 : 0;
}

sal_paddr_t
_l2p(int d, void *vaddr)
{
    if (_dma_mem_size) {
        /* dma memory is a contiguous block */
        if (vaddr) {
            return _dma_pbase + (PTR_TO_UINTPTR(vaddr) - PTR_TO_UINTPTR(_dma_vbase));
        }
        return 0;
    }
    return ((sal_paddr_t)virt_to_bus(vaddr));
}

void *
_p2l(int d, sal_paddr_t paddr)
{
    sal_vaddr_t vaddr = (sal_vaddr_t)_dma_vbase;

    if (_dma_mem_size) {
        /* DMA memory is a contiguous block */
        if (paddr == 0) {
            return NULL;
        }
        return (void *)(vaddr + (sal_vaddr_t)(paddr - _dma_pbase));
    }
    return bus_to_virt(paddr);
}

/*
 * Some of the driver malloc's are too large for
 * kmalloc(), so 'sal_alloc' and 'sal_free' in the
 * linux kernel sal cannot be implemented with kmalloc().
 *
 * Instead, they expect someone to provide an allocator
 * that can handle the gimongous size of some of the
 * allocations, and we provide it here, by allocating
 * this memory out of the boot-time dma pool.
 *
 * These are the functions in question:
 */

void* kmalloc_giant(int sz)
{
    return mpool_alloc(_dma_pool, sz);
}

void kfree_giant(void* ptr)
{
    return mpool_free(_dma_pool, ptr);
}

uint32_t *
_salloc(int d, int size, const char *name)
{
    void *ptr;

    if (_dma_mem_size) {
        return mpool_alloc(_dma_pool, size);
    }
    if ((ptr = kmalloc(size, mem_flags)) == NULL) {
        ptr = _pgalloc(size);
    }
    return ptr;
}

void
_sfree(int d, void *ptr)
{
    if (_dma_mem_size) {
        return mpool_free(_dma_pool, ptr);
    }
    if (_pgfree(ptr) < 0) {
        kfree(ptr);
    }
}

int
_sinval(int d, void *ptr, int length)
{
#if defined(dma_cache_wback_inv)
     dma_cache_wback_inv((unsigned long)ptr, length);
#else
#if defined(IPROC_CMICD) || defined(BCM958525)
    
    dma_sync_single_for_cpu(NULL, (unsigned long)ptr, length, DMA_BIDIRECTIONAL);
#else
    dma_cache_sync(NULL, ptr, length, DMA_BIDIRECTIONAL);
#endif
#endif
    return 0;
}

int
_sflush(int d, void *ptr, int length)
{
#if defined(dma_cache_wback_inv)
    dma_cache_wback_inv((unsigned long)ptr, length);
#else
#if defined(IPROC_CMICD) || defined(BCM958525)
    
    dma_sync_single_for_cpu(NULL, (unsigned long)ptr, length, DMA_BIDIRECTIONAL);
#else
    dma_cache_sync(NULL, ptr, length, DMA_BIDIRECTIONAL);
#endif
#endif

    return 0;
}

int
lkbde_get_dma_info(phys_addr_t* cpu_pbase, phys_addr_t* dma_pbase, ssize_t* size)
{
    if (_dma_vbase == NULL) {
        if (_dma_mem_size == 0) {
            _dma_mem_size = DMA_MEM_DEFAULT;
        }
        _alloc_mpool(_dma_mem_size);
    }
    *cpu_pbase = _cpu_pbase;
    *dma_pbase = _dma_pbase;
    *size = (_dma_vbase) ? _dma_mem_size : 0;
    return 0;
}

void
_dma_pprint(void)
{
    pprintf("\tdmasize=%s\n", dmasize);
    pprintf("\thimem=%s\n", himem);
    pprintf("\thimemaddr=%s\n", himemaddr);
    pprintf("DMA Memory (%s): %d bytes, %d used, %d free%s\n",
            (_use_himem) ? "high" : "kernel",
            (_dma_vbase) ? _dma_mem_size : 0,
            (_dma_vbase) ? mpool_usage(_dma_pool) : 0,
            (_dma_vbase) ? _dma_mem_size - mpool_usage(_dma_pool) : 0,
            USE_LINUX_BDE_MMAP ? ", local mmap" : "");
}

/*
 * Export functions
 */
LKM_EXPORT_SYM(kmalloc_giant);
LKM_EXPORT_SYM(kfree_giant);
LKM_EXPORT_SYM(lkbde_get_dma_info);
