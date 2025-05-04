#ifndef KEYWORD_H
#define KEYWORD_H

#include <string>
#include "utils/tst.hpp"

namespace HXSL
{
	enum Keyword : int
	{
		Keyword_Unknown,
		Keyword_AppendStructuredBuffer,
		Keyword_Asm,
		Keyword_AsmFragment,
		Keyword_BlendState,
		Keyword_Bool,
		Keyword_Break,
		Keyword_Buffer,
		Keyword_ByteAddressBuffer,
		Keyword_Case,
		Keyword_Cbuffer,
		Keyword_Centroid,
		Keyword_Class,
		Keyword_ColumnMajor,
		Keyword_Compile,
		Keyword_CompileFragment,
		Keyword_CompileShader,
		Keyword_Const,
		Keyword_Continue,
		Keyword_ComputeShader,
		Keyword_ConsumeStructuredBuffer,
		Keyword_Default,
		Keyword_DepthStencilState,
		Keyword_DepthStencilView,
		Keyword_Discard,
		Keyword_Do,
		Keyword_Double,
		Keyword_DomainShader,
		Keyword_Dword,
		Keyword_Else,
		Keyword_Export,
		Keyword_Extern,
		Keyword_False,
		Keyword_Float,
		Keyword_For,
		Keyword_Fxgroup,
		Keyword_GeometryShader,
		Keyword_Groupshared,
		Keyword_Half,
		Keyword_Hullshader,
		Keyword_If,
		Keyword_In,
		Keyword_Inline,
		Keyword_Inout,
		Keyword_InputPatch,
		Keyword_Int,
		Keyword_Interface,
		Keyword_Line,
		Keyword_Lineadj,
		Keyword_Linear,
		Keyword_LineStream,
		Keyword_Matrix,
		Keyword_Min16float,
		Keyword_Min10float,
		Keyword_Min16int,
		Keyword_Min12int,
		Keyword_Min16uint,
		Keyword_Namespace,
		Keyword_NoInterpolation,
		Keyword_Noperspective,
		Keyword_Null,
		Keyword_Out,
		Keyword_OutputPatch,
		Keyword_Packoffset,
		Keyword_Pass,
		Keyword_Pixelfragment,
		Keyword_PixelShader,
		Keyword_Point,
		Keyword_PointStream,
		Keyword_Precise,
		Keyword_RasterizerState,
		Keyword_RenderTargetView,
		Keyword_Return,
		Keyword_Register,
		Keyword_RowMajor,
		Keyword_RWBuffer,
		Keyword_RWByteAddressBuffer,
		Keyword_RWStructuredBuffer,
		Keyword_RWTexture1D,
		Keyword_RWTexture1DArray,
		Keyword_RWTexture2D,
		Keyword_RWTexture2DArray,
		Keyword_RWTexture3D,
		Keyword_Sample,
		Keyword_Sampler,
		Keyword_SamplerState,
		Keyword_SamplerComparisonState,
		Keyword_Shared,
		Keyword_Snorm,
		Keyword_Stateblock,
		Keyword_StateblockState,
		Keyword_Static,
		Keyword_String,
		Keyword_Struct,
		Keyword_Switch,
		Keyword_StructuredBuffer,
		Keyword_Tbuffer,
		Keyword_Technique,
		Keyword_Technique10,
		Keyword_Technique11,
		Keyword_Texture,
		Keyword_Texture1D,
		Keyword_Texture1DArray,
		Keyword_Texture2D,
		Keyword_Texture2DArray,
		Keyword_Texture2DMS,
		Keyword_Texture2DMSArray,
		Keyword_Texture3D,
		Keyword_TextureCube,
		Keyword_TextureCubeArray,
		Keyword_True,
		Keyword_Typedef,
		Keyword_Triangle,
		Keyword_Triangleadj,
		Keyword_TriangleStream,
		Keyword_Uint,
		Keyword_Uniform,
		Keyword_Unorm,
		Keyword_Unsigned,
		Keyword_Vector,
		Keyword_Vertexfragment,
		Keyword_VertexShader,
		Keyword_Void,
		Keyword_Volatile,
		Keyword_While,
		Keyword_Operator,
		Keyword_Implicit,
		Keyword_Explicit,
		Keyword_Using,
		Keyword_Private,
		Keyword_Protected,
		Keyword_Internal,
		Keyword_Public,
		Keyword_PrepDefine,
		Keyword_PrepIf,
		Keyword_PrepElif,
		Keyword_PrepElse,
		Keyword_PrepEndif,
		Keyword_PrepInclude,
		Keyword_PrepError,
		Keyword_PrepPragma
	};

