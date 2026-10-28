#pragma once

// TODO: this must be function
static const uint Subgroup_Size =
#ifdef SUBGROUP_SIZE
    SUBGROUP_SIZE;
#else
    64;
#endif

__intrinsic_op(218)
T __spirvLoadDescriptorFromHeap<T>(uint heap, uint index);

__intrinsic_op(221)
uint __spirvSamplerHeap();

__intrinsic_op(220)
uint __spirvResourceHeap();

interface IBindlessDescriptor {};

// TODO: we need data layouts
__generic<T, L : IBufferDataLayout = Std430DataLayout>
typealias UniformBuffer = ConstantBuffer<T, L>;

__generic<T : ITexelElement>
typealias ReadonlyImage2D = Texture2D<T>;
__generic<T : ITexelElement>
typealias WritableImage2D = RWTexture2D<T>;
__generic<T : ITexelElement>
typealias WriteonlyImage2D = WTexture2D<T>;

__generic<T : ITexelElement>
typealias ReadonlyImage2DArray = Texture2DArray<T>;
__generic<T : ITexelElement>
typealias WritableImage2DArray = RWTexture2DArray<T>;
__generic<T : ITexelElement>
typealias WriteonlyImage2DArray = WTexture2DArray<T>;

__generic<T : ITexelElement>
typealias ReadonlyImage3D = Texture3D<T>;
__generic<T : ITexelElement>
typealias WritableImage3D = RWTexture3D<T>;
__generic<T : ITexelElement>
typealias WriteonlyImage3D = WTexture3D<T>;

__generic<T : ITexelElement>
typealias ReadonlyImageCube = TextureCube<T>;

typealias Sampler = SamplerState;

__generic<T, L : IBufferDataLayout = Std430DataLayout>
extension UniformBuffer<T, L> : IBindlessDescriptor {}

__generic<T : ITexelElement>
extension ReadonlyImage2D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WritableImage2D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WriteonlyImage2D<T> : IBindlessDescriptor {}

__generic<T : ITexelElement>
extension ReadonlyImage2DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WritableImage2DArray<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WriteonlyImage2DArray<T> : IBindlessDescriptor {}

__generic<T : ITexelElement>
extension ReadonlyImage3D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WritableImage3D<T> : IBindlessDescriptor {}
__generic<T : ITexelElement>
extension WriteonlyImage3D<T> : IBindlessDescriptor {}

__generic<T : ITexelElement>
extension ReadonlyImageCube<T> : IBindlessDescriptor {}

extension Sampler : IBindlessDescriptor {}

[vk::binding(0, 0)]
[require(spirv)]
__DynamicResource<__DynamicResourceKind.Sampler> __DnmRHI_SpirV_Sampler_Set__[];

[vk::binding(0, 1)]
[require(spirv)]
__DynamicResource<__DynamicResourceKind.General> __DnmRHI_SpirV_Resource_Set__[];

T GetBindless<T>(uint index)
    where T : IOpaqueDescriptor, IBindlessDescriptor {
    __target_switch
    {
        case spirv:
        __target_switch
        {
        case spvDescriptorHeapEXT:
            if (T.kind == DescriptorKind.Sampler)
                return __spirvLoadDescriptorFromHeap<T>(__spirvSamplerHeap(), index);
            else
                return __spirvLoadDescriptorFromHeap<T>(__spirvResourceHeap(), index);
        default:
            if (T.kind == DescriptorKind.Sampler)
                return __DnmRHI_SpirV_Sampler_Set__[index].asOpaqueDescriptor<T>();
            else
                return __DnmRHI_SpirV_Resource_Set__[index].asOpaqueDescriptor<T>();
        }
    }
}

typealias i8 = int8_t;
typealias i16 = int16_t;
typealias i32 = int32_t;
typealias i64 = int64_t;

// TODO: this is bad
typealias int_in_out =
#ifdef IN_OUT_16
    i16;
#else
    i32;
#endif

typealias i8_2 = vector<i8, 2>;
typealias i8_3 = vector<i8, 3>;
typealias i8_4 = vector<i8, 4>;

typealias i16_2 = vector<i16, 2>;
typealias i16_3 = vector<i16, 3>;
typealias i16_4 = vector<i16, 4>;

typealias i32_2 = vector<i32, 2>;
typealias i32_3 = vector<i32, 3>;
typealias i32_4 = vector<i32, 4>;

typealias int_2_in_out = vector<int_in_out, 2>;
typealias int_3_in_out = vector<int_in_out, 3>;
typealias int_4_in_out = vector<int_in_out, 4>;

