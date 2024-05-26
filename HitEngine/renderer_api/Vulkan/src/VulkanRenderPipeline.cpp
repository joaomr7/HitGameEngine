#include "VulkanRenderPipeline.h"
#include "VulkanRenderpass.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"

#include "Core/Log.h"

#include <vector>
#include <utility>
#include <algorithm>

namespace hit::vulkan_helper
{
	VkShaderStageFlagBits program_type_to_vulkan_stage(ShaderProgram::Type type)
	{
		switch(type)
		{
			case hit::ShaderProgram::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
			case hit::ShaderProgram::Fragment:	return VK_SHADER_STAGE_FRAGMENT_BIT;
			default: hit_assert(false, "Invalid shader program type!");
		}
	}

	VkShaderModule create_shader_module(const VulkanDevice* device, const std::vector<ui32>& code)
	{
		VkShaderModuleCreateInfo module_info{ };
		module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_info.codeSize = code.size() * sizeof(ui32);
		module_info.pCode = code.data();

		VkShaderModule shader_module;
		if(!check_vk_result(vkCreateShaderModule(device->get_device(), &module_info, device->get_alloc_callback(), &shader_module)))
		{
			hit_error("Failed to create vulkan shader module!");
			return nullptr;
		}

		return shader_module;
	}

	VkPipelineShaderStageCreateInfo create_shader_stage_info(ShaderProgram::Type program_type, VkShaderModule shader_module)
	{
		VkPipelineShaderStageCreateInfo stage_info{ };
		stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info.stage = program_type_to_vulkan_stage(program_type);
		stage_info.module = shader_module;
		stage_info.pName = "main";

		return stage_info;
	}

	VkFormat shader_data_type_to_vk_format(ShaderData::Type type)
	{
		switch(type)
		{
			case ShaderData::Bool:
			case ShaderData::Int:		return VK_FORMAT_R32_SINT;

			case ShaderData::Int2:		return VK_FORMAT_R32G32_SINT;
			case ShaderData::Int3:		return VK_FORMAT_R32G32B32_SINT;
			case ShaderData::Int4:		return VK_FORMAT_R32G32B32A32_SINT;

			case ShaderData::Float:		return VK_FORMAT_R32_SFLOAT;
			case ShaderData::Float2:	return VK_FORMAT_R32G32_SFLOAT;
			case ShaderData::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderData::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;

			default:					return VK_FORMAT_UNDEFINED;
		}
	}

