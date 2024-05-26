#pragma once

#include "Core/Types.h"
#include "Math/Math.h"
#include "Renderer/Shader.h"

namespace hit
{
	class StandardShader : public Shader
	{
	public:
		StandardShader(Renderer* renderer) : Shader(renderer) { }

		bool create(const Ref<Renderpass>& pass) override;
		void destroy() override;

		Ref<ShaderAttribute> create_program_attribute(ShaderProgram::Type program_type) override;
		void destroy_program_attribute(const Ref<ShaderAttribute>& attribute) override;
	};

	class StandardGlobalData : public ShaderAttribute
	{
	public:
		bool create_resources() override;
		void release_resources() override;

		void set_view_matrix(const Mat4& view_matrix);
		void set_projection_matrix(const Mat4& projection_matrix);

	private:
		ui64 m_view_matrix_location;
		Mat4 m_view_matrix;

		ui64 m_projection_matrix_location;
		Mat4 m_projection_matrix;
	};

	class StandardMaterial : public ShaderAttribute
	{
	public:
		bool create_resources() override;
		void release_resources() override;

		void set_base_color(const Vec4& base_color);

	private:
		ui64 m_base_color_location;
		Vec4 m_base_color;
	};
}