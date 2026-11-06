#version 330

uniform vec4 color;
uniform sampler2D tex;

in vec2 texCoordFrag;
flat in vec4 frameBounds;
out vec4 outColor;

void main()
{
	// Discard fragment if texture sample has alpha < 0.5
	// otherwise compose the texture sample with the fragment's interpolated color
	// Clamp sampling, not the UV span, so edge pixels keep their full width.
	vec2 halfTexel = 0.5f / vec2(textureSize(tex, 0));
	vec2 sampleCoord = clamp(texCoordFrag, frameBounds.xy + halfTexel, frameBounds.zw - halfTexel);
	vec4 texColor = texture(tex, sampleCoord);
	if(texColor.a < 0.5f)
		discard;
	outColor = color * texColor;
}

