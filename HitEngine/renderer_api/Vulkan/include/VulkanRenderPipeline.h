#pragma once

#include "Renderer/RenderPipeline.h"

#include "VulkanCommon.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

#include "Utils/FastHandleList.h"
#include "Utils/Ref.h"

namespace hit
{
	constexpr ui16 vk_pipeline_max_descriptors_per_instance = 3;
	constexpr ui16 vk_max_textures_per_instance = 16;

	struct VulkanPipelineInstance
	{
		Ref<VulkanTexture> textures[vk_max_textures_per_instance];

		VkDescriptorSet sets[vk_pipeline_max_descriptors_per_instance];
		bool dirty[vk_pipeline_max_descriptors_per_instance];
	};

	struct VulkanPipelineSetConfig
	{
		VkDescriptorSetLayout layout;
		VkDescriptorPool pool;

		ui64 max_instances;
		ui64 uniforms_size;
		std::vector<ShaderUniform> uniforms;
		FastHandleList<VulkanPipelineInstance> instances;

		Scope<Buffer> buffer = nullptr;
	};

	class VulkanRenderPipeline : public RenderPipeline
	{
	public:
		VulkanRenderPipeline(const VulkanContext context) : m_context(context) { }
		~VulkanRenderPipeline() = default;

		bool create(const PipelineConfig& config) override;
		void destroy() override;

		bool push_constant(ShaderProgram::Type at, ui64 size, void* data) override;

		bool bind_pipeline() override;
		bool unbind_pipeline() override;

		PipelineInstance create_instance(ui32 bind) override;
		void destroy_instance(PipelineInstance& instance) override;

		bool bind_instance(const PipelineInstance& instance) override;
		bool unbind_instance(const PipelineInstance& instance) override;

		bool update_instance(const PipelineInstance& instance, ui64 offset, ui64 size, void* data) override;
		bool update_instance(const PipelineInstance& instance, ui64 offset, const Ref<Texture>& texture) override;

		bool has_uniform_data(ShaderProgram::Type at, const std::string& uniform_name) override;
		bool has_uniform_data(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name) override;

		ui64 get_uniform_data_location(ShaderProgram::Type at, const std::string& uniform_name) override;
		ui64 get_uniform_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name) override;

		inline const VkPipeline get_pipeline() const { return m_pipeline; }
		inline const VkPipelineLayout get_layout() const { return m_layout; }

	private:
		VulkanContext m_context;

		VkPipeline m_pipeline;
		VkPipelineLayout m_layout;

		std::vector<ui64> m_push_constant_sizes;
		std::vector<VulkanPipelineSetConfig> m_sets_configs;
	};
}