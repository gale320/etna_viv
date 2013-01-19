/*
 * Copyright (c) 2012-2013 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/* Rotating, animated cube 
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>

#include <errno.h>

#include "etna/state.xml.h"
#include "etna/cmdstream.xml.h"
#include "write_bmp.h"
#include "viv.h"
#include "etna.h"
#include "etna_state.h"
#include "etna_rs.h"
#include "etna_fb.h"

#include "esUtil.h"

float vVertices[] = {
  // front
  -1.0f, -1.0f, +1.0f, // point blue
  +1.0f, -1.0f, +1.0f, // point magenta
  -1.0f, +1.0f, +1.0f, // point cyan
  +1.0f, +1.0f, +1.0f, // point white
  // back
  +1.0f, -1.0f, -1.0f, // point red
  -1.0f, -1.0f, -1.0f, // point black
  +1.0f, +1.0f, -1.0f, // point yellow
  -1.0f, +1.0f, -1.0f, // point green
  // right
  +1.0f, -1.0f, +1.0f, // point magenta
  +1.0f, -1.0f, -1.0f, // point red
  +1.0f, +1.0f, +1.0f, // point white
  +1.0f, +1.0f, -1.0f, // point yellow
  // left
  -1.0f, -1.0f, -1.0f, // point black
  -1.0f, -1.0f, +1.0f, // point blue
  -1.0f, +1.0f, -1.0f, // point green
  -1.0f, +1.0f, +1.0f, // point cyan
  // top
  -1.0f, +1.0f, +1.0f, // point cyan
  +1.0f, +1.0f, +1.0f, // point white
  -1.0f, +1.0f, -1.0f, // point green
  +1.0f, +1.0f, -1.0f, // point yellow
  // bottom
  -1.0f, -1.0f, -1.0f, // point black
  +1.0f, -1.0f, -1.0f, // point red
  -1.0f, -1.0f, +1.0f, // point blue
  +1.0f, -1.0f, +1.0f  // point magenta
};

float vColors[] = {
  // front
  0.0f,  0.0f,  1.0f, // blue
  1.0f,  0.0f,  1.0f, // magenta
  0.0f,  1.0f,  1.0f, // cyan
  1.0f,  1.0f,  1.0f, // white
  // back
  1.0f,  0.0f,  0.0f, // red
  0.0f,  0.0f,  0.0f, // black
  1.0f,  1.0f,  0.0f, // yellow
  0.0f,  1.0f,  0.0f, // green
  // right
  1.0f,  0.0f,  1.0f, // magenta
  1.0f,  0.0f,  0.0f, // red
  1.0f,  1.0f,  1.0f, // white
  1.0f,  1.0f,  0.0f, // yellow
  // left
  0.0f,  0.0f,  0.0f, // black
  0.0f,  0.0f,  1.0f, // blue
  0.0f,  1.0f,  0.0f, // green
  0.0f,  1.0f,  1.0f, // cyan
  // top
  0.0f,  1.0f,  1.0f, // cyan
  1.0f,  1.0f,  1.0f, // white
  0.0f,  1.0f,  0.0f, // green
  1.0f,  1.0f,  0.0f, // yellow
  // bottom
  0.0f,  0.0f,  0.0f, // black
  1.0f,  0.0f,  0.0f, // red
  0.0f,  0.0f,  1.0f, // blue
  1.0f,  0.0f,  1.0f  // magenta
};

float vNormals[] = {
  // front
  +0.0f, +0.0f, +1.0f, // forward
  +0.0f, +0.0f, +1.0f, // forward
  +0.0f, +0.0f, +1.0f, // forward
  +0.0f, +0.0f, +1.0f, // forward
  // back
  +0.0f, +0.0f, -1.0f, // backbard
  +0.0f, +0.0f, -1.0f, // backbard
  +0.0f, +0.0f, -1.0f, // backbard
  +0.0f, +0.0f, -1.0f, // backbard
  // right
  +1.0f, +0.0f, +0.0f, // right
  +1.0f, +0.0f, +0.0f, // right
  +1.0f, +0.0f, +0.0f, // right
  +1.0f, +0.0f, +0.0f, // right
  // left
  -1.0f, +0.0f, +0.0f, // left
  -1.0f, +0.0f, +0.0f, // left
  -1.0f, +0.0f, +0.0f, // left
  -1.0f, +0.0f, +0.0f, // left
  // top
  +0.0f, +1.0f, +0.0f, // up
  +0.0f, +1.0f, +0.0f, // up
  +0.0f, +1.0f, +0.0f, // up
  +0.0f, +1.0f, +0.0f, // up
  // bottom
  +0.0f, -1.0f, +0.0f, // down
  +0.0f, -1.0f, +0.0f, // down
  +0.0f, -1.0f, +0.0f, // down
  +0.0f, -1.0f, +0.0f  // down
};
#define COMPONENTS_PER_VERTEX (3)
#define NUM_VERTICES (6*4)

uint32_t vs[] = {
    0x01831009, 0x00000000, 0x00000000, 0x203fc048,
    0x02031009, 0x00000000, 0x00000000, 0x203fc058,
    0x07841003, 0x39000800, 0x00000050, 0x00000000,
    0x07841002, 0x39001800, 0x00aa0050, 0x00390048,
    0x07841002, 0x39002800, 0x01540050, 0x00390048,
    0x07841002, 0x39003800, 0x01fe0050, 0x00390048,
    0x03851003, 0x29004800, 0x000000d0, 0x00000000,
    0x03851002, 0x29005800, 0x00aa00d0, 0x00290058,
    0x03811002, 0x29006800, 0x015400d0, 0x00290058,
    0x07851003, 0x39007800, 0x00000050, 0x00000000,
    0x07851002, 0x39008800, 0x00aa0050, 0x00390058,
    0x07851002, 0x39009800, 0x01540050, 0x00390058,
    0x07801002, 0x3900a800, 0x01fe0050, 0x00390058,
    0x0401100c, 0x00000000, 0x00000000, 0x003fc008,
    0x03801002, 0x69000800, 0x01fe00c0, 0x00290038,
    0x03831005, 0x29000800, 0x01480040, 0x00000000,
    0x0383100d, 0x00000000, 0x00000000, 0x00000038,
    0x03801003, 0x29000800, 0x014801c0, 0x00000000,
    0x00801005, 0x29001800, 0x01480040, 0x00000000,
    0x0080108f, 0x3fc06800, 0x00000050, 0x203fc068,
    0x03801003, 0x00000800, 0x01480140, 0x00000000,
    0x04001009, 0x00000000, 0x00000000, 0x200000b8,
    0x02041001, 0x2a804800, 0x00000000, 0x003fc048,
    0x02041003, 0x2a804800, 0x00aa05c0, 0x00000002,
};
uint32_t ps[] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};
size_t vs_size = sizeof(vs);
size_t ps_size = sizeof(ps);

int main(int argc, char **argv)
{
    int rv;
    int width = 256;
    int height = 256;
    int padded_width, padded_height;
    int backbuffer = 0;
    
    fb_info fb;
    rv = fb_open(0, &fb);
    if(rv!=0)
    {
        exit(1);
    }
    width = fb.fb_var.xres;
    height = fb.fb_var.yres;
    padded_width = etna_align_up(width, 64);
    padded_height = etna_align_up(height, 64);

    printf("padded_width %i padded_height %i\n", padded_width, padded_height);
    rv = viv_open();
    if(rv!=0)
    {
        fprintf(stderr, "Error opening device\n");
        exit(1);
    }
    printf("Succesfully opened device\n");

    /* allocate main render target */
    gcuVIDMEM_NODE_PTR rt_node = 0;
    if(viv_alloc_linear_vidmem(padded_width * padded_height * 4, 0x40, gcvSURF_RENDER_TARGET, gcvPOOL_DEFAULT, &rt_node)!=0)
    {
        fprintf(stderr, "Error allocating render target buffer memory\n");
        exit(1);
    }
    printf("Allocated render target node: node=%08x\n", (uint32_t)rt_node);

    viv_addr_t rt_physical = 0; /* ADDR_A */
    void *rt_logical = 0;
    if(viv_lock_vidmem(rt_node, &rt_physical, &rt_logical)!=0)
    {
        fprintf(stderr, "Error locking render target memory\n");
        exit(1);
    }
    printf("Locked render target: phys=%08x log=%08x\n", (uint32_t)rt_physical, (uint32_t)rt_logical);
    memset(rt_logical, 0xff, padded_width * padded_height * 4); /* clear previous result just in case, test that clearing works */

    /* allocate tile status for main render target */
    gcuVIDMEM_NODE_PTR rt_ts_node = 0;
    uint32_t rt_ts_size = etna_align_up((padded_width * padded_height * 4)/0x100, 0x100);
    if(viv_alloc_linear_vidmem(rt_ts_size, 0x40, gcvSURF_TILE_STATUS, gcvPOOL_DEFAULT, &rt_ts_node)!=0)
    {
        fprintf(stderr, "Error allocating render target tile status memory\n");
        exit(1);
    }
    printf("Allocated render target tile status node: node=%08x size=%08x\n", (uint32_t)rt_ts_node, rt_ts_size);

    viv_addr_t rt_ts_physical = 0; /* ADDR_B */
    void *rt_ts_logical = 0;
    if(viv_lock_vidmem(rt_ts_node, &rt_ts_physical, &rt_ts_logical)!=0)
    {
        fprintf(stderr, "Error locking render target memory\n");
        exit(1);
    }
    printf("Locked render target ts: phys=%08x log=%08x\n", (uint32_t)rt_ts_physical, (uint32_t)rt_ts_logical);

    /* allocate depth for main render target */
    gcuVIDMEM_NODE_PTR z_node = 0;
    if(viv_alloc_linear_vidmem(padded_width * padded_height * 2, 0x40, gcvSURF_DEPTH, gcvPOOL_DEFAULT, &z_node)!=0)
    {
        fprintf(stderr, "Error allocating depth memory\n");
        exit(1);
    }
    printf("Allocated depth node: node=%08x\n", (uint32_t)z_node);

    viv_addr_t z_physical = 0; /* ADDR_C */
    void *z_logical = 0;
    if(viv_lock_vidmem(z_node, &z_physical, &z_logical)!=0)
    {
        fprintf(stderr, "Error locking depth target memory\n");
        exit(1);
    }
    printf("Locked depth target: phys=%08x log=%08x\n", (uint32_t)z_physical, (uint32_t)z_logical);

    /* allocate depth ts for main render target */
    gcuVIDMEM_NODE_PTR z_ts_node = 0;
    uint32_t z_ts_size = etna_align_up((padded_width * padded_height * 2)/0x100, 0x100);
    if(viv_alloc_linear_vidmem(z_ts_size, 0x40, gcvSURF_TILE_STATUS, gcvPOOL_DEFAULT, &z_ts_node)!=0)
    {
        fprintf(stderr, "Error allocating depth memory\n");
        exit(1);
    }
    printf("Allocated depth ts node: node=%08x size=%08x\n", (uint32_t)z_ts_node, z_ts_size);

    viv_addr_t z_ts_physical = 0; /* ADDR_D */
    void *z_ts_logical = 0;
    if(viv_lock_vidmem(z_ts_node, &z_ts_physical, &z_ts_logical)!=0)
    {
        fprintf(stderr, "Error locking depth target ts memory\n");
        exit(1);
    }
    printf("Locked depth ts target: phys=%08x log=%08x\n", (uint32_t)z_ts_physical, (uint32_t)z_ts_logical);

    /* allocate vertex buffer */
    gcuVIDMEM_NODE_PTR vtx_node = 0;
    if(viv_alloc_linear_vidmem(0x60000, 0x40, gcvSURF_VERTEX, gcvPOOL_DEFAULT, &vtx_node)!=0)
    {
        fprintf(stderr, "Error allocating vertex memory\n");
        exit(1);
    }
    printf("Allocated vertex node: node=%08x\n", (uint32_t)vtx_node);

    viv_addr_t vtx_physical = 0; /* ADDR_E */
    void *vtx_logical = 0;
    if(viv_lock_vidmem(vtx_node, &vtx_physical, &vtx_logical)!=0)
    {
        fprintf(stderr, "Error locking vertex memory\n");
        exit(1);
    }
    printf("Locked vertex memory: phys=%08x log=%08x\n", (uint32_t)vtx_physical, (uint32_t)vtx_logical);

    /* allocate aux render target */
    gcuVIDMEM_NODE_PTR aux_rt_node = 0;
    if(viv_alloc_linear_vidmem(0x4000, 0x40, gcvSURF_RENDER_TARGET, gcvPOOL_SYSTEM /*why?*/, &aux_rt_node)!=0)
    {
        fprintf(stderr, "Error allocating aux render target buffer memory\n");
        exit(1);
    }
    printf("Allocated aux render target node: node=%08x\n", (uint32_t)aux_rt_node);

    viv_addr_t aux_rt_physical = 0; /* ADDR_F */
    void *aux_rt_logical = 0;
    if(viv_lock_vidmem(aux_rt_node, &aux_rt_physical, &aux_rt_logical)!=0)
    {
        fprintf(stderr, "Error locking aux render target memory\n");
        exit(1);
    }
    printf("Locked aux render target: phys=%08x log=%08x\n", (uint32_t)aux_rt_physical, (uint32_t)aux_rt_logical);

    /* allocate tile status for aux render target */
    gcuVIDMEM_NODE_PTR aux_rt_ts_node = 0;
    if(viv_alloc_linear_vidmem(0x100, 0x40, gcvSURF_TILE_STATUS, gcvPOOL_DEFAULT, &aux_rt_ts_node)!=0)
    {
        fprintf(stderr, "Error allocating aux render target tile status memory\n");
        exit(1);
    }
    printf("Allocated aux render target tile status node: node=%08x\n", (uint32_t)aux_rt_ts_node);

    viv_addr_t aux_rt_ts_physical = 0; /* ADDR_G */
    void *aux_rt_ts_logical = 0;
    if(viv_lock_vidmem(aux_rt_ts_node, &aux_rt_ts_physical, &aux_rt_ts_logical)!=0)
    {
        fprintf(stderr, "Error locking aux ts render target memory\n");
        exit(1);
    }
    printf("Locked aux render target ts: phys=%08x log=%08x\n", (uint32_t)aux_rt_ts_physical, (uint32_t)aux_rt_ts_logical);

    /* Allocate video memory for BITMAP, lock */
    gcuVIDMEM_NODE_PTR bmp_node = 0;
    if(viv_alloc_linear_vidmem(width*height*4, 0x40, gcvSURF_BITMAP, gcvPOOL_DEFAULT, &bmp_node)!=0)
    {
        fprintf(stderr, "Error allocating bitmap status memory\n");
        exit(1);
    }
    printf("Allocated bitmap node: node=%08x\n", (uint32_t)bmp_node);

    viv_addr_t bmp_physical = 0;
    void *bmp_logical = 0;
    if(viv_lock_vidmem(bmp_node, &bmp_physical, &bmp_logical)!=0)
    {
        fprintf(stderr, "Error locking bmp memory\n");
        exit(1);
    }
    memset(bmp_logical, 0xff, width*height*4); /* clear previous result */
    printf("Locked bmp: phys=%08x log=%08x\n", (uint32_t)bmp_physical, (uint32_t)bmp_logical);

    /* Phew, now we got all the memory we need.
     * Write interleaved attribute vertex stream.
     * Unlike the GL example we only do this once, not every time glDrawArrays is called, the same would be accomplished
     * from GL by using a vertex buffer object.
     */
    for(int vert=0; vert<NUM_VERTICES; ++vert)
    {
        int src_idx = vert * COMPONENTS_PER_VERTEX;
        int dest_idx = vert * COMPONENTS_PER_VERTEX * 3;
        for(int comp=0; comp<COMPONENTS_PER_VERTEX; ++comp)
        {
            ((float*)vtx_logical)[dest_idx+comp+0] = vVertices[src_idx + comp]; /* 0 */
            ((float*)vtx_logical)[dest_idx+comp+3] = vNormals[src_idx + comp]; /* 1 */
            ((float*)vtx_logical)[dest_idx+comp+6] = vColors[src_idx + comp]; /* 2 */
        }
    }

    etna_ctx *ctx = etna_create();

    /* XXX how important is the ordering? I suppose we could group states (except the flushes, kickers, semaphores etc)
     * and simply submit them at once. Especially for consecutive states and masked stated this could be a big win
     * in DMA command buffer size. */

    for(int frame=0; frame<1000; ++frame)
    {
        printf("*** FRAME %i ****\n", frame);
        /* XXX part of this can be put outside the loop, but until we have usable context management
         * this is safest.
         */

        etna_set_state(ctx, VIVS_GL_VERTEX_ELEMENT_CONFIG, 0x1);
        etna_set_state(ctx, VIVS_RA_CONTROL, 0x1);

        etna_set_state(ctx, VIVS_PA_W_CLIP_LIMIT, 0x34000001);
        etna_set_state(ctx, VIVS_PA_SYSTEM_MODE, 0x11);
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_BIT(VIVS_PA_CONFIG_UNK22, 0));
        etna_set_state(ctx, VIVS_SE_LAST_PIXEL_ENABLE, 0x0);
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_PE_COLOR_FORMAT, 
                ETNA_MASKED_BIT(VIVS_PE_COLOR_FORMAT_PARTIAL, 0));
        etna_set_state(ctx, VIVS_PE_ALPHA_CONFIG, /* can & all these together */
                ETNA_MASKED_BIT(VIVS_PE_ALPHA_CONFIG_BLEND_ENABLE_COLOR, 0) &
                ETNA_MASKED_BIT(VIVS_PE_ALPHA_CONFIG_BLEND_ENABLE_ALPHA, 0));
        etna_set_state(ctx, VIVS_PE_ALPHA_CONFIG,
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_SRC_FUNC_COLOR, BLEND_FUNC_ONE) &
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_SRC_FUNC_ALPHA, BLEND_FUNC_ONE));
        etna_set_state(ctx, VIVS_PE_ALPHA_CONFIG,
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_DST_FUNC_COLOR, BLEND_FUNC_ZERO) &
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_DST_FUNC_ALPHA, BLEND_FUNC_ZERO));
        etna_set_state(ctx, VIVS_PE_ALPHA_CONFIG,
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_EQ_COLOR, BLEND_EQ_ADD) &
                ETNA_MASKED(VIVS_PE_ALPHA_CONFIG_EQ_ALPHA, BLEND_EQ_ADD));
        etna_set_state(ctx, VIVS_PE_ALPHA_BLEND_COLOR, 
                VIVS_PE_ALPHA_BLEND_COLOR_B(0) | 
                VIVS_PE_ALPHA_BLEND_COLOR_G(0) | 
                VIVS_PE_ALPHA_BLEND_COLOR_R(0) | 
                VIVS_PE_ALPHA_BLEND_COLOR_A(0));
        etna_set_state(ctx, VIVS_PE_ALPHA_OP, ETNA_MASKED_BIT(VIVS_PE_ALPHA_OP_ALPHA_TEST, 0));
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_INL(VIVS_PA_CONFIG_CULL_FACE_MODE, OFF));
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_WRITE_ENABLE, 0));
        etna_set_state(ctx, VIVS_PE_STENCIL_CONFIG, ETNA_MASKED(VIVS_PE_STENCIL_CONFIG_REF_FRONT, 0) &
                                                    ETNA_MASKED(VIVS_PE_STENCIL_CONFIG_MASK_FRONT, 0xff) & 
                                                    ETNA_MASKED(VIVS_PE_STENCIL_CONFIG_WRITE_MASK, 0xff));
        etna_set_state(ctx, VIVS_PE_STENCIL_OP, ETNA_MASKED(VIVS_PE_STENCIL_OP_FUNC_FRONT, COMPARE_FUNC_ALWAYS) &
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_FUNC_BACK, COMPARE_FUNC_ALWAYS) &
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_FAIL_FRONT, STENCIL_OP_KEEP) & 
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_FAIL_BACK, STENCIL_OP_KEEP) & 
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_DEPTH_FAIL_FRONT, STENCIL_OP_KEEP) & 
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_DEPTH_FAIL_BACK, STENCIL_OP_KEEP) &
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_PASS_FRONT, STENCIL_OP_KEEP) &
                                                ETNA_MASKED(VIVS_PE_STENCIL_OP_PASS_BACK, STENCIL_OP_KEEP));

        etna_set_state(ctx, VIVS_PE_COLOR_FORMAT, ETNA_MASKED(VIVS_PE_COLOR_FORMAT_COMPONENTS, 0xf));

        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_EARLY_Z, 0));
        etna_set_state(ctx, VIVS_SE_DEPTH_SCALE, 0x0);
        etna_set_state(ctx, VIVS_SE_DEPTH_BIAS, 0x0);
        
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_INL(VIVS_PA_CONFIG_FILL_MODE, SOLID));
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_INL(VIVS_PA_CONFIG_SHADE_MODEL, SMOOTH));
        etna_set_state(ctx, VIVS_PE_COLOR_FORMAT, 
                ETNA_MASKED(VIVS_PE_COLOR_FORMAT_FORMAT, RS_FORMAT_X8R8G8B8) &
                ETNA_MASKED_BIT(VIVS_PE_COLOR_FORMAT_SUPER_TILED, 1));

        etna_set_state(ctx, VIVS_PE_COLOR_ADDR, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_PE_COLOR_STRIDE, padded_width * 4); 
        etna_set_state(ctx, VIVS_GL_MULTI_SAMPLE_CONFIG, 
                ETNA_MASKED(VIVS_GL_MULTI_SAMPLE_CONFIG_UNK0, 0x0) &
                ETNA_MASKED(VIVS_GL_MULTI_SAMPLE_CONFIG_UNK4, 0xf) &
                ETNA_MASKED(VIVS_GL_MULTI_SAMPLE_CONFIG_UNK12, 0x0) &
                ETNA_MASKED(VIVS_GL_MULTI_SAMPLE_CONFIG_UNK16, 0x0)
                ); 
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_PE_COLOR_FORMAT, ETNA_MASKED_BIT(VIVS_PE_COLOR_FORMAT_PARTIAL, 1));
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_TS_COLOR_CLEAR_VALUE, 0);
        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_TS_MEM_CONFIG, VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR);

        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, 
                ETNA_MASKED_INL(VIVS_PE_DEPTH_CONFIG_DEPTH_FORMAT, D16) &
                ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_SUPER_TILED, 1)
                );
        etna_set_state(ctx, VIVS_PE_DEPTH_ADDR, z_physical); /* ADDR_C */
        etna_set_state(ctx, VIVS_PE_DEPTH_STRIDE, padded_width * 2);
        etna_set_state(ctx, VIVS_PE_STENCIL_CONFIG, ETNA_MASKED_INL(VIVS_PE_STENCIL_CONFIG_MODE, DISABLED));
        etna_set_state(ctx, VIVS_PE_HDEPTH_CONTROL, VIVS_PE_HDEPTH_CONTROL_FORMAT_DISABLED);
        etna_set_state_f32(ctx, VIVS_PE_DEPTH_NORMALIZE, 65535.0);
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_EARLY_Z, 0));
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_DEPTH);

        etna_set_state(ctx, VIVS_TS_DEPTH_CLEAR_VALUE, 0xffffffff);
        etna_set_state(ctx, VIVS_TS_DEPTH_STATUS_BASE, z_ts_physical); /* ADDR_D */
        etna_set_state(ctx, VIVS_TS_DEPTH_SURFACE_BASE, z_physical); /* ADDR_C */
        etna_set_state(ctx, VIVS_TS_MEM_CONFIG, 
                VIVS_TS_MEM_CONFIG_DEPTH_FAST_CLEAR |
                VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR |
                VIVS_TS_MEM_CONFIG_DEPTH_16BPP | 
                VIVS_TS_MEM_CONFIG_DEPTH_COMPRESSION);
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_EARLY_Z, 1)); /* flip-flopping once again */

        /* Warm up RS on aux render target */
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
        etna_warm_up_rs(ctx, aux_rt_physical, aux_rt_ts_physical);

        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */

        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
        etna_warm_up_rs(ctx, aux_rt_physical, aux_rt_ts_physical);

        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
       
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
        etna_warm_up_rs(ctx, aux_rt_physical, aux_rt_ts_physical);
        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */
        
        etna_stall(ctx, SYNC_RECIPIENT_RA, SYNC_RECIPIENT_PE);

        /* Set up the resolve to clear tile status for main render target 
         * Regard the TS as an image of width 16 with 4 bytes per pixel (64 bytes per row)
         * XXX need to clear the depth ts too.
         * */
        etna_set_state(ctx, VIVS_RS_CONFIG,
                (RS_FORMAT_A8R8G8B8 << VIVS_RS_CONFIG_SOURCE_FORMAT__SHIFT) |
                (RS_FORMAT_A8R8G8B8 << VIVS_RS_CONFIG_DEST_FORMAT__SHIFT)
                );
        etna_set_state_multi(ctx, VIVS_RS_DITHER(0), 2, (uint32_t[]){0xffffffff, 0xffffffff});
        etna_set_state(ctx, VIVS_RS_DEST_ADDR, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_RS_DEST_STRIDE, 0x40);
        etna_set_state(ctx, VIVS_RS_WINDOW_SIZE, 
                ((rt_ts_size/0x40) << VIVS_RS_WINDOW_SIZE_HEIGHT__SHIFT) |
                (16 << VIVS_RS_WINDOW_SIZE_WIDTH__SHIFT));
        etna_set_state(ctx, VIVS_RS_FILL_VALUE(0), 0x55555555);
        etna_set_state(ctx, VIVS_RS_CLEAR_CONTROL, 
                VIVS_RS_CLEAR_CONTROL_MODE_ENABLED |
                (0xffff << VIVS_RS_CLEAR_CONTROL_BITS__SHIFT));
        etna_set_state(ctx, VIVS_RS_EXTRA_CONFIG, 
                0); /* no AA, no endian switch */
        etna_set_state(ctx, VIVS_RS_KICKER, 
                0xbeebbeeb);
        /** Done */
        
        etna_set_state(ctx, VIVS_TS_COLOR_CLEAR_VALUE, 0xff7f7f7f);
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_TS_COLOR_CLEAR_VALUE, 0xff7f7f7f);
        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_TS_MEM_CONFIG, 
                VIVS_TS_MEM_CONFIG_DEPTH_FAST_CLEAR |
                VIVS_TS_MEM_CONFIG_COLOR_FAST_CLEAR |
                VIVS_TS_MEM_CONFIG_DEPTH_16BPP | 
                VIVS_TS_MEM_CONFIG_DEPTH_COMPRESSION);
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_INL(VIVS_PA_CONFIG_CULL_FACE_MODE, CCW));
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);

        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_WRITE_ENABLE, 0));
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_INL(VIVS_PE_DEPTH_CONFIG_DEPTH_MODE, NONE));
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_WRITE_ENABLE, 0));
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED(VIVS_PE_DEPTH_CONFIG_DEPTH_FUNC, COMPARE_FUNC_ALWAYS));
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_INL(VIVS_PE_DEPTH_CONFIG_DEPTH_MODE, Z));
        etna_set_state_f32(ctx, VIVS_PE_DEPTH_NEAR, 0.0);
        etna_set_state_f32(ctx, VIVS_PE_DEPTH_FAR, 1.0);
        etna_set_state_f32(ctx, VIVS_PE_DEPTH_NORMALIZE, 65535.0);

        /* set up primitive assembly */
        etna_set_state_f32(ctx, VIVS_PA_VIEWPORT_OFFSET_Z, 0.0);
        etna_set_state_f32(ctx, VIVS_PA_VIEWPORT_SCALE_Z, 1.0);
        etna_set_state(ctx, VIVS_PE_DEPTH_CONFIG, ETNA_MASKED_BIT(VIVS_PE_DEPTH_CONFIG_ONLY_DEPTH, 0));
        etna_set_state_fixp(ctx, VIVS_PA_VIEWPORT_OFFSET_X, width << 15);
        etna_set_state_fixp(ctx, VIVS_PA_VIEWPORT_OFFSET_Y, height << 15);
        etna_set_state_fixp(ctx, VIVS_PA_VIEWPORT_SCALE_X, width << 15);
        etna_set_state_fixp(ctx, VIVS_PA_VIEWPORT_SCALE_Y, height << 15);
        etna_set_state_fixp(ctx, VIVS_SE_SCISSOR_LEFT, 0);
        etna_set_state_fixp(ctx, VIVS_SE_SCISSOR_TOP, 0);
        etna_set_state_fixp(ctx, VIVS_SE_SCISSOR_RIGHT, (width << 16) | 5);
        etna_set_state_fixp(ctx, VIVS_SE_SCISSOR_BOTTOM, (height << 16) | 5);

        /* shader setup */
        etna_set_state(ctx, VIVS_VS_END_PC, vs_size/16);
        etna_set_state_multi(ctx, VIVS_VS_INPUT_COUNT, 3, (uint32_t[]){
                /* VIVS_VS_INPUT_COUNT */ (1<<8) | 3,
                /* VIVS_VS_TEMP_REGISTER_CONTROL */ 6 << VIVS_VS_TEMP_REGISTER_CONTROL_NUM_TEMPS__SHIFT,
                /* VIVS_VS_OUTPUT(0) */ 4});
        etna_set_state(ctx, VIVS_VS_START_PC, 0x0);
        etna_set_state_f32(ctx, VIVS_VS_UNIFORMS(45), 0.5); /* u11.y */
        etna_set_state_f32(ctx, VIVS_VS_UNIFORMS(44), 1.0); /* u11.x */
        etna_set_state_f32(ctx, VIVS_VS_UNIFORMS(27), 0.0); /* u6.w */
        etna_set_state_f32(ctx, VIVS_VS_UNIFORMS(23), 20.0); /* u5.w */
        etna_set_state_f32(ctx, VIVS_VS_UNIFORMS(19), 2.0); /* u4.w */

        /* Now load the shader itself */
        etna_set_state_multi(ctx, VIVS_VS_INST_MEM(0), vs_size/4, vs);
        etna_set_state(ctx, VIVS_RA_CONTROL, 0x1);
        etna_set_state_multi(ctx, VIVS_PS_END_PC, 2, (uint32_t[]){
                /* VIVS_PS_END_PC */ ps_size/16,
                /* VIVS_PS_OUTPUT_REG */ 0x1});
        etna_set_state(ctx, VIVS_PS_START_PC, 0x0);
        etna_set_state(ctx, VIVS_PA_SHADER_ATTRIBUTES(0), 0x200);
        etna_set_state(ctx, VIVS_GL_PS_VARYING_NUM_COMPONENTS,  /* one varying, with four components */
                (4 << VIVS_GL_VS_VARYING_NUM_COMPONENTS_VAR0__SHIFT)
                );
        etna_set_state_multi(ctx, VIVS_GL_PS_VARYING_COMPONENT_USE(0), 2, (uint32_t[]){ /* one varying, with four components */
                (VARYING_COMPONENT_USE_USED << VIVS_GL_PS_VARYING_COMPONENT_USE_COMP0__SHIFT) |
                (VARYING_COMPONENT_USE_USED << VIVS_GL_PS_VARYING_COMPONENT_USE_COMP1__SHIFT) |
                (VARYING_COMPONENT_USE_USED << VIVS_GL_PS_VARYING_COMPONENT_USE_COMP2__SHIFT) |
                (VARYING_COMPONENT_USE_USED << VIVS_GL_PS_VARYING_COMPONENT_USE_COMP3__SHIFT)
                , 0
                });
        etna_set_state_multi(ctx, VIVS_PS_INST_MEM(0), ps_size/4, ps);
        etna_set_state(ctx, VIVS_PS_INPUT_COUNT, (31<<8)|2);
        etna_set_state(ctx, VIVS_PS_TEMP_REGISTER_CONTROL, 
                (2 << VIVS_PS_TEMP_REGISTER_CONTROL_NUM_TEMPS__SHIFT));
        etna_set_state(ctx, VIVS_PS_CONTROL, 
                VIVS_PS_CONTROL_UNK1
                );
        etna_set_state(ctx, VIVS_PA_ATTRIBUTE_ELEMENT_COUNT, 0x100);
        etna_set_state(ctx, VIVS_GL_VS_VARYING_NUM_COMPONENTS,  /* one varying, with four components */
                (4 << VIVS_GL_VS_VARYING_NUM_COMPONENTS_VAR0__SHIFT)
                );
        etna_set_state(ctx, VIVS_VS_LOAD_BALANCING, 0xf3f0582);
        etna_set_state(ctx, VIVS_VS_OUTPUT_COUNT, 2);
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_BIT(VIVS_PA_CONFIG_POINT_SIZE_ENABLE, 0));
        
        /*   Compute transform matrices in the same way as cube egl demo */ 
        ESMatrix modelview;
        esMatrixLoadIdentity(&modelview);
        esTranslate(&modelview, 0.0f, 0.0f, -8.0f);
        esRotate(&modelview, 45.0f, 1.0f, 0.0f, 0.0f);
        esRotate(&modelview, 45.0f, 0.0f, 1.0f, 0.0f);
        esRotate(&modelview, frame*0.5f, 0.0f, 0.0f, 1.0f);

        GLfloat aspect = (GLfloat)(height) / (GLfloat)(width);

        ESMatrix projection;
        esMatrixLoadIdentity(&projection);
        esFrustum(&projection, -2.8f, +2.8f, -2.8f * aspect, +2.8f * aspect, 6.0f, 10.0f);

        ESMatrix modelviewprojection;
        esMatrixLoadIdentity(&modelviewprojection);
        esMatrixMultiply(&modelviewprojection, &modelview, &projection);

        float normal[9]; /* normal transformation matrix */
        normal[0] = modelview.m[0][0];
        normal[1] = modelview.m[0][1];
        normal[2] = modelview.m[0][2];
        normal[3] = modelview.m[1][0];
        normal[4] = modelview.m[1][1];
        normal[5] = modelview.m[1][2];
        normal[6] = modelview.m[2][0];
        normal[7] = modelview.m[2][1];
        normal[8] = modelview.m[2][2];
        
        etna_set_state_multi(ctx, VIVS_VS_UNIFORMS(0), 16, (uint32_t*)&modelviewprojection.m[0][0]);
        etna_set_state_multi(ctx, VIVS_VS_UNIFORMS(16), 3, (uint32_t*)&normal[0]); /* u4.xyz */
        etna_set_state_multi(ctx, VIVS_VS_UNIFORMS(20), 3, (uint32_t*)&normal[3]); /* u5.xyz */
        etna_set_state_multi(ctx, VIVS_VS_UNIFORMS(24), 3, (uint32_t*)&normal[6]); /* u6.xyz */
        etna_set_state_multi(ctx, VIVS_VS_UNIFORMS(28), 16, (uint32_t*)&modelview.m[0][0]);
        etna_set_state(ctx, VIVS_FE_VERTEX_STREAM_BASE_ADDR, vtx_physical); /* ADDR_E */
        etna_set_state(ctx, VIVS_FE_VERTEX_STREAM_CONTROL, 
                0x24 << VIVS_FE_VERTEX_STREAM_CONTROL_VERTEX_STRIDE__SHIFT);
        etna_set_state(ctx, VIVS_FE_VERTEX_ELEMENT_CONFIG(0), 
                VIVS_FE_VERTEX_ELEMENT_CONFIG_TYPE_FLOAT |
                (ENDIAN_MODE_NO_SWAP << VIVS_FE_VERTEX_ELEMENT_CONFIG_ENDIAN__SHIFT) |
                (0 << VIVS_FE_VERTEX_ELEMENT_CONFIG_STREAM__SHIFT) |
                (3 <<VIVS_FE_VERTEX_ELEMENT_CONFIG_NUM__SHIFT) |
                VIVS_FE_VERTEX_ELEMENT_CONFIG_NORMALIZE_OFF |
                (0x0 << VIVS_FE_VERTEX_ELEMENT_CONFIG_START__SHIFT) |
                (0xc << VIVS_FE_VERTEX_ELEMENT_CONFIG_END__SHIFT));
        etna_set_state(ctx, VIVS_FE_VERTEX_ELEMENT_CONFIG(1), 
                VIVS_FE_VERTEX_ELEMENT_CONFIG_TYPE_FLOAT |
                (ENDIAN_MODE_NO_SWAP << VIVS_FE_VERTEX_ELEMENT_CONFIG_ENDIAN__SHIFT) |
                (0 << VIVS_FE_VERTEX_ELEMENT_CONFIG_STREAM__SHIFT) |
                (3 <<VIVS_FE_VERTEX_ELEMENT_CONFIG_NUM__SHIFT) |
                VIVS_FE_VERTEX_ELEMENT_CONFIG_NORMALIZE_OFF |
                (0xc << VIVS_FE_VERTEX_ELEMENT_CONFIG_START__SHIFT) |
                (0x18 << VIVS_FE_VERTEX_ELEMENT_CONFIG_END__SHIFT));
        etna_set_state(ctx, VIVS_FE_VERTEX_ELEMENT_CONFIG(2), 
                VIVS_FE_VERTEX_ELEMENT_CONFIG_TYPE_FLOAT |
                (ENDIAN_MODE_NO_SWAP << VIVS_FE_VERTEX_ELEMENT_CONFIG_ENDIAN__SHIFT) |
                VIVS_FE_VERTEX_ELEMENT_CONFIG_NONCONSECUTIVE |
                (0 << VIVS_FE_VERTEX_ELEMENT_CONFIG_STREAM__SHIFT) |
                (3 <<VIVS_FE_VERTEX_ELEMENT_CONFIG_NUM__SHIFT) |
                VIVS_FE_VERTEX_ELEMENT_CONFIG_NORMALIZE_OFF |
                (0x18 << VIVS_FE_VERTEX_ELEMENT_CONFIG_START__SHIFT) |
                (0x24 << VIVS_FE_VERTEX_ELEMENT_CONFIG_END__SHIFT));
        etna_set_state(ctx, VIVS_VS_INPUT(0), 0x20100);
        etna_set_state(ctx, VIVS_PA_CONFIG, ETNA_MASKED_BIT(VIVS_PA_CONFIG_POINT_SPRITE_ENABLE, 0));

        for(int prim=0; prim<6; ++prim)
        {
            etna_draw_primitives(ctx, PRIMITIVE_TYPE_TRIANGLE_STRIP, prim*4, 2);
        }
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);

        etna_flush(ctx);

        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
        etna_set_state(ctx, VIVS_RS_CONFIG,
                VIVS_RS_CONFIG_SOURCE_FORMAT(RS_FORMAT_A8R8G8B8) |
                VIVS_RS_CONFIG_SOURCE_TILED |
                VIVS_RS_CONFIG_DEST_FORMAT(RS_FORMAT_A8R8G8B8) |
                VIVS_RS_CONFIG_DEST_TILED);
        etna_set_state(ctx, VIVS_RS_SOURCE_STRIDE, (padded_width * 4 * 4) | VIVS_RS_SOURCE_STRIDE_TILING);
        etna_set_state(ctx, VIVS_RS_DEST_STRIDE, (padded_width * 4 * 4) | VIVS_RS_DEST_STRIDE_TILING);
        etna_set_state(ctx, VIVS_RS_DITHER(0), 0xffffffff);
        etna_set_state(ctx, VIVS_RS_DITHER(1), 0xffffffff);
        etna_set_state(ctx, VIVS_RS_CLEAR_CONTROL, VIVS_RS_CLEAR_CONTROL_MODE_DISABLED);
        etna_set_state(ctx, VIVS_RS_EXTRA_CONFIG, 0); /* no AA, no endian switch */
        etna_set_state(ctx, VIVS_RS_SOURCE_ADDR, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_RS_DEST_ADDR, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_RS_WINDOW_SIZE, 
                VIVS_RS_WINDOW_SIZE_HEIGHT(padded_height) |
                VIVS_RS_WINDOW_SIZE_WIDTH(padded_width));
        etna_set_state(ctx, VIVS_RS_KICKER, 0xbeebbeeb);

        /* Submit second command buffer */
        etna_flush(ctx);

        etna_warm_up_rs(ctx, aux_rt_physical, aux_rt_ts_physical);

        etna_set_state(ctx, VIVS_TS_COLOR_STATUS_BASE, rt_ts_physical); /* ADDR_B */
        etna_set_state(ctx, VIVS_TS_COLOR_SURFACE_BASE, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_TS_MEM_CONFIG, 
                VIVS_TS_MEM_CONFIG_DEPTH_FAST_CLEAR |
                VIVS_TS_MEM_CONFIG_DEPTH_16BPP | 
                VIVS_TS_MEM_CONFIG_DEPTH_COMPRESSION);
        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR);
        etna_set_state(ctx, VIVS_PE_COLOR_FORMAT, 
                ETNA_MASKED_BIT(VIVS_PE_COLOR_FORMAT_PARTIAL, 0));

        /* Submit third command buffer, wait for pixel engine to finish */
        etna_finish(ctx);


        etna_set_state(ctx, VIVS_GL_FLUSH_CACHE, VIVS_GL_FLUSH_CACHE_COLOR | VIVS_GL_FLUSH_CACHE_DEPTH);
        etna_set_state(ctx, VIVS_RS_CONFIG,
                VIVS_RS_CONFIG_SOURCE_FORMAT(RS_FORMAT_A8R8G8B8) |
                VIVS_RS_CONFIG_SOURCE_TILED |
                VIVS_RS_CONFIG_DEST_FORMAT(RS_FORMAT_A8R8G8B8) /*|
                VIVS_RS_CONFIG_SWAP_RB*/);
        etna_set_state(ctx, VIVS_RS_SOURCE_STRIDE, (padded_width * 4 * 4) | VIVS_RS_SOURCE_STRIDE_TILING);
        etna_set_state(ctx, VIVS_RS_DEST_STRIDE, fb.fb_fix.line_length);
        etna_set_state(ctx, VIVS_RS_DITHER(0), 0xffffffff);
        etna_set_state(ctx, VIVS_RS_DITHER(1), 0xffffffff);
        etna_set_state(ctx, VIVS_RS_CLEAR_CONTROL, VIVS_RS_CLEAR_CONTROL_MODE_DISABLED);
        etna_set_state(ctx, VIVS_RS_EXTRA_CONFIG, 
                0); /* no AA, no endian switch */
        etna_set_state(ctx, VIVS_RS_SOURCE_ADDR, rt_physical); /* ADDR_A */
        etna_set_state(ctx, VIVS_RS_DEST_ADDR, fb.physical[backbuffer]); /* ADDR_J */
        etna_set_state(ctx, VIVS_RS_WINDOW_SIZE, 
                VIVS_RS_WINDOW_SIZE_HEIGHT(height) |
                VIVS_RS_WINDOW_SIZE_WIDTH(width));
        etna_set_state(ctx, VIVS_RS_KICKER, 0xbeebbeeb);
        etna_finish(ctx);
        /* switch buffers */
        fb_set_buffer(&fb, backbuffer);
        backbuffer = 1-backbuffer;
    }
    
    /* Unlock video memory */
    if(viv_unlock_vidmem(bmp_node, gcvSURF_BITMAP, 1) != 0)
    {
        fprintf(stderr, "Cannot unlock vidmem\n");
        exit(1);
    }

    etna_free(ctx);
    viv_close();
    return 0;
}