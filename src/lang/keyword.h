#ifndef KEYWORD_H
#define KEYWORD_H

#include <string>
#include "tst.hpp"

namespace HXSL
{
	enum HXSLKeyword : int
	{
		HXSLKeyword_Unknown,
		HXSLKeyword_AppendStructuredBuffer,
		HXSLKeyword_Asm,
		HXSLKeyword_AsmFragment,
		HXSLKeyword_BlendState,
		HXSLKeyword_Bool,
		HXSLKeyword_Break,
		HXSLKeyword_Buffer,
		HXSLKeyword_ByteAddressBuffer,
		HXSLKeyword_Case,
		HXSLKeyword_Cbuffer,
		HXSLKeyword_Centroid,
		HXSLKeyword_Class,
		HXSLKeyword_ColumnMajor,
		HXSLKeyword_Compile,
		HXSLKeyword_CompileFragment,
		HXSLKeyword_CompileShader,
		HXSLKeyword_Const,
		HXSLKeyword_Continue,
		HXSLKeyword_ComputeShader,
		HXSLKeyword_ConsumeStructuredBuffer,
		HXSLKeyword_Default,
		HXSLKeyword_DepthStencilState,
		HXSLKeyword_DepthStencilView,
		HXSLKeyword_Discard,
		HXSLKeyword_Do,
		HXSLKeyword_Double,
		HXSLKeyword_DomainShader,
		HXSLKeyword_Dword,
		HXSLKeyword_Else,
		HXSLKeyword_Export,
		HXSLKeyword_Extern,
		HXSLKeyword_False,
		HXSLKeyword_Float,
		HXSLKeyword_For,
		HXSLKeyword_Fxgroup,
		HXSLKeyword_GeometryShader,
		HXSLKeyword_Groupshared,
		HXSLKeyword_Half,
		HXSLKeyword_Hullshader,
		HXSLKeyword_If,
		HXSLKeyword_In,
		HXSLKeyword_Inline,
		HXSLKeyword_Inout,
		HXSLKeyword_InputPatch,
		HXSLKeyword_Int,
		HXSLKeyword_Interface,
		HXSLKeyword_Line,
		HXSLKeyword_Lineadj,
		HXSLKeyword_Linear,
		HXSLKeyword_LineStream,
		HXSLKeyword_Matrix,
		HXSLKeyword_Min16float,
		HXSLKeyword_Min10float,
		HXSLKeyword_Min16int,
		HXSLKeyword_Min12int,
		HXSLKeyword_Min16uint,
		HXSLKeyword_Namespace,
		HXSLKeyword_Nointerpolation,
		HXSLKeyword_Noperspective,
		HXSLKeyword_Null,
		HXSLKeyword_Out,
		HXSLKeyword_OutputPatch,
		HXSLKeyword_Packoffset,
		HXSLKeyword_Pass,
		HXSLKeyword_Pixelfragment,
		HXSLKeyword_PixelShader,
		HXSLKeyword_Point,
		HXSLKeyword_PointStream,
		HXSLKeyword_Precise,
		HXSLKeyword_RasterizerState,
		HXSLKeyword_RenderTargetView,
		HXSLKeyword_Return,
		HXSLKeyword_Register,
		HXSLKeyword_RowMajor,
		HXSLKeyword_RWBuffer,
		HXSLKeyword_RWByteAddressBuffer,
		HXSLKeyword_RWStructuredBuffer,
		HXSLKeyword_RWTexture1D,
		HXSLKeyword_RWTexture1DArray,
		HXSLKeyword_RWTexture2D,
		HXSLKeyword_RWTexture2DArray,
		HXSLKeyword_RWTexture3D,
		HXSLKeyword_Sample,
		HXSLKeyword_Sampler,
		HXSLKeyword_SamplerState,
		HXSLKeyword_SamplerComparisonState,
		HXSLKeyword_Shared,
		HXSLKeyword_Snorm,
		HXSLKeyword_Stateblock,
		HXSLKeyword_StateblockState,
		HXSLKeyword_Static,
		HXSLKeyword_String,
		HXSLKeyword_Struct,
		HXSLKeyword_Switch,
		HXSLKeyword_StructuredBuffer,
		HXSLKeyword_Tbuffer,
		HXSLKeyword_Technique,
		HXSLKeyword_Technique10,
		HXSLKeyword_Technique11,
		HXSLKeyword_Texture,
		HXSLKeyword_Texture1D,
		HXSLKeyword_Texture1DArray,
		HXSLKeyword_Texture2D,
		HXSLKeyword_Texture2DArray,
		HXSLKeyword_Texture2DMS,
		HXSLKeyword_Texture2DMSArray,
		HXSLKeyword_Texture3D,
		HXSLKeyword_TextureCube,
		HXSLKeyword_TextureCubeArray,
		HXSLKeyword_True,
		HXSLKeyword_Typedef,
		HXSLKeyword_Triangle,
		HXSLKeyword_Triangleadj,
		HXSLKeyword_TriangleStream,
		HXSLKeyword_Uint,
		HXSLKeyword_Uniform,
		HXSLKeyword_Unorm,
		HXSLKeyword_Unsigned,
		HXSLKeyword_Vector,
		HXSLKeyword_Vertexfragment,
		HXSLKeyword_VertexShader,
		HXSLKeyword_Void,
		HXSLKeyword_Volatile,
		HXSLKeyword_While,
		HXSLKeyword_Using,
		HXSLKeyword_Private,
		HXSLKeyword_Internal,
		HXSLKeyword_Public,
	};

