# Standard Hit Engine Shader

Shader {
    name: StandardShader
    pass: WorldPass

    # vertex attributes
    VertexInput 
    {
        v_position: vec3
        v_uv: vec2
    }

    # model uniform
    VertexPushConstant
    {
        model: mat4
    }

    # global uniform
    VertexUniform
    {
	    instances: 1

	    Layout {
	        projection: mat4
            view: mat4
	    }
    }

    # material uniform
    FragmentUniform
    {
        instances: 1000

	    Layout {
	    
	    }

        TexturesMaps {
            base_texture: texture
        }
    }

    Vertex::
    {
        #version 450

        layout(location = 0) in vec3 v_position;
        layout(location = 1) in vec2 v_uv;

        layout(location = 0) out vec2 out_uv;

        layout(set = 0, binding = 0) uniform global_uniform
        {
            mat4 projection;
            mat4 view;
        } global;

        layout(push_constant) uniform push_constants
        {
            mat4 model;
        } transform;

        void main()
        {
            out_uv = v_uv;
	        gl_Position = global.projection * global.view * transform.model * vec4(v_position, 1.0);
        }
    }

    Fragment::
    {
        #version 450

        layout(location = 0) out vec4 out_color;

        layout(location = 0) in vec2 in_uv;

	    layout(set = 1, binding = 0) uniform sampler2D base_texture;

        void main()
        {
            out_color = texture(base_texture, in_uv);
        }
    }
}