	VkPrimitiveTopology shader_toppology_to_vk_topology(PipelineTopology toppology)
	{
		switch(toppology)
		{
			case PipelineTopology::PointList:					return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PipelineTopology::LineList:					return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PipelineTopology::LineStrip:					return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PipelineTopology::TriangleList:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PipelineTopology::TriangleStrip:				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PipelineTopology::TriangleFan:					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			case PipelineTopology::LineListWithAdjacency:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			case PipelineTopology::TriangleListWithAdjacency:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			case PipelineTopology::TraingleStripWithAdjacency:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			case PipelineTopology::PatchList:					return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			default:											return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	VkPolygonMode shader_polygon_to_vk_polygon_mode(PipelinePolygon polygon)
	{
		switch(polygon)
		{
			case PipelinePolygon::Fill:		return VK_POLYGON_MODE_FILL;
			case PipelinePolygon::Line:		return VK_POLYGON_MODE_LINE;
			case PipelinePolygon::Point:	return VK_POLYGON_MODE_POINT;
			default:						return VK_POLYGON_MODE_FILL;
		}
	}

	VkCullModeFlagBits shader_cull_mode_to_vk_cull_mode(PipelineCullMode cull_mode)
	{
		switch(cull_mode)
		{
			case PipelineCullMode::None:			return VK_CULL_MODE_NONE;
			case PipelineCullMode::Front:			return VK_CULL_MODE_FRONT_BIT;
			case PipelineCullMode::Back:			return VK_CULL_MODE_BACK_BIT;
			case PipelineCullMode::FrontAndBack:	return VK_CULL_MODE_FRONT_AND_BACK;
			default:								return VK_CULL_MODE_NONE;
		}
	}

	VkLogicOp logical_to_vulkan_logic_op(PipelineColorBlending::Logical logical)
	{
		switch(logical)
		{
			case PipelineColorBlending::LogicalClear:			return VK_LOGIC_OP_CLEAR;
			case PipelineColorBlending::LogicalAnd:				return VK_LOGIC_OP_AND;
			case PipelineColorBlending::LogicalAndReverse:		return VK_LOGIC_OP_AND_REVERSE;
			case PipelineColorBlending::LogicalCopy:			return VK_LOGIC_OP_COPY;
			case PipelineColorBlending::LogicalAndInverted:		return VK_LOGIC_OP_AND_INVERTED;
			case PipelineColorBlending::LogicalNoOp:			return VK_LOGIC_OP_NO_OP;
			case PipelineColorBlending::LogicalXor:				return VK_LOGIC_OP_XOR;
			case PipelineColorBlending::LogicalOr:				return VK_LOGIC_OP_OR;
			case PipelineColorBlending::LogicalNor:				return VK_LOGIC_OP_NOR;
			case PipelineColorBlending::LogicalEquivalent:		return VK_LOGIC_OP_EQUIVALENT;
			case PipelineColorBlending::LogicalInvert:			return VK_LOGIC_OP_INVERT;
			case PipelineColorBlending::LogicalOrReverse:		return VK_LOGIC_OP_OR_REVERSE;
			case PipelineColorBlending::LogicalCopyInverted:	return VK_LOGIC_OP_COPY_INVERTED;
			case PipelineColorBlending::LogicalOrInverted:		return VK_LOGIC_OP_OR_INVERTED;
			case PipelineColorBlending::LogicalNand:			return VK_LOGIC_OP_NAND;
			case PipelineColorBlending::LogicalSet:				return VK_LOGIC_OP_SET;
			default:											return VK_LOGIC_OP_NO_OP;
		}
	}

	VkBlendOp blend_op_to_vulkan_blend_op(PipelineColorBlending::Op op)
	{
		switch(op)
		{
			case PipelineColorBlending::OpAdd:		return VK_BLEND_OP_ADD;
			case PipelineColorBlending::OpSub:		return VK_BLEND_OP_SUBTRACT;
			case PipelineColorBlending::OpRevSub:	return VK_BLEND_OP_REVERSE_SUBTRACT;
			case PipelineColorBlending::OpMin:		return VK_BLEND_OP_MIN;
			case PipelineColorBlending::OpMax:		return VK_BLEND_OP_MAX;
			default:								return VK_BLEND_OP_ADD;
		}
	}

	VkBlendFactor blend_factor_to_vulkan_blend_factor(PipelineColorBlending::Factor factor)
	{
		switch(factor)
		{
			case PipelineColorBlending::FactorZero:					return VK_BLEND_FACTOR_ZERO;
			case PipelineColorBlending::FactorOne:					return VK_BLEND_FACTOR_ONE;
			case PipelineColorBlending::FactorSrcColor:				return VK_BLEND_FACTOR_SRC_COLOR;
			case PipelineColorBlending::FactorOneMinusSrcColor:		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case PipelineColorBlending::FactorDestColor:			return VK_BLEND_FACTOR_DST_COLOR;
			case PipelineColorBlending::FactorOneMinusDestColor:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case PipelineColorBlending::FactorSrcAlpha:				return VK_BLEND_FACTOR_SRC_ALPHA;
			case PipelineColorBlending::FactorOneMinusSrcAlpha:		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case PipelineColorBlending::FactorDestAlpha:			return VK_BLEND_FACTOR_DST_ALPHA;
			case PipelineColorBlending::FactorOneMinusDestAlpha:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case PipelineColorBlending::FactorSrcAlphaSaturate:		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			default:												return VK_BLEND_FACTOR_ZERO;
		}
	}

	VkDescriptorType uniform_type_to_vulkan_descriptor_type(ShaderUniform::Type type)
	{
		switch(type)
		{
			case hit::ShaderUniform::ImageSampler:		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case hit::ShaderUniform::UniformBuffer:		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			default:
				hit_assert(false, "Invalid uniform type.");
		}
	}
}

namespace hit
{
	bool VulkanRenderPipeline::create(const PipelineConfig& config)
	{
		auto device = m_context->get_device();
		auto vk_renderpass = cast_ref<VulkanRenderpass>(config.pass);

		// create shader programs
		std::vector<VkShaderModule> shader_modules;
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos;

		// safe delete allocated resources, use just if this method fail
		auto safe_deleter = [&]()
		{
			// destroy shader modules
			for (auto& shader_module : shader_modules)
			{
				vkDestroyShaderModule(device->get_device(), shader_module, device->get_alloc_callback());
				shader_module = nullptr;
			}
			shader_modules.clear();
			
			// destroy already allocated resources
			destroy();
		};

		// sort programs by the type of program
		// vertex < fragment < ...
		auto programs = config.programs;
		std::ranges::sort(programs, [](auto& e1, auto& e2)
		{
			return e1.type < e2.type;
		});

		for(auto& program : programs)
		{
			// create shader modules
			auto shader_module = vulkan_helper::create_shader_module(device, program.source);
			if(!shader_module)
			{
				safe_deleter();

				hit_error("Failed to create pipeline module!");
				return false;
			}
			shader_modules.push_back(shader_module);

			// create stage info
			shader_stage_infos.push_back(vulkan_helper::create_shader_stage_info(program.type, shader_module));
		}

		// dynamic state
		VkPipelineDynamicStateCreateInfo dynamic_state_info{ };
		dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

		// TODO: check for other dynamic states in pipeline config
		std::vector<VkDynamicState> dynamic_states =
		{
			// default dynamic states
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		dynamic_state_info.dynamicStateCount = dynamic_states.size();
		dynamic_state_info.pDynamicStates = dynamic_states.data();
		
		// vertex input state
		VkPipelineVertexInputStateCreateInfo vertex_input_info{ };
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		// create vertex input descs
		std::vector<VkVertexInputBindingDescription> vertex_input_binding_descs;
		std::vector<VkVertexInputAttributeDescription> vertex_input_attributes_descs;
		for(auto& program : programs)
		{
			// Find vertex shader
			if(program.type == ShaderProgram::Vertex && program.attributes.elem_count() > 0)
			{
				// binding description
				VkVertexInputBindingDescription binding_desc { };
				binding_desc.binding = 0;
				binding_desc.stride = program.attributes.stride();
				binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				vertex_input_binding_descs.push_back(binding_desc);

				// attribute description
				for(ui32 location = 0; auto & attribute : program.attributes)
				{
					VkVertexInputAttributeDescription desc { };
					desc.binding = 0;
					desc.location = location++;
					desc.format = vulkan_helper::shader_data_type_to_vk_format(attribute.type);
					desc.offset = attribute.offset;

					vertex_input_attributes_descs.push_back(desc);
				}

				vertex_input_info.vertexBindingDescriptionCount = vertex_input_binding_descs.size();
				vertex_input_info.pVertexBindingDescriptions = vertex_input_binding_descs.data();

				vertex_input_info.vertexAttributeDescriptionCount = vertex_input_attributes_descs.size();
				vertex_input_info.pVertexAttributeDescriptions = vertex_input_attributes_descs.data();

				break;
			}
		}

		// input assembly
		VkPipelineInputAssemblyStateCreateInfo assembly_info{ };
		assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_info.topology = vulkan_helper::shader_toppology_to_vk_topology(config.topology);
		assembly_info.primitiveRestartEnable = VK_FALSE;

		// viewport
		VkPipelineViewportStateCreateInfo viewport_info{ };
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.scissorCount = 1;

		// rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer_info{ };
		rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer_info.depthClampEnable = VK_FALSE;
		rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
		rasterizer_info.polygonMode = vulkan_helper::shader_polygon_to_vk_polygon_mode(config.polygon);
		rasterizer_info.cullMode = vulkan_helper::shader_cull_mode_to_vk_cull_mode(config.cull_mode);
		rasterizer_info.frontFace = config.front_face_clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer_info.depthBiasEnable = VK_FALSE;
		rasterizer_info.depthBiasConstantFactor = 0.0f;
		rasterizer_info.depthBiasClamp = 0.0f;
		rasterizer_info.depthBiasSlopeFactor = 0.0f;
		rasterizer_info.lineWidth = 1.0f;

		// multisampling (TODO: add full support)
		VkPipelineMultisampleStateCreateInfo multisample_info{ };
		multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_info.sampleShadingEnable = VK_FALSE;
		multisample_info.minSampleShading = 1.0f;
		multisample_info.pSampleMask = VK_NULL_HANDLE;
		multisample_info.alphaToCoverageEnable = VK_FALSE;
		multisample_info.alphaToOneEnable = VK_FALSE;

		// depth stencil state
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info{ };
		depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_info.depthTestEnable = config.use_depth ? VK_TRUE : VK_FALSE;
		depth_stencil_info.depthWriteEnable = config.write_depth ? VK_TRUE : VK_FALSE;
		depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_info.minDepthBounds = 0.0f;
		depth_stencil_info.maxDepthBounds = 1.0f;

		// color blend
		VkPipelineColorBlendStateCreateInfo color_blend_info{ };
		color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_info.logicOpEnable = config.color_blend.use_logical ? VK_TRUE : VK_FALSE;
		color_blend_info.logicOp = vulkan_helper::logical_to_vulkan_logic_op(config.color_blend.logical_op);

		//create color blending attachment
		VkPipelineColorBlendAttachmentState color_blend_attach{};
		color_blend_attach.blendEnable = config.color_blend.use_blend ? VK_TRUE : VK_FALSE;
		color_blend_attach.colorWriteMask = config.color_blend.mask;

		color_blend_attach.srcColorBlendFactor = vulkan_helper::blend_factor_to_vulkan_blend_factor(config.color_blend.src_color_factor);
		color_blend_attach.dstColorBlendFactor = vulkan_helper::blend_factor_to_vulkan_blend_factor(config.color_blend.dest_color_factor);
		color_blend_attach.colorBlendOp = vulkan_helper::blend_op_to_vulkan_blend_op(config.color_blend.color_blend_op);

		color_blend_attach.srcAlphaBlendFactor = vulkan_helper::blend_factor_to_vulkan_blend_factor(config.color_blend.src_alpha_factor);
		color_blend_attach.dstAlphaBlendFactor = vulkan_helper::blend_factor_to_vulkan_blend_factor(config.color_blend.dest_alpha_factor);
		color_blend_attach.alphaBlendOp = vulkan_helper::blend_op_to_vulkan_blend_op(config.color_blend.alpha_blend_op);

		color_blend_info.attachmentCount = 1;
		color_blend_info.pAttachments = &color_blend_attach;

		// get swapchin image count
		ui64 image_count = m_context->get_swapchain()->get_image_count();

		// pipeline layout
		std::vector<VkPushConstantRange> push_constant_ranges;
		std::vector<VkDescriptorSetLayout> descriptor_sets_layouts;
		for(auto& program : programs)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			ui64 bind = 0;

			bool has_empty_buffer = false;
			for(auto& uniform : program.uniforms)
			{
				if (uniform.layout.stride() == 0)
				{
					has_empty_buffer = true;
					continue;
				}

				if(uniform.is_push_constant())
				{
					VkPushConstantRange push_constant{ };
					push_constant.offset = 0;
					push_constant.size = uniform.layout.stride();
					push_constant.stageFlags = vulkan_helper::program_type_to_vulkan_stage(program.type);

					push_constant_ranges.push_back(push_constant);
					m_push_constant_sizes.push_back(uniform.layout.stride());
				}
				else
				{
					VkDescriptorSetLayoutBinding binding;
					binding.binding = bind++;
					binding.descriptorCount = 1;
					binding.descriptorType = vulkan_helper::uniform_type_to_vulkan_descriptor_type(uniform.type);
					binding.stageFlags = vulkan_helper::program_type_to_vulkan_stage(program.type);

					bindings.push_back(binding);
				}
			}

			if (has_empty_buffer)
				continue;

			if(!bindings.empty())
			{
				VkDescriptorSetLayoutCreateInfo layout_info{ };
				layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout_info.bindingCount = bindings.size();
				layout_info.pBindings = bindings.data();

				m_sets_configs.emplace_back();
				auto& set_config = m_sets_configs.back();
				
				// create descriptor layout
				if (!check_vk_result(vkCreateDescriptorSetLayout(device->get_device(), &layout_info, device->get_alloc_callback(), &set_config.layout)))
				{
					safe_deleter();

					hit_error("Failed to create uniform layout.");
					return false;
				}

				// descriptor pool 
				bool create_buffer = false;
				ui64 uniform_buffer_size = 0;
				std::vector<VkDescriptorPoolSize> pool_sizes;
				for (auto& uniform : program.uniforms)
				{
					if (uniform.layout.stride() == 0)
						continue;

					if (uniform.type == ShaderUniform::PushConstant)
						continue;

					VkDescriptorPoolSize pool_size;
					pool_size.type = vulkan_helper::uniform_type_to_vulkan_descriptor_type(uniform.type);
					pool_size.descriptorCount = 1;

					pool_sizes.push_back(pool_size);

					if (uniform.is_uniform_buffer())
					{
						uniform_buffer_size += uniform.layout.stride();
						create_buffer = true;
					}
				}

				// create descriptor pool
				VkDescriptorPoolCreateInfo pool_info { };
				pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
				pool_info.maxSets = image_count * program.max_uniforms;
				pool_info.poolSizeCount = pool_sizes.size();
				pool_info.pPoolSizes = pool_sizes.data();

				set_config.max_instances = program.max_uniforms;

				if (!check_vk_result(vkCreateDescriptorPool(device->get_device(), &pool_info, device->get_alloc_callback(), &set_config.pool)))
				{
					safe_deleter();

					hit_error("Failed to create vulkan descriptor pool.");
					return false;
				}

				// create buffer
				if (create_buffer)
				{
					set_config.buffer = create_scope<VulkanBuffer>(m_context);

					ui64 buffer_size = uniform_buffer_size * image_count * set_config.max_instances;
					if (!set_config.buffer->create(buffer_size, BufferType::Uniform, BufferAllocationType::None))
					{
						safe_deleter();

						hit_error("Failed to create uniform buffers.");
						return false;
					}
				}

				set_config.uniforms_size = uniform_buffer_size;
				set_config.uniforms = program.uniforms;
				descriptor_sets_layouts.push_back(set_config.layout);
			}
		}

		VkPipelineLayoutCreateInfo layout_info{ };
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.setLayoutCount = descriptor_sets_layouts.size();
		layout_info.pSetLayouts = descriptor_sets_layouts.data();
		layout_info.pushConstantRangeCount = push_constant_ranges.size();
		layout_info.pPushConstantRanges = push_constant_ranges.data();

		if(!check_vk_result(
			vkCreatePipelineLayout(device->get_device(), &layout_info, device->get_alloc_callback(), &m_layout)))
		{
			safe_deleter();

			hit_error("Failed to create pipeline layout.");
			return false;
		}

		// pipeline
		VkGraphicsPipelineCreateInfo pipeline_info{ };
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = shader_stage_infos.size();
		pipeline_info.pStages = shader_stage_infos.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &assembly_info;
		pipeline_info.pDynamicState = &dynamic_state_info;
		pipeline_info.pViewportState = &viewport_info;
		pipeline_info.pRasterizationState = &rasterizer_info;
		pipeline_info.pMultisampleState = &multisample_info;
		pipeline_info.pDepthStencilState = &depth_stencil_info;
		pipeline_info.pColorBlendState = &color_blend_info;
		pipeline_info.layout = m_layout;
		pipeline_info.renderPass = vk_renderpass->get_pass();
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		if(!check_vk_result(
			vkCreateGraphicsPipelines(device->get_device(), VK_NULL_HANDLE, 1, &pipeline_info, device->get_alloc_callback(), &m_pipeline)))
		{
			safe_deleter();

			hit_error("Failed to create pipeline.");
			return false;
		}

		// destroy shader modules
		for(auto& shader_module : shader_modules)
		{
			vkDestroyShaderModule(device->get_device(), shader_module, device->get_alloc_callback());
			shader_module = nullptr;
		}
		shader_modules.clear();

		return true;
	}

	void VulkanRenderPipeline::destroy()
	{
		auto device = m_context->get_device();

		device->wait_idle();

		if (m_pipeline)
		{
			vkDestroyPipeline(device->get_device(), m_pipeline, device->get_alloc_callback());
			m_pipeline = VK_NULL_HANDLE;
		}

		if (m_layout)
		{
			vkDestroyPipelineLayout(device->get_device(), m_layout, device->get_alloc_callback());
			m_layout = VK_NULL_HANDLE;
		}

		for (auto& set_config : m_sets_configs)
		{
			if (set_config.layout)
			{
				vkDestroyDescriptorSetLayout(device->get_device(), set_config.layout, device->get_alloc_callback());
			}

			if (set_config.pool)
			{
				vkDestroyDescriptorPool(device->get_device(), set_config.pool, device->get_alloc_callback());
			}

			if (set_config.buffer)
			{
				set_config.buffer->destroy();
				set_config.buffer = nullptr;
			}
		}

		m_sets_configs.clear();
		m_push_constant_sizes.clear();
	}

	bool VulkanRenderPipeline::push_constant(ShaderProgram::Type at, ui64 size, void* data)
	{
		if (!data)
		{
			hit_error("Can't push null data to push constant.");
			return false;
		}

		if (m_push_constant_sizes.empty())
		{
			hit_error("There is no push constant.");
			return false;
		}

		if ((ui64)at >= m_push_constant_sizes.size())
		{
			hit_error("Out of bound push constant.");
			return false;
		}

		auto push_size = m_push_constant_sizes[(ui64)at];
		if (push_size != size)
		{
			hit_error("Invalid push constant size. Actual size is {} bytes, given {} bytes.", push_size, size);
		}

		auto command = m_context->get_graphics_command().get_command_buffer();
		vkCmdPushConstants(command, m_layout, vulkan_helper::program_type_to_vulkan_stage(at), 0, size, data);

		return true;
	}

	bool VulkanRenderPipeline::bind_pipeline()
	{
		if (!m_pipeline)
		{
			hit_fatal("Vulkan pipeline is null.");
			return false;
		}

		auto command = m_context->get_graphics_command().get_command_buffer();
		vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		return true;
	}

	bool VulkanRenderPipeline::unbind_pipeline()
	{
		return true;
	}

	PipelineInstance VulkanRenderPipeline::create_instance(ui32 bind)
	{
		auto device = m_context->get_device();

		if (m_sets_configs.empty() || bind >= m_sets_configs.size())
		{
			hit_error("Pipeline has no uniform set to be instantiated at bind {}.", bind);
			return PipelineInstance();
		}

		auto& instance_list = m_sets_configs[bind].instances;
		if (instance_list.size() >= m_sets_configs[bind].max_instances)
		{
			hit_error("Max instances to bind {} reached. Can't create new instance.", bind);
			return PipelineInstance();
		}

		auto new_instance = instance_list.emplace();
		auto vk_instance = instance_list.get(new_instance);
		auto out_instance = PipelineInstance(new_instance, (i32)bind);

		if (!vk_instance)
		{
			hit_error("Failed to allocate new pipeline instance.");
			return PipelineInstance();
		}

		// initialize instance
		ui64 image_count = m_context->get_swapchain()->get_image_count();
		for (ui64 i = 0; i < image_count; i++)
		{
			VkDescriptorSetAllocateInfo set_info { };
			set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			set_info.descriptorPool = m_sets_configs[bind].pool;
			set_info.descriptorSetCount = 1;
			set_info.pSetLayouts = &m_sets_configs[bind].layout;

			if (!check_vk_result(vkAllocateDescriptorSets(device->get_device(), &set_info, &vk_instance->sets[i])))
			{
				destroy_instance(out_instance);

				hit_error("Failed to create new pipeline instance.");
				return PipelineInstance();
			}

			vk_instance->dirty[i] = true;
		}

		return out_instance;
	}

	void VulkanRenderPipeline::destroy_instance(PipelineInstance& instance)
	{
		auto device = m_context->get_device();

		device->wait_idle();

		if (!instance.is_valid())
		{
			hit_error("Can't destroy invalid pipeline instance!");
			return;
		}

		if (m_sets_configs.empty() || instance.bind >= m_sets_configs.size())
		{
			hit_error("Pipeline has no uniform set to be destroyed at bind {}.", instance.bind);
			return;
		}

		auto& instance_list = m_sets_configs[instance.bind].instances;
		Handle<VulkanPipelineInstance> vk_instance = instance.handle;

		auto intern_instance = instance_list.get(vk_instance);

		if (!intern_instance)
		{
			hit_error("Internal pipeline instance is null!");
			return;
		}

		// get swapchin image count
		ui64 image_count = m_context->get_swapchain()->get_image_count();

		// destroy instance
		for (ui64 i = 0; i < image_count; i++)
		{
			if (!check_vk_result(vkFreeDescriptorSets(device->get_device(), m_sets_configs[instance.bind].pool, 1, &intern_instance->sets[i])))
			{
				hit_fatal("Failed to free descriptor set!");
			}

			intern_instance->sets[i] = VK_NULL_HANDLE;
			intern_instance->dirty[i] = true;
		}

		instance_list.remove(vk_instance);
		instance = PipelineInstance();
	}

	bool VulkanRenderPipeline::bind_instance(const PipelineInstance & instance)
	{
		auto device = m_context->get_device();

		if (!instance.is_valid())
		{
			hit_error("Can't bind invalid pipeline instance!");
			return false;
		}

		if (m_sets_configs.empty() || instance.bind >= m_sets_configs.size())
		{
			hit_error("Pipeline has no uniform set to be bind at bind {}.", instance.bind);
			return false;
		}

		auto& instance_list = m_sets_configs[instance.bind].instances;
		Handle<VulkanPipelineInstance> vk_instance = instance.handle;

		auto intern_instance = instance_list.get(vk_instance);

		if (!intern_instance)
		{
			hit_error("Internal pipeline instance is null!");
			return false;
		}

		// get swapchin image count
		ui64 current_frame = m_context->get_current_frame();
		auto vk_set = intern_instance->sets[current_frame];

		// update set if it's dirty, only once per frame
		if (intern_instance->dirty[current_frame])
		{
			std::vector<VkDescriptorBufferInfo> buffer_infos;
			std::vector<VkWriteDescriptorSet> writes;

			auto vk_buffer = (VulkanBuffer*)m_sets_configs[instance.bind].buffer.get();
			vk_buffer->bind();

			for (ui64 binding = 0; auto & uniform : m_sets_configs[instance.bind].uniforms)
			{
				if (uniform.is_push_constant())
					continue;

				VkWriteDescriptorSet write { };
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = vk_set;
				write.dstBinding = binding++;
				write.dstArrayElement = 0;
				write.descriptorCount = 1;

				if (uniform.is_uniform_buffer())
				{
					VkDescriptorBufferInfo buffer_info;
					buffer_info.buffer = vk_buffer->get_buffer();
					buffer_info.offset = 0;
					buffer_info.range = uniform.layout.stride();

					buffer_infos.push_back(buffer_info);

					write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					write.pBufferInfo = &buffer_infos.back();
				}
				else if (uniform.is_image_sampler())
				{
					hit_warning("Global image sampler not supported yet");
					continue;
				}
				else
				{
					hit_assert(false, "Updating invalid uniform type");
				}

				writes.push_back(write);
			}

			vkUpdateDescriptorSets(device->get_device(), writes.size(), writes.data(), 0, nullptr);

			intern_instance->dirty[current_frame] = false;
		}

		auto command = m_context->get_graphics_command().get_command_buffer();
		vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, 1, &vk_set, 0, nullptr);

		return true;
	}

	bool VulkanRenderPipeline::unbind_instance(const PipelineInstance& instance)
	{
		return true;
	}

	bool VulkanRenderPipeline::update_instance(const PipelineInstance& instance, ui64 offset, ui64 size, void* data)
	{
		auto device = m_context->get_device();

		if (!instance.is_valid())
		{
			hit_error("Can't update invalid pipeline instance!");
			return false;
		}

		if (m_sets_configs.empty() || instance.bind >= m_sets_configs.size())
		{
			hit_error("Pipeline has no uniform set to be updated at bind {}.", instance.bind);
			return false;
		}

		auto& instance_list = m_sets_configs[instance.bind].instances;
		Handle<VulkanPipelineInstance> vk_instance = instance.handle;

		auto intern_instance = instance_list.get(vk_instance);

		if (!intern_instance)
		{
			hit_error("Internal pipeline instance is null!");
			return false;
		}

		// get swapchin image count
		ui64 image_count = m_context->get_swapchain()->get_image_count();

		ui64 instance_offset = m_sets_configs[instance.bind].uniforms_size * instance.handle.index;
		if (!m_sets_configs[instance.bind].buffer->load(offset, size, data))
		{
			hit_fatal("Failed to update pipeline instance buffer.");
			return false;
		}

		// invalidate sets
		for (auto& d : intern_instance->dirty) d = true;
		
		return true;
	}

	bool VulkanRenderPipeline::has_uniform_data(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name)
	{
		if ((ui64)at >= m_sets_configs.size())
		{
			return false;
		}

		for (auto& uniform : m_sets_configs[(ui64)at].uniforms)
		{
			if (uniform.name == uniform_name)
			{
				return uniform.layout.has_data(data_name);
			}
		}

		return false;
	}

	ui64 VulkanRenderPipeline::get_uniform_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name)
	{
		hit_assert((ui64)at < m_sets_configs.size(), "Out of bounds program!");

		for (auto& uniform : m_sets_configs[(ui64)at].uniforms)
		{
			if (uniform.name == uniform_name)
			{
				return uniform.layout.get_data(data_name).offset;
			}
		}

		hit_assert(false, "Can't find uniform '{}', data '{}'!", uniform_name, data_name);
		return 0;
	}

}