	static std::string ToString(HXSLKeyword keyword)
	{
		switch (keyword)
		{
		case HXSLKeyword_Unknown: return "Unknown";
		case HXSLKeyword_AppendStructuredBuffer: return "AppendStructuredBuffer";
		case HXSLKeyword_Asm: return "asm";
		case HXSLKeyword_AsmFragment: return "asm_fragment";
		case HXSLKeyword_BlendState: return "BlendState";
		case HXSLKeyword_Bool: return "bool";
		case HXSLKeyword_Break: return "break";
		case HXSLKeyword_Buffer: return "Buffer";
		case HXSLKeyword_ByteAddressBuffer: return "ByteAddressBuffer";
		case HXSLKeyword_Case: return "case";
		case HXSLKeyword_Cbuffer: return "cbuffer";
		case HXSLKeyword_Centroid: return "centroid";
		case HXSLKeyword_Class: return "class";
		case HXSLKeyword_ColumnMajor: return "column_major";
		case HXSLKeyword_Compile: return "compile";
		case HXSLKeyword_CompileFragment: return "compile_fragment";
		case HXSLKeyword_CompileShader: return "CompileShader";
		case HXSLKeyword_Const: return "const";
		case HXSLKeyword_Continue: return "continue";
		case HXSLKeyword_ComputeShader: return "ComputeShader";
		case HXSLKeyword_ConsumeStructuredBuffer: return "ConsumeStructuredBuffer";
		case HXSLKeyword_Default: return "default";
		case HXSLKeyword_DepthStencilState: return "DepthStencilState";
		case HXSLKeyword_DepthStencilView: return "DepthStencilView";
		case HXSLKeyword_Discard: return "discard";
		case HXSLKeyword_Do: return "do";
		case HXSLKeyword_Double: return "double";
		case HXSLKeyword_DomainShader: return "DomainShader";
		case HXSLKeyword_Dword: return "dword";
		case HXSLKeyword_Else: return "else";
		case HXSLKeyword_Export: return "export";
		case HXSLKeyword_Extern: return "extern";
		case HXSLKeyword_False: return "false";
		case HXSLKeyword_Float: return "float";
		case HXSLKeyword_For: return "for";
		case HXSLKeyword_Fxgroup: return "fxgroup";
		case HXSLKeyword_GeometryShader: return "GeometryShader";
		case HXSLKeyword_Groupshared: return "groupshared";
		case HXSLKeyword_Half: return "half";
		case HXSLKeyword_Hullshader: return "Hullshader";
		case HXSLKeyword_If: return "if";
		case HXSLKeyword_In: return "in";
		case HXSLKeyword_Inline: return "inline";
		case HXSLKeyword_Inout: return "inout";
		case HXSLKeyword_InputPatch: return "InputPatch";
		case HXSLKeyword_Int: return "int";
		case HXSLKeyword_Interface: return "interface";
		case HXSLKeyword_Line: return "line";
		case HXSLKeyword_Lineadj: return "lineadj";
		case HXSLKeyword_Linear: return "linear";
		case HXSLKeyword_LineStream: return "LineStream";
		case HXSLKeyword_Matrix: return "matrix";
		case HXSLKeyword_Min16float: return "min16float";
		case HXSLKeyword_Min10float: return "min10float";
		case HXSLKeyword_Min16int: return "min16int";
		case HXSLKeyword_Min12int: return "min12int";
		case HXSLKeyword_Min16uint: return "min16uint";
		case HXSLKeyword_Namespace: return "namespace";
		case HXSLKeyword_Nointerpolation: return "nointerpolation";
		case HXSLKeyword_Noperspective: return "noperspective";
		case HXSLKeyword_Null: return "NULL";
		case HXSLKeyword_Out: return "out";
		case HXSLKeyword_OutputPatch: return "OutputPatch";
		case HXSLKeyword_Packoffset: return "packoffset";
		case HXSLKeyword_Pass: return "pass";
		case HXSLKeyword_Pixelfragment: return "pixelfragment";
		case HXSLKeyword_PixelShader: return "PixelShader";
		case HXSLKeyword_Point: return "point";
		case HXSLKeyword_PointStream: return "PointStream";
		case HXSLKeyword_Precise: return "precise";
		case HXSLKeyword_RasterizerState: return "RasterizerState";
		case HXSLKeyword_RenderTargetView: return "RenderTargetView";
		case HXSLKeyword_Return: return "return";
		case HXSLKeyword_Register: return "register";
		case HXSLKeyword_RowMajor: return "row_major";
		case HXSLKeyword_RWBuffer: return "RWBuffer";
		case HXSLKeyword_RWByteAddressBuffer: return "RWByteAddressBuffer";
		case HXSLKeyword_RWStructuredBuffer: return "RWStructuredBuffer";
		case HXSLKeyword_RWTexture1D: return "RWTexture1D";
		case HXSLKeyword_RWTexture1DArray: return "RWTexture1DArray";
		case HXSLKeyword_RWTexture2D: return "RWTexture2D";
		case HXSLKeyword_RWTexture2DArray: return "RWTexture2DArray";
		case HXSLKeyword_RWTexture3D: return "RWTexture3D";
		case HXSLKeyword_Sample: return "sample";
		case HXSLKeyword_Sampler: return "sampler";
		case HXSLKeyword_SamplerState: return "SamplerState";
		case HXSLKeyword_SamplerComparisonState: return "SamplerComparisonState";
		case HXSLKeyword_Shared: return "shared";
		case HXSLKeyword_Snorm: return "snorm";
		case HXSLKeyword_Stateblock: return "stateblock";
		case HXSLKeyword_StateblockState: return "stateblock_state";
		case HXSLKeyword_Static: return "static";
		case HXSLKeyword_String: return "string";
		case HXSLKeyword_Struct: return "struct";
		case HXSLKeyword_Switch: return "switch";
		case HXSLKeyword_StructuredBuffer: return "StructuredBuffer";
		case HXSLKeyword_Tbuffer: return "tbuffer";
		case HXSLKeyword_Technique: return "technique";
		case HXSLKeyword_Technique10: return "technique10";
		case HXSLKeyword_Technique11: return "technique11";
		case HXSLKeyword_Texture: return "texture";
		case HXSLKeyword_Texture1D: return "Texture1D";
		case HXSLKeyword_Texture1DArray: return "Texture1DArray";
		case HXSLKeyword_Texture2D: return "Texture2D";
		case HXSLKeyword_Texture2DArray: return "Texture2DArray";
		case HXSLKeyword_Texture2DMS: return "Texture2DMS";
		case HXSLKeyword_Texture2DMSArray: return "Texture2DMSArray";
		case HXSLKeyword_Texture3D: return "Texture3D";
		case HXSLKeyword_TextureCube: return "TextureCube";
		case HXSLKeyword_TextureCubeArray: return "TextureCubeArray";
		case HXSLKeyword_True: return "true";
		case HXSLKeyword_Typedef: return "typedef";
		case HXSLKeyword_Triangle: return "triangle";
		case HXSLKeyword_Triangleadj: return "triangleadj";
		case HXSLKeyword_TriangleStream: return "TriangleStream";
		case HXSLKeyword_Uint: return "uint";
		case HXSLKeyword_Uniform: return "uniform";
		case HXSLKeyword_Unorm: return "unorm";
		case HXSLKeyword_Unsigned: return "unsigned";
		case HXSLKeyword_Vector: return "vector";
		case HXSLKeyword_Vertexfragment: return "vertexfragment";
		case HXSLKeyword_VertexShader: return "VertexShader";
		case HXSLKeyword_Void: return "void";
		case HXSLKeyword_Volatile: return "volatile";
		case HXSLKeyword_While: return "while";
		case HXSLKeyword_Using: return "using";
		case HXSLKeyword_Private: return "private";
		case HXSLKeyword_Internal: return "internal";
		case HXSLKeyword_Public: return "public";
		default: return "Unknown";
		}
	}

