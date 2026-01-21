#ifndef KEYWORD_HPP
#define KEYWORD_HPP

#include "pch/std.hpp"
#include "utils/radix_tree.hpp"

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
		Keyword_This,
		Keyword_New,
		Keyword_MNew,
		keyword_MFree,
		Keyword_PrepDefine,
		Keyword_PrepIf,
		Keyword_PrepElif,
		Keyword_PrepElse,
		Keyword_PrepEndif,
		Keyword_PrepIfdef,
		Keyword_PrepIfndef,
		Keyword_PrepInclude,
		Keyword_PrepError,
		Keyword_PrepWarning,
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
		case Keyword_This: return "this";
		case Keyword_New: return "new";
		case Keyword_MNew: return "mnew";
		case keyword_MFree: return "mfree";
		case Keyword_PrepDefine: return "#define";
		case Keyword_PrepIf: return "#if";
		case Keyword_PrepElif: return "#elif";
		case Keyword_PrepElse: return "#else";
		case Keyword_PrepEndif: return "#endif";
		case Keyword_PrepIfdef: return "#ifdef";
		case Keyword_PrepIfndef: return "#ifndef";
		case Keyword_PrepInclude: return "#include";
		case Keyword_PrepError: return "#error";
		case Keyword_PrepWarning: return "#warning";
		case Keyword_PrepPragma: return "#pragma";
		default: return "Unknown";
		}
	}

	static void BuildKeywordRadix(RadixTree<int>& t)
	{
		t.Insert("AppendStructuredBuffer", Keyword_AppendStructuredBuffer);
		t.Insert("asm", Keyword_Asm);
		t.Insert("asm_fragment", Keyword_AsmFragment);
		t.Insert("BlendState", Keyword_BlendState);
		t.Insert("bool", Keyword_Bool);
		t.Insert("break", Keyword_Break);
		t.Insert("Buffer", Keyword_Buffer);
		t.Insert("ByteAddressBuffer", Keyword_ByteAddressBuffer);
		t.Insert("case", Keyword_Case);
		t.Insert("cbuffer", Keyword_Cbuffer);
		t.Insert("centroid", Keyword_Centroid);
		t.Insert("class", Keyword_Class);
		t.Insert("column_major", Keyword_ColumnMajor);
		t.Insert("compile", Keyword_Compile);
		t.Insert("compile_fragment", Keyword_CompileFragment);
		t.Insert("CompileShader", Keyword_CompileShader);
		t.Insert("const", Keyword_Const);
		t.Insert("continue", Keyword_Continue);
		t.Insert("ComputeShader", Keyword_ComputeShader);
		t.Insert("ConsumeStructuredBuffer", Keyword_ConsumeStructuredBuffer);
		t.Insert("default", Keyword_Default);
		t.Insert("DepthStencilState", Keyword_DepthStencilState);
		t.Insert("DepthStencilView", Keyword_DepthStencilView);
		t.Insert("discard", Keyword_Discard);
		t.Insert("do", Keyword_Do);
		t.Insert("double", Keyword_Double);
		t.Insert("DomainShader", Keyword_DomainShader);
		t.Insert("dword", Keyword_Dword);
		t.Insert("else", Keyword_Else);
		t.Insert("export", Keyword_Export);
		t.Insert("extern", Keyword_Extern);
		t.Insert("false", Keyword_False);
		t.Insert("float", Keyword_Float);
		t.Insert("for", Keyword_For);
		t.Insert("fxgroup", Keyword_Fxgroup);
		t.Insert("GeometryShader", Keyword_GeometryShader);
		t.Insert("groupshared", Keyword_Groupshared);
		t.Insert("half", Keyword_Half);
		t.Insert("Hullshader", Keyword_Hullshader);
		t.Insert("if", Keyword_If);
		t.Insert("in", Keyword_In);
		t.Insert("inline", Keyword_Inline);
		t.Insert("inout", Keyword_Inout);
		t.Insert("InputPatch", Keyword_InputPatch);
		t.Insert("int", Keyword_Int);
		t.Insert("interface", Keyword_Interface);
		t.Insert("line", Keyword_Line);
		t.Insert("lineadj", Keyword_Lineadj);
		t.Insert("linear", Keyword_Linear);
		t.Insert("LineStream", Keyword_LineStream);
		t.Insert("matrix", Keyword_Matrix);
		t.Insert("min16float", Keyword_Min16float);
		t.Insert("min10float", Keyword_Min10float);
		t.Insert("min16int", Keyword_Min16int);
		t.Insert("min12int", Keyword_Min12int);
		t.Insert("min16uint", Keyword_Min16uint);
		t.Insert("namespace", Keyword_Namespace);
		t.Insert("nointerpolation", Keyword_NoInterpolation);
		t.Insert("noperspective", Keyword_Noperspective);
		t.Insert("NULL", Keyword_Null);
		t.Insert("out", Keyword_Out);
		t.Insert("OutputPatch", Keyword_OutputPatch);
		t.Insert("packoffset", Keyword_Packoffset);
		t.Insert("pass", Keyword_Pass);
		t.Insert("pixelfragment", Keyword_Pixelfragment);
		t.Insert("PixelShader", Keyword_PixelShader);
		t.Insert("point", Keyword_Point);
		t.Insert("PointStream", Keyword_PointStream);
		t.Insert("precise", Keyword_Precise);
		t.Insert("RasterizerState", Keyword_RasterizerState);
		t.Insert("RenderTargetView", Keyword_RenderTargetView);
		t.Insert("return", Keyword_Return);
		t.Insert("register", Keyword_Register);
		t.Insert("row_major", Keyword_RowMajor);
		t.Insert("RWBuffer", Keyword_RWBuffer);
		t.Insert("RWByteAddressBuffer", Keyword_RWByteAddressBuffer);
		t.Insert("RWStructuredBuffer", Keyword_RWStructuredBuffer);
		t.Insert("RWTexture1D", Keyword_RWTexture1D);
		t.Insert("RWTexture1DArray", Keyword_RWTexture1DArray);
		t.Insert("RWTexture2D", Keyword_RWTexture2D);
		t.Insert("RWTexture2DArray", Keyword_RWTexture2DArray);
		t.Insert("RWTexture3D", Keyword_RWTexture3D);
		t.Insert("sample", Keyword_Sample);
		t.Insert("sampler", Keyword_Sampler);
		t.Insert("SamplerState", Keyword_SamplerState);
		t.Insert("SamplerComparisonState", Keyword_SamplerComparisonState);
		t.Insert("shared", Keyword_Shared);
		t.Insert("snorm", Keyword_Snorm);
		t.Insert("stateblock", Keyword_Stateblock);
		t.Insert("stateblock_state", Keyword_StateblockState);
		t.Insert("static", Keyword_Static);
		t.Insert("string", Keyword_String);
		t.Insert("struct", Keyword_Struct);
		t.Insert("switch", Keyword_Switch);
		t.Insert("StructuredBuffer", Keyword_StructuredBuffer);
		t.Insert("tbuffer", Keyword_Tbuffer);
		t.Insert("technique", Keyword_Technique);
		t.Insert("technique10", Keyword_Technique10);
		t.Insert("technique11", Keyword_Technique11);
		t.Insert("texture", Keyword_Texture);
		t.Insert("Texture1D", Keyword_Texture1D);
		t.Insert("Texture1DArray", Keyword_Texture1DArray);
		t.Insert("Texture2D", Keyword_Texture2D);
		t.Insert("Texture2DArray", Keyword_Texture2DArray);
		t.Insert("Texture2DMS", Keyword_Texture2DMS);
		t.Insert("Texture2DMSArray", Keyword_Texture2DMSArray);
		t.Insert("Texture3D", Keyword_Texture3D);
		t.Insert("TextureCube", Keyword_TextureCube);
		t.Insert("TextureCubeArray", Keyword_TextureCubeArray);
		t.Insert("true", Keyword_True);
		t.Insert("typedef", Keyword_Typedef);
		t.Insert("triangle", Keyword_Triangle);
		t.Insert("triangleadj", Keyword_Triangleadj);
		t.Insert("TriangleStream", Keyword_TriangleStream);
		t.Insert("uint", Keyword_Uint);
		t.Insert("uniform", Keyword_Uniform);
		t.Insert("unorm", Keyword_Unorm);
		t.Insert("unsigned", Keyword_Unsigned);
		t.Insert("vector", Keyword_Vector);
		t.Insert("vertexfragment", Keyword_Vertexfragment);
		t.Insert("VertexShader", Keyword_VertexShader);
		t.Insert("void", Keyword_Void);
		t.Insert("volatile", Keyword_Volatile);
		t.Insert("while", Keyword_While);
		t.Insert("operator", Keyword_Operator);
		t.Insert("implicit", Keyword_Implicit);
		t.Insert("explicit", Keyword_Explicit);
		t.Insert("using", Keyword_Using);
		t.Insert("private", Keyword_Private);
		t.Insert("protected", Keyword_Protected);
		t.Insert("internal", Keyword_Internal);
		t.Insert("public", Keyword_Public);
		//t.Insert("this", Keyword_This);
		t.Insert("new", Keyword_New);
		t.Insert("mnew", Keyword_MNew);
		t.Insert("mfree", keyword_MFree);
		t.Insert("#define", Keyword_PrepDefine);
		t.Insert("#if", Keyword_PrepIf);
		t.Insert("#elif", Keyword_PrepElif);
		t.Insert("#else", Keyword_PrepElse);
		t.Insert("#endif", Keyword_PrepEndif);
		t.Insert("#ifdef", Keyword_PrepIfdef);
		t.Insert("#ifndef", Keyword_PrepIfndef);
		t.Insert("#include", Keyword_PrepInclude);
		t.Insert("#error", Keyword_PrepError);
		t.Insert("#warning", Keyword_PrepWarning);
		t.Insert("#pragma", Keyword_PrepPragma);
	}
}
#endif