//  This file is part of seismic-rtm.
//
//  seismic-rtm is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  seismic-rtm is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seismic.  If not, see <http://www.gnu.org/licenses/>.

#include "kernel_ARM_NEON.h"

/*
     http://www.xilinx.com/support/documentation/application_notes/xapp792-high-performance-video-zynq.pdf

     Achieving over 60% Utilization
     ------------------------------

     To achieve optimal utilization of the memory controller, transactions from
     the master interfaces need to occur in different banks and must be aligned to KB/MB boundaries.

With video designs, frame buffers need to be accessed in different banks, and the overlapping of banks must be
minimized while the video design is running.
This design demonstrates 1080P60 (1920 x 1080) with four bytes per pixel. Each horizontal
line is about 8 KB (1920 x 4 = 7680), so the AXI VDMA line stride is set to 8 KB boundaries.
The start of every new line is aligned on an 8 KB boundary. Each frame’s vertical lines (1080
lines per frame) are aligned to a 2 KB boundary for each frame. Therefore, each frame buffer in
this design is aligned on 16 MB boundaries (8 KB x 2 KB).
With multiple video pipelines, N video devices are accessing N frame buffers using identical
timing. Assuming 16 MB frame buffers, this means that nearly simultaneous access requests
from different video devices have nearly identical addresses [23:0]. Because in the default
address mapping, the bank select address is on bits [14:12], they are also identical addresses.
Therefore, requests access the same bank but from different pages, causing a full page miss
overhead (precharge-activate-read/write) each ti
me. This can cause very low DRAM efficiency
(in the 40–50% range).
The memory controller on the ZC702 board is configured for row (13 bits), bank (3 bits), column
(10 bits), and word selection (2 bits) using this DDR address configuration:

      DDR_ADDR[27:15]   Row
      DDR_ADDR[14:12]   Bank
      DDR_ADDR[11: 0]   Column/Word

The AXI master address is reordered to ensure that each frame buffer is in its own bank.
Because each frame buffer is aligned on 16 MB boundaries, it takes 24 bits to represent the
address space [23:0] and three bits to represent the bank [26:24]. AXI address bits [26:24] are
moved to [14:12] for the DDR address. The reordered AXI master address looks like:
      axi_addr[31:27] axi_addr[23:12] axi_addr[26:24] axi_addr[11:0]
An additional EDK IP core was created for this application note to reorder the AXI master
address to the AXI interface. With this reordering, the processor or software must handle the
address reordering.
*/


inline __attribute__((always_inline)) void kernel_arm_neon_aligned( stack_t * data, float32x4_t neon_two, float32x4_t neon_sixteen, float32x4_t neon_minus_sixty  )
{
    float32x4_t neon_ppf, neon_vel, neon_actual, neon_above1, neon_left1, neon_under1, neon_right1, neon_sum, neon_under, neon_above;
    float32x4_t neon_above2, neon_under2, neon_left2, neon_right2;

    unsigned i, j;
    // spatial loop in x
    for (i=data->x_start; i<data->x_end; i++) {
        // spatial loop in y
        for (j=data->y_start; j<data->y_end; j+=4) {
            unsigned r = i * data->height + j;
            unsigned r_min1 = r - data->height;
            unsigned r_min2 = r - (data->height << 1);
            unsigned r_plus1 = r + data->height;
            unsigned r_plus2 = r + (data->height << 1);

            float32x4_t neon_ppf = vld1q_f32( (const float32_t *) &data->nppf[ r ] );
            float32x4_t neon_vel = vld1q_f32( (const float32_t *) &data->vel[ r ] );
            float32x4_t neon_actual = vld1q_f32( (const float32_t *) &data->apf[ r ] );
            float32x4_t neon_right_pre = vld1q_f32( (const float32_t *) &data->apf[ r + 4 ] );
            float32x4_t neon_left_pre  = vld1q_f32( (const float32_t *) &data->apf[ r - 4 ] );
            float32x4_t neon_under1  = vld1q_f32( (const float32_t *) &data->apf[ r_min1 ] );
            float32x4_t neon_under2  = vld1q_f32( (const float32_t *) &data->apf[ r_min2 ] );
            float32x4_t neon_above1  = vld1q_f32( (const float32_t *) &data->apf[ r_plus1 ] );
            float32x4_t neon_above2  = vld1q_f32( (const float32_t *) &data->apf[ r_plus2 ] );
            float32x4_t neon_right1 = vextq_f32( neon_actual, neon_right_pre, 1 );
            float32x4_t neon_right2 = vextq_f32( neon_actual, neon_right_pre, 2 );
            float32x4_t neon_left1 = vextq_f32( neon_left_pre, neon_actual, 3 );
            float32x4_t neon_left2 = vextq_f32( neon_left_pre, neon_actual, 2 );
            float32x4_t sum0 = vaddq_f32( neon_left1, neon_right1 );
            float32x4_t sum1 = vaddq_f32( neon_under1, neon_above1 );
            float32x4_t sum2 = vaddq_f32( neon_left2, neon_right2 );
            float32x4_t sum3 = vaddq_f32( neon_under2, neon_above2 );
            float32x4_t sum4 = vaddq_f32( sum0, sum1 );
            float32x4_t sum5 = vaddq_f32( sum2, sum3 );
            float32x4_t sum6 = vmlsq_f32( sum5, sum4, neon_sixteen ); // negiert!
            sum6 = vnegq_f32( sum6 ); // ToDo!
            float32x4_t sum7 = vmlaq_f32( sum6, neon_actual, neon_minus_sixty );
            float32x4_t sum8 = vmlsq_f32( neon_ppf, neon_actual, neon_two ); // negiert!
            sum8 = vnegq_f32( sum8 );
            float32x4_t sum9 = vmlaq_f32( sum8, neon_vel, sum7 );
            vst1q_f32( (float32_t *) &(data->nppf[r]), sum9);
        }
    }
}


SEISMIC_EXEC_ARM_NEON_FCT( aligned );
