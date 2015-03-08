!!ARBfp1.0
# cgc version 3.1.0013, build date Apr 18 2012
# command line args: -oglsl -profile arbfp1
# source file: tilemap.frag
#vendor NVIDIA Corporation
#version 3.1.0.13
#profile arbfp1
#program main
#semantic tileset
#semantic indices
#semantic colors
#semantic bgcolors
#semantic tileset_size
#semantic size
#var float4 gl_FragCoord : $vin.WPOS : WPOS : -1 : 1
#var float4 gl_FragColor : $vout.COLOR : COL : -1 : 1
#var sampler2D tileset :  : texunit 3 : -1 : 1
#var sampler2D indices :  : texunit 0 : -1 : 1
#var sampler2D colors :  : texunit 1 : -1 : 1
#var sampler2D bgcolors :  : texunit 2 : -1 : 1
#var float2 tileset_size :  : c[0] : -1 : 1
#var float2 size :  : c[1] : -1 : 1
#var float2 tex_coord :  :  : -1 : 0
#const c[2] = 0.0020000001 1 0.00125 256
PARAM c[3] = { 
	program.local[0..1],
	{ 0.0020000001, 1, 0.00125, 256 }
};
TEMP R0;
TEMP R1;
TEMP R2;
TEMP R3;

# R2 := normalized screen coords

MAD R2.z, -fragment.position.y, c[2].x, c[2].y;
MUL R2.x, fragment.position, c[2].z;
MOV R2.y, R2.z;

TEX R3, R2, texture[2], 2D;
TEX R0, R2, texture[1], 2D;
TEX R1, R2, texture[0], 2D;

# multiply by screen size
MUL R2.x, R2.x, c[0].x;
MUL R2.y, R2.y, c[0].y;
# backup original
MOV R2.z, R2.x;
MOV R2.w, R2.y;

# multiply by inverse of font size
MUL R2.x, R2.x, c[1].z;
MUL R2.y, R2.y, c[1].w;
FLR R2.x, R2.x;
FLR R2.y, R2.y;
MUL R2.x, R2.x, c[1].x;
MUL R2.y, R2.y, c[1].y;
# now we have a bit of a staircase, take the original minus staircase
ADD R2.x, R2.z, -R2.x;
ADD R2.y, R2.w, -R2.y;
# modulo is complete

# normalize per unit (inv font size)
MUL R2.x, R2.x, c[1].z;
MUL R2.y, R2.y, c[1].w;
# divide by 16 for proper texture offset
MUL R2.x, R2.x, .0625;
MUL R2.y, R2.y, .0625;
# add to given texture offset
ADD R2.x, R2.x, R1.x;
ADD R2.y, R2.y, R1.y;

# ... and sample!
TEX R2, R2, texture[3], 2D;

#R2 is tile color
#R3 is background color
#R0 is color color
MUL R0, R0, R2;
#R0 is result color
SUB R3, R3, R0;
#R3 is bgcolor - rescolor

# lerp R3 (multiply by 1 - r)
MAD R3, R3, -R0.a, R3;

#R3 is (bgcolor - rescolor) * rescolor.a - (bgcolor - rescolor)
ADD result.color, R3, R0;
END
# 29 instructions, 3 R-regs
