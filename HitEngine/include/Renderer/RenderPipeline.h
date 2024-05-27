#pragma once

#include "ShaderTypes.h"
#include "Renderpass.h"
#include "Utils/Ref.h"
#include "Utils/FastHandleList.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace hit
{
	enum class PipelineCullMode : ui8
	{
		None, Front, Back, FrontAndBack
	};

	enum class PipelinePolygon : ui8
	{
		Fill, Line, Point
	};

	enum class PipelineTopology : ui8
	{
		PointList,
		LineList, LineStrip,
		TriangleList, TriangleStrip, TriangleFan,
		LineListWithAdjacency, TriangleListWithAdjacency,
		TraingleStripWithAdjacency,
		PatchList
	};

	struct PipelineColorBlending
	{
		enum Logical : ui8
		{
			LogicalClear, LogicalCopy, LogicalCopyInverted,
			LogicalAnd, LogicalAndReverse, LogicalAndInverted,
			LogicalOr, LogicalOrReverse, LogicalOrInverted,
			LogicalNand, LogicalNor, LogicalXor,
			LogicalEquivalent, LogicalSet, LogicalInvert,
			LogicalNoOp
		};

		enum Op : ui8
		{
			OpAdd, OpSub, OpRevSub, OpMin, OpMax,
		};

		enum Factor : ui8
		{
			FactorZero, FactorOne,
			FactorSrcColor, FactorSrcAlpha,
			FactorDestColor, FactorDestAlpha,
			FactorOneMinusSrcColor, FactorOneMinusDestColor,
			FactorOneMinusSrcAlpha, FactorOneMinusDestAlpha,
			FactorSrcAlphaSaturate
		};

		enum Mask : ui8
		{
			MaskN = 0x0, // none
			MaskR = 0x1,
			MaskG = 0x2,
			MaskB = 0x4,
			MaskA = 0x8,
			MaskAll = MaskR | MaskG | MaskB | MaskA
		};

		bool use_logical;
		Logical logical_op;

		bool use_blend;
		ui8 mask;

		Op color_blend_op;
		Factor src_color_factor;
		Factor dest_color_factor;

		Op alpha_blend_op;
		Factor src_alpha_factor;
		Factor dest_alpha_factor;
	};

	struct PipelineConfig
	{
		Ref<Renderpass> pass;
		std::vector<ShaderProgram> programs;

		PipelineColorBlending color_blend;

		PipelineTopology topology;
		PipelinePolygon polygon;
		PipelineCullMode cull_mode;

		bool front_face_clockwise;
		bool write_depth;
		bool use_depth;
	};

	using PipelineHandle = Handle<struct internal_pipeline_handle>;

	struct PipelineInstance
	{
		PipelineInstance(PipelineHandle handle = PipelineHandle(), i32 bind = -1) : handle(handle), bind(bind) { }

		PipelineHandle handle;
		i32 bind;

		inline bool is_valid() const { return handle.valid() && bind >= 0; }
	};

	class RenderPipeline
	{
	public:
		virtual ~RenderPipeline() = default;

		virtual bool create(const PipelineConfig& config) = 0;
		virtual void destroy() = 0;

		virtual bool push_constant(ShaderProgram::Type at, ui64 size, void* data) = 0;

		virtual bool bind_pipeline() = 0;
		virtual bool unbind_pipeline() = 0;

		virtual PipelineInstance create_instance(ui32 bind) = 0;
		virtual void destroy_instance(PipelineInstance& instance) = 0;

		virtual bool bind_instance(const PipelineInstance& instance) = 0;
		virtual bool unbind_instance(const PipelineInstance& instance) = 0;

		virtual bool update_instance(const PipelineInstance& instance, ui64 offset, ui64 size, void* data) = 0;
		virtual bool update_instance(const PipelineInstance& instance, ui64 offset, const Ref<Texture>& texture) = 0;

		virtual bool has_uniform_data(ShaderProgram::Type at, const std::string& uniform_name) = 0;
		virtual bool has_uniform_data(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name) = 0;
		
		virtual ui64 get_uniform_data_location(ShaderProgram::Type at, const std::string& uniform_name) = 0;
		virtual ui64 get_uniform_data_location(ShaderProgram::Type at, const std::string& uniform_name, const std::string& data_name) = 0;
	};
}