// BackendMetal.h

#ifndef __BackendMetal_h_
#define __BackendMetal_h_

#import <Metal/Metal.h>
#import <Metal/MTLDrawable.h>

namespace axgl {

struct RectSize {
	uint32_t width;
	uint32_t height;
};

MTLPrimitiveType convert_primitive_type(int32_t mode);
MTLCompareFunction convert_compare_func(int32_t func);
MTLCompareFunction convert_depth_stencil_compare_func(bool enable, int32_t func);
BOOL convert_depth_write_mask(bool enable, int32_t writeMask);
MTLStencilOperation convert_stencil_op(bool enable, int32_t op);
uint32_t convert_stencil_mask(bool enable, uint32_t mask);
MTLVertexFormat convert_vertex_format(int32_t type, int32_t size, bool normalized);
MTLPixelFormat convert_internalformat(int32_t internalformat);
MTLColorWriteMask convert_color_write_mask(bool red, bool green, bool blue, bool alpha);
MTLBlendOperation convert_blend_operation(int32_t eq);
MTLBlendFactor convert_blend_factor(int32_t func);
MTLIndexType convert_index_type(int32_t type);
MTLWinding convert_winding(int32_t mode, bool flipY);
MTLCullMode convert_cull_mode(int32_t enable, int32_t mode);
MTLPixelFormat get_pixel_format(int32_t format, int32_t type);
NSUInteger get_bytes_per_pixel(int32_t format, int32_t type);
NSUInteger get_bytes_per_row_blocks(int32_t format, int32_t width);
int32_t get_shader_data_type(MTLDataType type);
uint32_t get_size_from_gltype(int32_t gltype);
int32_t get_internalformat(MTLPixelFormat pixelFormat);
void get_shader_constant_copy_params(int32_t gltype, uint32_t* size, uint32_t* stride, uint32_t* count);

} // namespace axgl

#endif // __BackendMetal_h_
