#version 150

in vec2 fragTC;
out vec4 color;
uniform sampler2D tex;

#if 0
void main(void)
{
   color = texture(tex, fragTC);
}
#else
void main()
{
   // retrieve distance from texture
   float mask = texture(tex, fragTC).a;
   
   // use current drawing color
   vec4 clr;
   clr.rgb = texture(tex, fragTC).rgb;
   
   // perform simple thresholding
   if( mask < 0.5 )
   {
      clr.a = 0.0;
   }
   else
   {
      clr.a = 1.0;
   }
   // do some anti-aliasing
   clr.a *= smoothstep(0.50, 0.75, mask);
   
   // final color
   color = vec4(clr.a);
}
#endif
