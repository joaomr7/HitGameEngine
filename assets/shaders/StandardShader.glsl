# Standard Hit Engine Shader

Shader {
    name: StandardShader
    pass: WorldPass

    # vertex attributes
    VertexInput 
    {
        v_position: vec3
        v_color: vec3
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
        instances: 0

	Layout {
	    
	}
    }

    Vertex::
    {
        #version 450

        layout(location = 0) in vec3 v_position;
        layout(location = 1) in vec3 v_color;
        layout(location = 2) in vec2 v_uv;

        layout(location = 0) out vec3 out_color;

        layout(set=0, binding=0) uniform global_uniform
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
            out_color = v_color;
	    gl_Position = global.projection * global.view * transform.model * vec4(v_position, 1.0);
        }
    }

    Fragment::
    {
        #version 450

        layout(location = 0) out vec4 out_color;
        layout(location = 0) in vec3 in_color;

        void main()
        {
            out_color = vec4(in_color, 1.0);
        }
    }
}