typealias u8 = uint8_t;
typealias u16 = uint16_t;
typealias u32 = uint32_t;
typealias u64 = uint64_t;

// TODO: this is bad
typealias uint_in_out =
#ifdef IN_OUT_16
    u16;
#else
    u32;
#endif

typealias u8_2 = vector<u8, 2>;
typealias u8_3 = vector<u8, 3>;
typealias u8_4 = vector<u8, 4>;

typealias u16_2 = vector<u16, 2>;
typealias u16_3 = vector<u16, 3>;
typealias u16_4 = vector<u16, 4>;

typealias u32_2 = vector<u32, 2>;
typealias u32_3 = vector<u32, 3>;
typealias u32_4 = vector<u32, 4>;

typealias uint_2_in_out = vector<uint_in_out, 2>;
typealias uint_3_in_out = vector<uint_in_out, 3>;
typealias uint_4_in_out = vector<uint_in_out, 4>;

typealias f16 = float16_t;
typealias f32 = float32_t;

typealias fp = f32;

// TODO: this is bad
typealias fp_in_out =
#ifdef IN_OUT_16
    f16;
#else
    f32;
#endif

typealias f16_2 = vector<f16, 2>;
typealias f16_3 = vector<f16, 3>;
typealias f16_4 = vector<f16, 4>;

typealias f32_2 = vector<f32, 2>;
typealias f32_3 = vector<f32, 3>;
typealias f32_4 = vector<f32, 4>;

typealias fp_2 = vector<fp, 2>;
typealias fp_3 = vector<fp, 3>;
typealias fp_4 = vector<fp, 4>;

typealias fp_2_in_out = vector<fp_in_out, 2>;
typealias fp_3_in_out = vector<fp_in_out, 3>;
typealias fp_4_in_out = vector<fp_in_out, 4>;

typealias f16_2_2 = matrix<f16, 2, 2>;
typealias f16_2_3 = matrix<f16, 2, 3>;
typealias f16_2_4 = matrix<f16, 2, 4>;

typealias f16_3_2 = matrix<f16, 3, 2>;
typealias f16_3_3 = matrix<f16, 3, 3>;
typealias f16_3_4 = matrix<f16, 3, 4>;

typealias f16_4_2 = matrix<f16, 4, 2>;
typealias f16_4_3 = matrix<f16, 4, 3>;
typealias f16_4_4 = matrix<f16, 4, 4>;

typealias f32_2_2 = matrix<f32, 2, 2>;
typealias f32_2_3 = matrix<f32, 2, 3>;
typealias f32_2_4 = matrix<f32, 2, 4>;

typealias f32_3_2 = matrix<f32, 3, 2>;
typealias f32_3_3 = matrix<f32, 3, 3>;
typealias f32_3_4 = matrix<f32, 3, 4>;

typealias f32_4_2 = matrix<f32, 4, 2>;
typealias f32_4_3 = matrix<f32, 4, 3>;
typealias f32_4_4 = matrix<f32, 4, 4>;

typealias fp_2_2 = matrix<fp, 2, 2>;
typealias fp_2_3 = matrix<fp, 2, 3>;
typealias fp_2_4 = matrix<fp, 2, 4>;

typealias fp_3_2 = matrix<fp, 3, 2>;
typealias fp_3_3 = matrix<fp, 3, 3>;
typealias fp_3_4 = matrix<fp, 3, 4>;

typealias fp_4_2 = matrix<fp, 4, 2>;
typealias fp_4_3 = matrix<fp, 4, 3>;
typealias fp_4_4 = matrix<fp, 4, 4>;

typealias fp_2_2_in_out = matrix<fp_in_out, 2, 2>;
typealias fp_2_3_in_out = matrix<fp_in_out, 2, 3>;
typealias fp_2_4_in_out = matrix<fp_in_out, 2, 4>;

typealias fp_3_2_in_out = matrix<fp_in_out, 3, 2>;
typealias fp_3_3_in_out = matrix<fp_in_out, 3, 3>;
typealias fp_3_4_in_out = matrix<fp_in_out, 3, 4>;

typealias fp_4_2_in_out = matrix<fp_in_out, 4, 2>;
typealias fp_4_3_in_out = matrix<fp_in_out, 4, 3>;
typealias fp_4_4_in_out = matrix<fp_in_out, 4, 4>;

struct IndirectDrawIndexedCommand {
    u32 index_count;
    u32 instance_count;
    u32 first_index;
    i32 vertex_offset;
    u32 first_instance;
};

struct IndirectDrawCommand {
    u32 vertex_count;
    u32 instance_count;
    u32 first_vertex;
    u32 first_instance;
};