	static void BuildKeywordTST(TernarySearchTreeDictionary<int>* tst)
	{
		tst->Insert("AppendStructuredBuffer", HXSLKeyword_AppendStructuredBuffer);
		tst->Insert("asm", HXSLKeyword_Asm);
		tst->Insert("asm_fragment", HXSLKeyword_AsmFragment);
		tst->Insert("BlendState", HXSLKeyword_BlendState);
		tst->Insert("bool", HXSLKeyword_Bool);
		tst->Insert("break", HXSLKeyword_Break);
		tst->Insert("Buffer", HXSLKeyword_Buffer);
		tst->Insert("ByteAddressBuffer", HXSLKeyword_ByteAddressBuffer);
		tst->Insert("case", HXSLKeyword_Case);
		tst->Insert("cbuffer", HXSLKeyword_Cbuffer);
		tst->Insert("centroid", HXSLKeyword_Centroid);
		tst->Insert("class", HXSLKeyword_Class);
		tst->Insert("column_major", HXSLKeyword_ColumnMajor);
		tst->Insert("compile", HXSLKeyword_Compile);
		tst->Insert("compile_fragment", HXSLKeyword_CompileFragment);
		tst->Insert("CompileShader", HXSLKeyword_CompileShader);
		tst->Insert("const", HXSLKeyword_Const);
		tst->Insert("continue", HXSLKeyword_Continue);
		tst->Insert("ComputeShader", HXSLKeyword_ComputeShader);
		tst->Insert("ConsumeStructuredBuffer", HXSLKeyword_ConsumeStructuredBuffer);
		tst->Insert("default", HXSLKeyword_Default);
		tst->Insert("DepthStencilState", HXSLKeyword_DepthStencilState);
		tst->Insert("DepthStencilView", HXSLKeyword_DepthStencilView);
		tst->Insert("discard", HXSLKeyword_Discard);
		tst->Insert("do", HXSLKeyword_Do);
		tst->Insert("double", HXSLKeyword_Double);
		tst->Insert("DomainShader", HXSLKeyword_DomainShader);
		tst->Insert("dword", HXSLKeyword_Dword);
		tst->Insert("else", HXSLKeyword_Else);
		tst->Insert("export", HXSLKeyword_Export);
		tst->Insert("extern", HXSLKeyword_Extern);
		tst->Insert("false", HXSLKeyword_False);
		tst->Insert("float", HXSLKeyword_Float);
		tst->Insert("for", HXSLKeyword_For);
		tst->Insert("fxgroup", HXSLKeyword_Fxgroup);
		tst->Insert("GeometryShader", HXSLKeyword_GeometryShader);
		tst->Insert("groupshared", HXSLKeyword_Groupshared);
		tst->Insert("half", HXSLKeyword_Half);
		tst->Insert("Hullshader", HXSLKeyword_Hullshader);
		tst->Insert("if", HXSLKeyword_If);
		tst->Insert("in", HXSLKeyword_In);
		tst->Insert("inline", HXSLKeyword_Inline);
		tst->Insert("inout", HXSLKeyword_Inout);
		tst->Insert("InputPatch", HXSLKeyword_InputPatch);
		tst->Insert("int", HXSLKeyword_Int);
		tst->Insert("interface", HXSLKeyword_Interface);
		tst->Insert("line", HXSLKeyword_Line);
		tst->Insert("lineadj", HXSLKeyword_Lineadj);
		tst->Insert("linear", HXSLKeyword_Linear);
		tst->Insert("LineStream", HXSLKeyword_LineStream);
		tst->Insert("matrix", HXSLKeyword_Matrix);
		tst->Insert("min16float", HXSLKeyword_Min16float);
		tst->Insert("min10float", HXSLKeyword_Min10float);
		tst->Insert("min16int", HXSLKeyword_Min16int);
		tst->Insert("min12int", HXSLKeyword_Min12int);
		tst->Insert("min16uint", HXSLKeyword_Min16uint);
		tst->Insert("namespace", HXSLKeyword_Namespace);
		tst->Insert("nointerpolation", HXSLKeyword_Nointerpolation);
		tst->Insert("noperspective", HXSLKeyword_Noperspective);
		tst->Insert("NULL", HXSLKeyword_Null);
		tst->Insert("out", HXSLKeyword_Out);
		tst->Insert("OutputPatch", HXSLKeyword_OutputPatch);
		tst->Insert("packoffset", HXSLKeyword_Packoffset);
		tst->Insert("pass", HXSLKeyword_Pass);
		tst->Insert("pixelfragment", HXSLKeyword_Pixelfragment);
		tst->Insert("PixelShader", HXSLKeyword_PixelShader);
		tst->Insert("point", HXSLKeyword_Point);
		tst->Insert("PointStream", HXSLKeyword_PointStream);
		tst->Insert("precise", HXSLKeyword_Precise);
		tst->Insert("RasterizerState", HXSLKeyword_RasterizerState);
		tst->Insert("RenderTargetView", HXSLKeyword_RenderTargetView);
		tst->Insert("return", HXSLKeyword_Return);
		tst->Insert("register", HXSLKeyword_Register);
		tst->Insert("row_major", HXSLKeyword_RowMajor);
		tst->Insert("RWBuffer", HXSLKeyword_RWBuffer);
		tst->Insert("RWByteAddressBuffer", HXSLKeyword_RWByteAddressBuffer);
		tst->Insert("RWStructuredBuffer", HXSLKeyword_RWStructuredBuffer);
		tst->Insert("RWTexture1D", HXSLKeyword_RWTexture1D);
		tst->Insert("RWTexture1DArray", HXSLKeyword_RWTexture1DArray);
		tst->Insert("RWTexture2D", HXSLKeyword_RWTexture2D);
		tst->Insert("RWTexture2DArray", HXSLKeyword_RWTexture2DArray);
		tst->Insert("RWTexture3D", HXSLKeyword_RWTexture3D);
		tst->Insert("sample", HXSLKeyword_Sample);
		tst->Insert("sampler", HXSLKeyword_Sampler);
		tst->Insert("SamplerState", HXSLKeyword_SamplerState);
		tst->Insert("SamplerComparisonState", HXSLKeyword_SamplerComparisonState);
		tst->Insert("shared", HXSLKeyword_Shared);
		tst->Insert("snorm", HXSLKeyword_Snorm);
		tst->Insert("stateblock", HXSLKeyword_Stateblock);
		tst->Insert("stateblock_state", HXSLKeyword_StateblockState);
		tst->Insert("static", HXSLKeyword_Static);
		tst->Insert("string", HXSLKeyword_String);
		tst->Insert("struct", HXSLKeyword_Struct);
		tst->Insert("switch", HXSLKeyword_Switch);
		tst->Insert("StructuredBuffer", HXSLKeyword_StructuredBuffer);
		tst->Insert("tbuffer", HXSLKeyword_Tbuffer);
		tst->Insert("technique", HXSLKeyword_Technique);
		tst->Insert("technique10", HXSLKeyword_Technique10);
		tst->Insert("technique11", HXSLKeyword_Technique11);
		tst->Insert("texture", HXSLKeyword_Texture);
		tst->Insert("Texture1D", HXSLKeyword_Texture1D);
		tst->Insert("Texture1DArray", HXSLKeyword_Texture1DArray);
		tst->Insert("Texture2D", HXSLKeyword_Texture2D);
		tst->Insert("Texture2DArray", HXSLKeyword_Texture2DArray);
		tst->Insert("Texture2DMS", HXSLKeyword_Texture2DMS);
		tst->Insert("Texture2DMSArray", HXSLKeyword_Texture2DMSArray);
		tst->Insert("Texture3D", HXSLKeyword_Texture3D);
		tst->Insert("TextureCube", HXSLKeyword_TextureCube);
		tst->Insert("TextureCubeArray", HXSLKeyword_TextureCubeArray);
		tst->Insert("true", HXSLKeyword_True);
		tst->Insert("typedef", HXSLKeyword_Typedef);
		tst->Insert("triangle", HXSLKeyword_Triangle);
		tst->Insert("triangleadj", HXSLKeyword_Triangleadj);
		tst->Insert("TriangleStream", HXSLKeyword_TriangleStream);
		tst->Insert("uint", HXSLKeyword_Uint);
		tst->Insert("uniform", HXSLKeyword_Uniform);
		tst->Insert("unorm", HXSLKeyword_Unorm);
		tst->Insert("unsigned", HXSLKeyword_Unsigned);
		tst->Insert("vector", HXSLKeyword_Vector);
		tst->Insert("vertexfragment", HXSLKeyword_Vertexfragment);
		tst->Insert("VertexShader", HXSLKeyword_VertexShader);
		tst->Insert("void", HXSLKeyword_Void);
		tst->Insert("volatile", HXSLKeyword_Volatile);
		tst->Insert("while", HXSLKeyword_While);
		tst->Insert("using", HXSLKeyword_Using);
		tst->Insert("private", HXSLKeyword_Private);
		tst->Insert("internal", HXSLKeyword_Internal);
		tst->Insert("public", HXSLKeyword_Public);
	}
}
#endif