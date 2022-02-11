/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

static void (FUNC_NAME)(gavl_video_scale_context_t * ctx, int scanline, uint8_t * dest_start)
  {
  int i, imax;
  int32_t tmp;
  
  uint8_t * src_1;
  uint8_t * src_2;
  uint8_t * dst;
#if 0  
  if(!ctx->scanline)
    fprintf(stderr, "scale_y_linear_mmx %d %d\n",
            ctx->table_v.pixels[ctx->scanline].factor_i[0],
            ctx->table_v.pixels[ctx->scanline].factor_i[1]);
#endif
  imax = (ctx->dst_size * WIDTH_MUL) / 8;
  //  imax = 0;
  
  src_1 =
    ctx->src + ctx->table_v.pixels[scanline].index * ctx->src_stride;
  src_2 = src_1 + ctx->src_stride;

  dst = dest_start;

  /* Load factors */

  /*
   *  mm0: Input1
   *  mm1: Input2
   *  mm2: Factor1
   *  mm3: Factor1
   *  mm4: Output1
   *  mm5: Output2
   *  mm6: Scratch
   *  mm7: 0
   */

  pxor_r2r(mm7, mm7);
  
  /* Load factor1 */
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[0], mm2);
  //  psllw_i2r(7, mm2);
#ifdef MMXEXT
  pshufw_r2r(mm2,mm2,0x00);
#else
  movq_r2r(mm2, mm6);
  psllq_i2r(16, mm6);
  por_r2r(mm6, mm2);
  movq_r2r(mm2, mm6);
  psllq_i2r(32, mm6);
  por_r2r(mm6, mm2);
#endif

  /* Load factor2 */
  movd_m2r(ctx->table_v.pixels[scanline].factor_i[1], mm3);
  //  psllw_i2r(7, mm3);
#ifdef MMXEXT
  pshufw_r2r(mm3,mm3,0x00);
#else
  movq_r2r(mm3, mm6);
  psllq_i2r(16, mm6);
  por_r2r(mm6, mm3);
  movq_r2r(mm3, mm6);
  psllq_i2r(32, mm6);
  por_r2r(mm6, mm3);
#endif
  
  for(i = 0; i < imax; i++)
    {
    /* Load input 1 */
    movq_m2r(*src_1,mm0);
    movq_r2r(mm0,mm1);
    punpcklbw_r2r(mm7, mm0);
    punpckhbw_r2r(mm7, mm1);
    psllw_i2r(7, mm0);
    psllw_i2r(7, mm1);

    /* Accumulate mm0 */
    pmulhw_r2r(mm2, mm0);
    /* Accumulate mm1 */ 
    pmulhw_r2r(mm2, mm1);

    /* Load input 2 */
    movq_m2r(*(src_2),mm4);
    movq_r2r(mm4,mm5);
    punpcklbw_r2r(mm7, mm4);
    punpckhbw_r2r(mm7, mm5);
    psllw_i2r(7, mm4);
    psllw_i2r(7, mm5);

    /* Accumulate mm0 */
    pmulhw_r2r(mm3, mm4);
    paddsw_r2r(mm4, mm0);
    /* Accumulate mm1 */ 
    pmulhw_r2r(mm3, mm5);
    paddsw_r2r(mm5, mm1);

    psraw_i2r(5, mm0);
    psraw_i2r(5, mm1);
    packuswb_r2r(mm1, mm0);
    MOVQ_R2M(mm0, *dst);
    
    dst += 8;
    src_1 += 8;
    src_2 += 8;
    }

  emms();
  
  imax = (ctx->dst_size * WIDTH_MUL) % 8;
  //  imax = (ctx->dst_size * WIDTH_MUL);
  
  if(!imax)
    return;
  
  for(i = 0; i < imax; i++)
    {
    tmp = (*src_1 * ctx->table_v.pixels[scanline].factor_i[0] +
           *src_2 * ctx->table_v.pixels[scanline].factor_i[1]) >> 14;
    //    tmp = (ctx->table_v.pixels[scanline].factor_i[0] +
    //           ctx->table_v.pixels[scanline].factor_i[1]) >> 14;
    

    *dst = (uint8_t)((tmp & ~0xFF)?((-tmp) >> 31) : tmp);
    /* Accum */
    dst++;
    src_1++;
    src_2++;
    }
  }

#undef FUNC_NAME
#undef WIDTH_MUL
