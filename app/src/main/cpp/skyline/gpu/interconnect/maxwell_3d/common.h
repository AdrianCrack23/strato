// SPDX-License-Identifier: MPL-2.0
// Copyright © 2022 Skyline Team and Contributors (https://github.com/skyline-emu/)

#pragma once

#include <common/dirty_tracking.h>
#include <vulkan/vulkan_raii.hpp>
#include <soc/gm20b/engines/maxwell/types.h>
#include <gpu/buffer.h>

namespace skyline::kernel {
    class MemoryManager;
}

namespace skyline::soc::gm20b {
    struct ChannelContext;
}

namespace skyline::gpu::interconnect {
    class CommandExecutor;
}

namespace skyline::gpu::interconnect::maxwell3d {
    namespace engine = skyline::soc::gm20b::engine::maxwell3d::type;

    /**
     * @brief Holds GPU context for an interconnect instance
     */
    struct InterconnectContext {
        soc::gm20b::ChannelContext &channelCtx;
        CommandExecutor &executor;
        GPU &gpu;
        nce::NCE &nce;
        kernel::MemoryManager &memory;
    };

    /**
     * @brief Helper around a buffer view that performs caching based on the underlying GPU mappings
     */
    class CachedMappedBufferView {
      private:
        span<u8> blockMapping; //!< The underlying mapping that `view` is a part of
        u64 blockMappingStartAddr; //!< The start GPU address of `blockMapping`
        u64 blockMappingEndAddr; //!< The end GPU address of `blockMapping`

      public:
        BufferView view; //!< The buffer view created as a result of a call to `Update()`

        /**
         * @brief Updates `view` based on the supplied GPU mapping
         */
        void Update(InterconnectContext &ctx, u64 address, u64 size, bool splitMappingWarn = true);

        /**
         * @brief Purges the cached block mapping so the next `Update()` call will perform a full lookup
         */
        void PurgeCaches();

        BufferView &operator*() {
            return view;
        }

        BufferView *operator->() {
            return &view;
        }
    };


    using DynamicBufferBinding = std::variant<BufferBinding, BufferView>;
    using DirtyManager = dirty::Manager<soc::gm20b::engine::EngineMethodsEnd * sizeof(u32), sizeof(u32)>;

    /**
     * @brief This is a stub for a future implementation
     */
    struct StateUpdateBuilder {
        void SetVertexBuffer(u32 index, BufferBinding *binding) {}

        void SetVertexBuffer(u32 index, BufferView &view) {
            view.GetBuffer()->BlockSequencedCpuBackingWrites();
        }

        void SetIndexBuffer(BufferBinding *binding, vk::IndexType indexType) {}

        void SetIndexBuffer(BufferView &view, vk::IndexType indexType) {
            view.GetBuffer()->BlockSequencedCpuBackingWrites();

        }

        void SetTransformFeedbackBuffer(u32 index, BufferView &view) {
            view.GetBuffer()->BlockSequencedCpuBackingWrites();

        }

        void SetTransformFeedbackBuffer(u32 index, BufferBinding *binding) {}

        void SetViewport(u32 index, const vk::Viewport &viewport) {}

        void SetScissor(u32 index, const vk::Rect2D &scissor) {}

        void SetLineWidth(float lineWidth) {}

        void SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {}

        void SetBlendConstants(const std::array<float, engine::BlendColorChannelCount> &blendConstants) {}

        void SetDepthBounds(float minDepthBounds, float maxDepthBounds) {}

        void SetBaseStencilState(vk::StencilFaceFlags flags, u32 funcRef, u32 funcMask, u32 mask) {}
    };
}