	static std::string ToString(Keyword keyword)
	{
		switch (keyword)
		{
		case Keyword_Unknown: return "Unknown";
		case Keyword_AppendStructuredBuffer: return "AppendStructuredBuffer";
		case Keyword_Asm: return "asm";
		case Keyword_AsmFragment: return "asm_fragment";
		case Keyword_BlendState: return "BlendState";
		case Keyword_Bool: return "bool";
		case Keyword_Break: return "break";
		case Keyword_Buffer: return "Buffer";
		case Keyword_ByteAddressBuffer: return "ByteAddressBuffer";
		case Keyword_Case: return "case";
		case Keyword_Cbuffer: return "cbuffer";
		case Keyword_Centroid: return "centroid";
		case Keyword_Class: return "class";
		case Keyword_ColumnMajor: return "column_major";
		case Keyword_Compile: return "compile";
		case Keyword_CompileFragment: return "compile_fragment";
		case Keyword_CompileShader: return "CompileShader";
		case Keyword_Const: return "const";
		case Keyword_Continue: return "continue";
		case Keyword_ComputeShader: return "ComputeShader";
		case Keyword_ConsumeStructuredBuffer: return "ConsumeStructuredBuffer";
		case Keyword_Default: return "default";
		case Keyword_DepthStencilState: return "DepthStencilState";
		case Keyword_DepthStencilView: return "DepthStencilView";
		case Keyword_Discard: return "discard";
		case Keyword_Do: return "do";
		case Keyword_Double: return "double";
		case Keyword_DomainShader: return "DomainShader";
		case Keyword_Dword: return "dword";
		case Keyword_Else: return "else";
		case Keyword_Export: return "export";
		case Keyword_Extern: return "extern";
		case Keyword_False: return "false";
		case Keyword_Float: return "float";
		case Keyword_For: return "for";
		case Keyword_Fxgroup: return "fxgroup";
		case Keyword_GeometryShader: return "GeometryShader";
		case Keyword_Groupshared: return "groupshared";
		case Keyword_Half: return "half";
		case Keyword_Hullshader: return "Hullshader";
		case Keyword_If: return "if";
		case Keyword_In: return "in";
		case Keyword_Inline: return "inline";
		case Keyword_Inout: return "inout";
		case Keyword_InputPatch: return "InputPatch";
		case Keyword_Int: return "int";
		case Keyword_Interface: return "interface";
		case Keyword_Line: return "line";
		case Keyword_Lineadj: return "lineadj";
		case Keyword_Linear: return "linear";
		case Keyword_LineStream: return "LineStream";
		case Keyword_Matrix: return "matrix";
		case Keyword_Min16float: return "min16float";
		case Keyword_Min10float: return "min10float";
		case Keyword_Min16int: return "min16int";
		case Keyword_Min12int: return "min12int";
		case Keyword_Min16uint: return "min16uint";
		case Keyword_Namespace: return "namespace";
		case Keyword_NoInterpolation: return "nointerpolation";
		case Keyword_Noperspective: return "noperspective";
		case Keyword_Null: return "NULL";
		case Keyword_Out: return "out";
		case Keyword_OutputPatch: return "OutputPatch";
		case Keyword_Packoffset: return "packoffset";
		case Keyword_Pass: return "pass";
		case Keyword_Pixelfragment: return "pixelfragment";
		case Keyword_PixelShader: return "PixelShader";
		case Keyword_Point: return "point";
		case Keyword_PointStream: return "PointStream";
		case Keyword_Precise: return "precise";
		case Keyword_RasterizerState: return "RasterizerState";
		case Keyword_RenderTargetView: return "RenderTargetView";
		case Keyword_Return: return "return";
		case Keyword_Register: return "register";
		case Keyword_RowMajor: return "row_major";
		case Keyword_RWBuffer: return "RWBuffer";
		case Keyword_RWByteAddressBuffer: return "RWByteAddressBuffer";
		case Keyword_RWStructuredBuffer: return "RWStructuredBuffer";
		case Keyword_RWTexture1D: return "RWTexture1D";
		case Keyword_RWTexture1DArray: return "RWTexture1DArray";
		case Keyword_RWTexture2D: return "RWTexture2D";
		case Keyword_RWTexture2DArray: return "RWTexture2DArray";
		case Keyword_RWTexture3D: return "RWTexture3D";
		case Keyword_Sample: return "sample";
		case Keyword_Sampler: return "sampler";
		case Keyword_SamplerState: return "SamplerState";
		case Keyword_SamplerComparisonState: return "SamplerComparisonState";
		case Keyword_Shared: return "shared";
		case Keyword_Snorm: return "snorm";
		case Keyword_Stateblock: return "stateblock";
		case Keyword_StateblockState: return "stateblock_state";
		case Keyword_Static: return "static";
		case Keyword_String: return "string";
		case Keyword_Struct: return "struct";
		case Keyword_Switch: return "switch";
		case Keyword_StructuredBuffer: return "StructuredBuffer";
		case Keyword_Tbuffer: return "tbuffer";
		case Keyword_Technique: return "technique";
		case Keyword_Technique10: return "technique10";
		case Keyword_Technique11: return "technique11";
		case Keyword_Texture: return "texture";
		case Keyword_Texture1D: return "Texture1D";
		case Keyword_Texture1DArray: return "Texture1DArray";
		case Keyword_Texture2D: return "Texture2D";
		case Keyword_Texture2DArray: return "Texture2DArray";
		case Keyword_Texture2DMS: return "Texture2DMS";
		case Keyword_Texture2DMSArray: return "Texture2DMSArray";
		case Keyword_Texture3D: return "Texture3D";
		case Keyword_TextureCube: return "TextureCube";
		case Keyword_TextureCubeArray: return "TextureCubeArray";
		case Keyword_True: return "true";
		case Keyword_Typedef: return "typedef";
		case Keyword_Triangle: return "triangle";
		case Keyword_Triangleadj: return "triangleadj";
		case Keyword_TriangleStream: return "TriangleStream";
		case Keyword_Uint: return "uint";
		case Keyword_Uniform: return "uniform";
		case Keyword_Unorm: return "unorm";
		case Keyword_Unsigned: return "unsigned";
		case Keyword_Vector: return "vector";
		case Keyword_Vertexfragment: return "vertexfragment";
		case Keyword_VertexShader: return "VertexShader";
		case Keyword_Void: return "void";
		case Keyword_Volatile: return "volatile";
		case Keyword_While: return "while";
		case Keyword_Operator: return "operator";
		case Keyword_Implicit: return "implicit";
		case Keyword_Explicit: return "explicit";
		case Keyword_Using: return "using";
		case Keyword_Private: return "private";
		case Keyword_Protected: return "protected";
		case Keyword_Internal: return "internal";
		case Keyword_Public: return "public";
		case Keyword_PrepDefine: return "#define";
		case Keyword_PrepIf: return "#if";
		case Keyword_PrepElif: return "#elif";
		case Keyword_PrepElse: return "#else";
		case Keyword_PrepEndif: return "#endif";
		case Keyword_PrepInclude: return "#include";
		case Keyword_PrepError: return "#error";
		case Keyword_PrepPragma: return "#pragma";
		default: return "Unknown";
		}
	}

