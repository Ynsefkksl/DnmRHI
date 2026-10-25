#pragma once

#include <DnmRHI/Defines.hpp>
#include <stdfloat>
#include <type_traits>

namespace DnmRHI {
    constexpr auto Frames_In_Flight_Count = 3u;
    constexpr auto Uniform_Buffer_Count = 15u; 
    constexpr auto Sampled_Image_Count = 200'000u; 
    constexpr auto Storage_Image_Count = 20'000u; 
    constexpr auto Sampler_Count = 2'000u;
    constexpr auto Push_Constant_Size = 256u;

	template<typename T>
	using PureType = std::remove_cvref_t<std::remove_pointer_t<T>>;

	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using uint = uint32_t;
	using uptr = std::uintptr_t;
	using usize = std::size_t;
	
	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;

	using b8 = bool;

	#if defined (__STDCPP_FLOAT32_T__)
		using f32 = std::float32_t;
	#else
		using f32 = float;
	#endif
	
	#if defined (__STDCPP_FLOAT64_T__)
		using f64 = std::float64_t;
	#else
		using f64 = double;
	#endif
	
	using fp = f32;

	class Device;
    class Image;
    class Buffer;
    class CommandBuffer;
    class GraphicsPipeline;
    class ComputePipeline;
    class Semaphore;
    class Swapchain;
    class Object;
    class Shader;
    class Resource;
    class ShaderCompiler;

    struct ImageDesc;
    struct BufferDesc;
    struct SamplerDesc;
    struct CommandBufferDesc;
    struct GraphicsPipelineDesc;
    struct ComputePipelineDesc;
    struct SemaphoreDesc;
    struct SwapchainDesc;
    struct ObjectDesc;
    struct ShaderDesc;
    struct ResourceDesc;

    struct VulkanSettings;
    struct D3D12Settings;

    enum class GpuVendor : u8 {
        eUnknown,
        eNvidia,
        eAmd,
        eIntel,
        eQualcomm,
    };

    enum class GraphicsApi : u8 {
        eVk,
        eD3D12
    };

    enum class ObjectType : u8 {
        eGraphicsPipeline,
        eComputePipeline,
        eRayTracingPipeline,
        eBuffer,
        eImage,
        eCommandBuffer,
        eSemaphore,
        eSwapchain,
        eShader,
        Count
    };

    enum class ResourceType : u8 {
        eBuffer,
        eImage,
    };

    enum class DeviceType : u8 {
        eUnknown,
        eIntegred,
        eDiscrete,
        eVirtual,
        eCpu
    };

    struct Offset2D {
        i32 x, y;

        constexpr auto &SetX(i32 x) noexcept { this->x = x; return *this; }
        constexpr auto &SetY(i32 y) noexcept { this->y = y; return *this; }
    };

    struct Offset3D {
        i32 x, y, z;

        constexpr auto &SetX(i32 x) noexcept { this->x = x; return *this; }
        constexpr auto &SetY(i32 y) noexcept { this->y = y; return *this; }
        constexpr auto &SetZ(i32 z) noexcept { this->z = z; return  *this; }
    };

    struct Extent2D {
        u32 width, height;

        constexpr auto &SetWidth(u32 width) noexcept { this->width = width; return *this; }
        constexpr auto &SetHeight(u32 height) noexcept { this->height = height; return *this; }
    };

    struct Extent3D {
        u32 width, height, depth;

        constexpr auto &SetWidth(u32 width) noexcept { this->width = width; return *this; }
        constexpr auto &SetHeight(u32 height) noexcept { this->height = height; return *this; }
        constexpr auto &SetDepth(u32 depth) noexcept { this->depth = depth; return  *this; }
    };

    class Object {
    public:
        explicit Object(Device &device, ObjectType type) noexcept : c_Type(type), m_device(device) {}
        virtual ~Object() noexcept = default;

        Object(const Object &) = delete;
        Object &operator=(const Object &) = delete;
        Object(Object &&) noexcept = delete;
        Object &operator=(Object &&) noexcept = delete;

        [[nodiscard]] DnmRHI_Force_Inline auto &GetDevice() const noexcept { return m_device; }
        [[nodiscard]] DnmRHI_Force_Inline auto GetObjectType() const noexcept { return c_Type; }

        Device &m_device;
        const ObjectType c_Type; 
    };

    class Resource : public Object {
        constexpr ResourceType GetResouceTypeFromObjectType(ObjectType type) const noexcept {
            switch (type) {
                case ObjectType::eBuffer: return ResourceType::eBuffer;
                case ObjectType::eImage: return ResourceType::eImage;
				//TODO:
                default: return ResourceType::eBuffer; 
            }
        }
    public:
        Resource(Device &device, ObjectType type) noexcept : Object(device, type), c_ResourceType(GetResouceTypeFromObjectType(type)) {}

        const ResourceType c_ResourceType;
    };
}
