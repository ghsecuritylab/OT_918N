


#include "vpx_ports/config.h"
#include "blockd.h"
#include "reconintra.h"
#include "vpx_mem/vpx_mem.h"
#include "recon.h"

#if HAVE_ARMV7
extern void vp8_build_intra_predictors_mby_neon_func(
    unsigned char *y_buffer,
    unsigned char *ypred_ptr,
    int y_stride,
    int mode,
    int Up,
    int Left);

void vp8_build_intra_predictors_mby_neon(MACROBLOCKD *x)
{
    unsigned char *y_buffer = x->dst.y_buffer;
    unsigned char *ypred_ptr = x->predictor;
    int y_stride = x->dst.y_stride;
    int mode = x->mode_info_context->mbmi.mode;
    int Up = x->up_available;
    int Left = x->left_available;

    vp8_build_intra_predictors_mby_neon_func(y_buffer, ypred_ptr, y_stride, mode, Up, Left);
}
#endif


#if HAVE_ARMV7
extern void vp8_build_intra_predictors_mby_s_neon_func(
    unsigned char *y_buffer,
    unsigned char *ypred_ptr,
    int y_stride,
    int mode,
    int Up,
    int Left);

void vp8_build_intra_predictors_mby_s_neon(MACROBLOCKD *x)
{
    unsigned char *y_buffer = x->dst.y_buffer;
    unsigned char *ypred_ptr = x->predictor;
    int y_stride = x->dst.y_stride;
    int mode = x->mode_info_context->mbmi.mode;
    int Up = x->up_available;
    int Left = x->left_available;

    vp8_build_intra_predictors_mby_s_neon_func(y_buffer, ypred_ptr, y_stride, mode, Up, Left);
}

#endif