	static void BuildKeywordTST(TernarySearchTreeDictionary<int>* tst)
	{
		tst->Insert("AppendStructuredBuffer", Keyword_AppendStructuredBuffer);
		tst->Insert("asm", Keyword_Asm);
		tst->Insert("asm_fragment", Keyword_AsmFragment);
		tst->Insert("BlendState", Keyword_BlendState);
		tst->Insert("bool", Keyword_Bool);
		tst->Insert("break", Keyword_Break);
		tst->Insert("Buffer", Keyword_Buffer);
		tst->Insert("ByteAddressBuffer", Keyword_ByteAddressBuffer);
		tst->Insert("case", Keyword_Case);
		tst->Insert("cbuffer", Keyword_Cbuffer);
		tst->Insert("centroid", Keyword_Centroid);
		tst->Insert("class", Keyword_Class);
		tst->Insert("column_major", Keyword_ColumnMajor);
		tst->Insert("compile", Keyword_Compile);
		tst->Insert("compile_fragment", Keyword_CompileFragment);
		tst->Insert("CompileShader", Keyword_CompileShader);
		tst->Insert("const", Keyword_Const);
		tst->Insert("continue", Keyword_Continue);
		tst->Insert("ComputeShader", Keyword_ComputeShader);
		tst->Insert("ConsumeStructuredBuffer", Keyword_ConsumeStructuredBuffer);
		tst->Insert("default", Keyword_Default);
		tst->Insert("DepthStencilState", Keyword_DepthStencilState);
		tst->Insert("DepthStencilView", Keyword_DepthStencilView);
		tst->Insert("discard", Keyword_Discard);
		tst->Insert("do", Keyword_Do);
		tst->Insert("double", Keyword_Double);
		tst->Insert("DomainShader", Keyword_DomainShader);
		tst->Insert("dword", Keyword_Dword);
		tst->Insert("else", Keyword_Else);
		tst->Insert("export", Keyword_Export);
		tst->Insert("extern", Keyword_Extern);
		tst->Insert("false", Keyword_False);
		tst->Insert("float", Keyword_Float);
		tst->Insert("for", Keyword_For);
		tst->Insert("fxgroup", Keyword_Fxgroup);
		tst->Insert("GeometryShader", Keyword_GeometryShader);
		tst->Insert("groupshared", Keyword_Groupshared);
		tst->Insert("half", Keyword_Half);
		tst->Insert("Hullshader", Keyword_Hullshader);
		tst->Insert("if", Keyword_If);
		tst->Insert("in", Keyword_In);
		tst->Insert("inline", Keyword_Inline);
		tst->Insert("inout", Keyword_Inout);
		tst->Insert("InputPatch", Keyword_InputPatch);
		tst->Insert("int", Keyword_Int);
		tst->Insert("interface", Keyword_Interface);
		tst->Insert("line", Keyword_Line);
		tst->Insert("lineadj", Keyword_Lineadj);
		tst->Insert("linear", Keyword_Linear);
		tst->Insert("LineStream", Keyword_LineStream);
		tst->Insert("matrix", Keyword_Matrix);
		tst->Insert("min16float", Keyword_Min16float);
		tst->Insert("min10float", Keyword_Min10float);
		tst->Insert("min16int", Keyword_Min16int);
		tst->Insert("min12int", Keyword_Min12int);
		tst->Insert("min16uint", Keyword_Min16uint);
		tst->Insert("namespace", Keyword_Namespace);
		tst->Insert("nointerpolation", Keyword_NoInterpolation);
		tst->Insert("noperspective", Keyword_Noperspective);
		tst->Insert("NULL", Keyword_Null);
		tst->Insert("out", Keyword_Out);
		tst->Insert("OutputPatch", Keyword_OutputPatch);
		tst->Insert("packoffset", Keyword_Packoffset);
		tst->Insert("pass", Keyword_Pass);
		tst->Insert("pixelfragment", Keyword_Pixelfragment);
		tst->Insert("PixelShader", Keyword_PixelShader);
		tst->Insert("point", Keyword_Point);
		tst->Insert("PointStream", Keyword_PointStream);
		tst->Insert("precise", Keyword_Precise);
		tst->Insert("RasterizerState", Keyword_RasterizerState);
		tst->Insert("RenderTargetView", Keyword_RenderTargetView);
		tst->Insert("return", Keyword_Return);
		tst->Insert("register", Keyword_Register);
		tst->Insert("row_major", Keyword_RowMajor);
		tst->Insert("RWBuffer", Keyword_RWBuffer);
		tst->Insert("RWByteAddressBuffer", Keyword_RWByteAddressBuffer);
		tst->Insert("RWStructuredBuffer", Keyword_RWStructuredBuffer);
		tst->Insert("RWTexture1D", Keyword_RWTexture1D);
		tst->Insert("RWTexture1DArray", Keyword_RWTexture1DArray);
		tst->Insert("RWTexture2D", Keyword_RWTexture2D);
		tst->Insert("RWTexture2DArray", Keyword_RWTexture2DArray);
		tst->Insert("RWTexture3D", Keyword_RWTexture3D);
		tst->Insert("sample", Keyword_Sample);
		tst->Insert("sampler", Keyword_Sampler);
		tst->Insert("SamplerState", Keyword_SamplerState);
		tst->Insert("SamplerComparisonState", Keyword_SamplerComparisonState);
		tst->Insert("shared", Keyword_Shared);
		tst->Insert("snorm", Keyword_Snorm);
		tst->Insert("stateblock", Keyword_Stateblock);
		tst->Insert("stateblock_state", Keyword_StateblockState);
		tst->Insert("static", Keyword_Static);
		tst->Insert("string", Keyword_String);
		tst->Insert("struct", Keyword_Struct);
		tst->Insert("switch", Keyword_Switch);
		tst->Insert("StructuredBuffer", Keyword_StructuredBuffer);
		tst->Insert("tbuffer", Keyword_Tbuffer);
		tst->Insert("technique", Keyword_Technique);
		tst->Insert("technique10", Keyword_Technique10);
		tst->Insert("technique11", Keyword_Technique11);
		tst->Insert("texture", Keyword_Texture);
		tst->Insert("Texture1D", Keyword_Texture1D);
		tst->Insert("Texture1DArray", Keyword_Texture1DArray);
		tst->Insert("Texture2D", Keyword_Texture2D);
		tst->Insert("Texture2DArray", Keyword_Texture2DArray);
		tst->Insert("Texture2DMS", Keyword_Texture2DMS);
		tst->Insert("Texture2DMSArray", Keyword_Texture2DMSArray);
		tst->Insert("Texture3D", Keyword_Texture3D);
		tst->Insert("TextureCube", Keyword_TextureCube);
		tst->Insert("TextureCubeArray", Keyword_TextureCubeArray);
		tst->Insert("true", Keyword_True);
		tst->Insert("typedef", Keyword_Typedef);
		tst->Insert("triangle", Keyword_Triangle);
		tst->Insert("triangleadj", Keyword_Triangleadj);
		tst->Insert("TriangleStream", Keyword_TriangleStream);
		tst->Insert("uint", Keyword_Uint);
		tst->Insert("uniform", Keyword_Uniform);
		tst->Insert("unorm", Keyword_Unorm);
		tst->Insert("unsigned", Keyword_Unsigned);
		tst->Insert("vector", Keyword_Vector);
		tst->Insert("vertexfragment", Keyword_Vertexfragment);
		tst->Insert("VertexShader", Keyword_VertexShader);
		tst->Insert("void", Keyword_Void);
		tst->Insert("volatile", Keyword_Volatile);
		tst->Insert("while", Keyword_While);
		tst->Insert("operator", Keyword_Operator);
		tst->Insert("implicit", Keyword_Implicit);
		tst->Insert("explicit", Keyword_Explicit);
		tst->Insert("using", Keyword_Using);
		tst->Insert("private", Keyword_Private);
		tst->Insert("protected", Keyword_Protected);
		tst->Insert("internal", Keyword_Internal);
		tst->Insert("public", Keyword_Public);
		tst->Insert("#define", Keyword_PrepDefine);
		tst->Insert("#if", Keyword_PrepIf);
		tst->Insert("#elif", Keyword_PrepElif);
		tst->Insert("#else", Keyword_PrepElse);
		tst->Insert("#endif", Keyword_PrepEndif);
		tst->Insert("#include", Keyword_PrepInclude);
		tst->Insert("#error", Keyword_PrepError);
		tst->Insert("#pragma", Keyword_PrepPragma);
	}
}
#endif