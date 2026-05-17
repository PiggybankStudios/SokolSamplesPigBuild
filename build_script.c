/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\16\2026
Description: 
	** None
*/

#define PIG_BUILD_PRINT_SYS_CMDS 1
#include "pig_build.h"

#define DEBUG_BUILD 1

#if DEBUG_BUILD
#define IF_DEBUG(...)    __VA_ARGS__
#define IF_RELEASE(...)  //nothing
#else
#define IF_DEBUG(...)    //nothing
#define IF_RELEASE(...)  __VA_ARGS__
#endif

void DownloadSokolIfNeeded();
void CrossCompileShadersIfNeeded();

int main(int argc, char* argv[])
{
	RecompileIfNeeded(nullptr);
	Str pigBuildFolder = StrLit(PIG_BUILD_ROOT);
	IF_WINDOWS(bool isMsvcInitialized = WasMsvcDevBatchRun());
	
	DownloadSokolIfNeeded();
	CrossCompileShadersIfNeeded();
	
	CliArgs commonArgs = EMPTY;
	
	// +==============================+
	// |     MSVC Compiler Flags      |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_NO_LOGO);
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, ".");
	// AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/../sokol");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/libs");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_MSVC_CL, CL_DEBUG_INFO));
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	
	// +==============================+
	// |     Clang Compiler Flags     |
	// +==============================+
	AddTaggedArg(&commonArgs, T_CLANG, CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_CLANG, CLANG_DEBUG_INFO_DEFAULT));
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_LANG_VERSION, "gnu2x");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, ".");
	// AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/../sokol");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/libs");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OUTPUT_FILE, "sokol_triangle");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX "==false", CLI_QUOTED_ARG, "[ROOT]/main.c");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX "==true", CLI_QUOTED_ARG, "main.m");
	
	// +==============================+
	// |     Linux Compiler Flags     |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "m");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xcursor");
	
	// +==============================+
	// |      OSX Compiler Flags      |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_DEFINE, "TARGET_IS_OSX=1");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "CoreFoundation");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "MetalKit");
	
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_WINDOWS, CL_DEFINE,    "SOKOL_D3D11"); //SOKOL_D3D11, SOKOL_GLCORE, SOKOL_WGPU, SOKOL_VULKAN
	AddTaggedArgNt(&commonArgs, T_CLANG   T_OSX,     CLANG_DEFINE, "SOKOL_METAL"); //SOKOL_METAL, SOKOL_GLCORE
	AddTaggedArgNt(&commonArgs, T_CLANG   T_LINUX,   CLANG_DEFINE, "SOKOL_GLCORE"); //SOKOL_GLCORE, SOKOL_GLES3, SOKOL_WGPU, SOKOL_VULKAN
	
	// +==============================+
	// |         Linker Flags         |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_LINK);
	AddTaggedArg(&commonArgs, T_MSVC_CL, LINK_DISABLE_INCREMENTAL);
	
	// +--------------------------------------------------------------+
	// |                Compile sokol Implementations                 |
	// +--------------------------------------------------------------+
	#if 0
	StrArray sokolHeaderFiles = EMPTY;
	AddStrLit(&sokolHeaderFiles, "[ROOT]/../sokol/sokol_app.h");
	AddStrLit(&sokolHeaderFiles, "[ROOT]/../sokol/sokol_gfx.h");
	AddStrLit(&sokolHeaderFiles, "[ROOT]/../sokol/sokol_log.h");
	AddStrLit(&sokolHeaderFiles, "[ROOT]/../sokol/sokol_glue.h");
	
	StrArray sokolObjFiles = EMPTY;
	for (u64 sIndex = 0; sIndex < sokolHeaderFiles.length; sIndex++)
	{
		Str headerPath = sokolHeaderFiles.strings[sIndex];
		Str headerFileName = GetFileNamePart(headerPath, true);
		Str headerFileNameNoExt = GetFileNamePart(headerPath, false);
		Str srcPath = JoinStrings2(headerFileNameNoExt, StrLit(".c"));
		Str objPath = JoinStrings2(headerFileNameNoExt, BUILDING_ON_WINDOWS ? StrLit(".obj") : StrLit(".o"));
		Str pdbPath = JoinStrings2(headerFileNameNoExt, StrLit(".pdb"));
		
		if (!DoesFileExist(objPath))
		{
			CreateAndWriteFile(srcPath, FormatStr("\n#define SOKOL_IMPL\n#include \"%.*s\"\n", StrPrint(headerFileName)), true);
			
			CliArgs args = EMPTY;
			AddArgStr(&args, CLI_QUOTED_ARG, srcPath);
			AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
			AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
			AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, objPath);
			AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, objPath);
			IF_WINDOWS(AddTaggedArgStr(&args, T_MSVC_CL, CL_PDB_FILE, pdbPath));
			AddArgList(&args, &commonArgs);
			
			#if BUILDING_ON_WINDOWS
			{
				PrintLine("[Building %.*s for WINDOWS...]", StrPrint(objPath));
				InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
				RunCliProgramAndExitOnFailure(StrLit("cl"), T_MSVC_CL T_WINDOWS, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(headerFileName), StrPrint(objPath)));
				AssertFileExist(objPath, true);
				PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(objPath));
			}
			#else
			#error The current platform is not supported yet!
			#endif
		}
		
		AddStr(&sokolObjFiles, objPath);
	}
	#endif
	Str sokolObjFile = BUILDING_ON_WINDOWS ? StrLit("sokol.obj") : StrLit("sokol.o");
	if (!DoesFileExist(sokolObjFile))
	{
		CliArgs args = EMPTY;
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/sokol/sokol.c");
		AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
		AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, sokolObjFile);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, sokolObjFile);
		AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "sokol.pdb");
		AddArgList(&args, &commonArgs);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(sokolObjFile));
			InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
			RunCliProgramAndExitOnFailure(StrLit("cl"), T_MSVC_CL T_WINDOWS, &args, FormatStr("Failed to compile sokol.c into %.*s", StrPrint(sokolObjFile)));
			AssertFileExist(sokolObjFile, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(sokolObjFile));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
	
	// +--------------------------------------------------------------+
	// |                       Compile Examples                       |
	// +--------------------------------------------------------------+
	StrArray examples = EMPTY;
	// AddStrLit(&examples, "[ROOT]/sapp/arraytex-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/basisu-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/blend-op-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/blend-playground-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/blend-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/bufferoffsets-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cgltf-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cimgui-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/clear-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/computeboids-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cube-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cubemap-jpeg-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cubemaprt-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/cursor-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/customresolve-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/debugtext-context-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/debugtext-layers-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/debugtext-printf-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/debugtext-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/debugtext-userfont-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/drawcallperf-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/drawex-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/droptest-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/dyntex-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/dyntex3d-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/events-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/fontstash-layers-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/fontstash-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/framebuffer-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/icon-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/ilbm-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imageblur-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-dock-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-highdpi-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-images-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-perf-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/imgui-usercallback-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/instancing-compute-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/instancing-pull-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/instancing-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/layerrender-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/letterbox-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/loadpng-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/mipmap-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/miprender-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/modplay-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/mrt-pixelformats-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/mrt-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/noentry-dll-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/noentry-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/noninterleaved-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/nuklear-images-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/nuklear-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/offscreen-msaa-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/offscreen-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/ozz-anim-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/ozz-skin-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/ozz-storagebuffer-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/pixelformats-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/plmpeg-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/primtypes-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/quad-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/restart-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/saudio-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sbufoffset-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sbuftex-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sdf-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sgl-context-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sgl-lines-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sgl-microui-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sgl-points-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/sgl-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shadows-depthtex-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shadows-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shapes-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shapes-transform-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shared-bindings-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/shdfeatures-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-contexts-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-inspector-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-layers-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-simple-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-skinsets-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/spine-switch-skinsets-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/tex3d-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/texcube-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/texview-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/triangle-bufferless-sapp.c");
	AddStrLit(&examples, "[ROOT]/sapp/triangle-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/uniformtypes-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/uvwrap-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/vertexindexbuffer-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/vertexpull-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/vertextexture-sapp.c");
	// AddStrLit(&examples, "[ROOT]/sapp/write-storageimage-sapp.c");
	
	for (u64 eIndex = 0; eIndex < examples.length; eIndex++)
	{
		Str exampleSrcPath = examples.strings[eIndex];
		Str exampleFileName = GetFileNamePart(exampleSrcPath, true);
		#if BUILDING_ON_WINDOWS
		Str exampleBinName = JoinStrings2(GetFileNamePart(exampleSrcPath, false), StrLit(".exe"));
		#else
		Str exampleBinName = GetFileNamePart(exampleSrcPath, false);
		#endif
		StrReplaceChars(exampleBinName, '-', '_');
		
		CliArgs args = EMPTY;
		AddArgStr(&args, CLI_QUOTED_ARG, exampleSrcPath);
		AddArgStr(&args, CLI_QUOTED_ARG, sokolObjFile);
		// for (u64 oIndex = 0; oIndex < sokolObjFiles.length; oIndex++) { AddArgStr(&args, CLI_QUOTED_ARG, sokolObjFiles.strings[oIndex]); }
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, exampleBinName);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, exampleBinName);
		IF_WINDOWS(AddTaggedArgStr(&args, T_MSVC_CL, CL_PDB_FILE, JoinStrings2(GetFileNamePart(exampleSrcPath, false), StrLit(".pdb"))));
		AddArgList(&args, &commonArgs);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(exampleBinName));
			InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
			RunCliProgramAndExitOnFailure(StrLit("cl"), T_MSVC_CL T_WINDOWS, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(exampleFileName), StrPrint(exampleBinName)));
			AssertFileExist(exampleBinName, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(exampleBinName));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
}

#if BUILDING_ON_WINDOWS
#define SHDC_BIN_PATH "..\\sokol_tools\\bin\\win32\\sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define SHDC_BIN_PATH "../sokol_tools/bin/linux/sokol-shdc"
#elif BUILDING_ON_OSX_ARM
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx_arm64/sokol-shdc"
#elif BUILDING_ON_OSX_INTEL
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx/sokol-shdc"
#else
#error build_script.c SHDC_BIN_PATH needs to be updated to support the current platform!
#endif

void DownloadSokolIfNeeded()
{
	// https://github.com/floooh/sokol/commit/453c71214fbb55d782683d20ea7e6c07314e3e9b
	// Commit 453c712 from May 14th 2026 - "sokol_framebuffer.h: fix some comment typos"
	Str sokolUrl = StrLit("https://github.com/floooh/sokol/archive/453c71214fbb55d782683d20ea7e6c07314e3e9b.zip");
	Str sokolZipPath = StrLit("sokol_453c712.zip");
	Str sokolZipRootFolder = StrLit("sokol-453c71214fbb55d782683d20ea7e6c07314e3e9b");
	Str sokolFolderPath = StrLit("../sokol");
	if (!DoesFileExist(sokolZipPath) || !DoesFolderExist(sokolFolderPath))
	{
		PrintLine("Downloading Sokol from \"%.*s\"", StrPrint(sokolUrl));
		// if (DoesFolderExist(sokolFolderPath)) { MyRemoveDirectory(sokolFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolUrl,
			sokolZipPath,
			1573414, 0x5707750981F0E26C,
			sokolFolderPath,
			sokolZipRootFolder
		);
	}
	
	// https://github.com/floooh/sokol-tools-bin/commit/1a9a4e54090fec42c5d13169b638f09f25474953
	// Commit 1a9a4e5 from April 26th 2026 - "updated (88)"
	Str sokolToolsUrl = StrLit("https://github.com/floooh/sokol-tools-bin/archive/1a9a4e54090fec42c5d13169b638f09f25474953.zip");
	Str sokolToolsZipPath = StrLit("sokol_tools_1a9a4e5.zip");
	Str sokolToolsZipRootFolder = StrLit("sokol-tools-bin-1a9a4e54090fec42c5d13169b638f09f25474953");
	Str sokolToolsFolderPath = StrLit("../sokol_tools");
	if (!DoesFileExist(sokolToolsZipPath) || !DoesFolderExist(sokolToolsFolderPath))
	{
		PrintLine("Downloading Sokol Tools from \"%.*s\"", StrPrint(sokolToolsUrl));
		// if (DoesFolderExist(sokolToolsFolderPath)) { MyRemoveDirectory(sokolToolsFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolToolsUrl,
			sokolToolsZipPath,
			18032476, 0xDF94D1D90D33715F,
			sokolToolsFolderPath,
			sokolToolsZipRootFolder
		);
		
		// Mark sokol-shdc as executable on Linux and OSX
		#if BUILDING_ON_OSX || BUILDING_ON_LINUX
		CliArgs chmodArgs = EMPTY;
		AddArg(&chmodArgs, "+x");
		AddArgNt(&chmodArgs, CLI_QUOTED_ARG, SHDC_BIN_PATH);
		RunCliProgramAndExitOnFailure(StrLit("chmod"), "", &chmodArgs, StrLit("Failed to make sokol-shdc executable!"));
		#endif
	}
}

void CrossCompileShadersIfNeeded()
{
	FileIter sappFolderIter = StartFileIter(StrLit("../sapp"));
	bool isFolder = false;
	Str filePath = Str_Empty_Const;
	while (StepFileIter(&sappFolderIter, &filePath, &isFolder))
	{
		if (!isFolder && StrAnyCaseEquals(GetFileExtPart(filePath, false), StrLit(".glsl")))
		{
			FixPathSlashes(filePath, '/');
			Str shaderSrcPath = filePath;
			Str shaderFileName = GetFileNamePart(shaderSrcPath, true);
			Str shaderHeaderPath = GetFileNamePart(shaderSrcPath, true);
			shaderHeaderPath = JoinStrings2(shaderHeaderPath, StrLit(".h"));
			
			if (!DoesFileExist(shaderHeaderPath))
			{
				PrintLine("Cross-compiling %.*s using sokol-shdc...", StrPrint(shaderFileName));
				
				CliArgs shdcArgs = EMPTY;
				AddArgNt(&shdcArgs, SHDC_FORMAT, "sokol_impl");
				AddArgNt(&shdcArgs, SHDC_ERROR_FORMAT, "msvc");
				// AddArg(&shdcArgs, SHDC_REFLECTION);
				AddArgNt(&shdcArgs, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:glsl300es:metal_macos");
				AddArgStr(&shdcArgs, SHDC_INPUT, shaderSrcPath);
				AddArgStr(&shdcArgs, SHDC_OUTPUT, shaderHeaderPath);
				
				int exitCode = RunCliProgram(StrLit(SHDC_BIN_PATH), "", &shdcArgs);
				if (exitCode != 0)
				{
					PrintLine("Failed to cross-compile %.*s using sokol-shdc! Exit code: %d", StrPrint(shaderFileName), exitCode);
				}
				else
				{
					AssertFileExist(shaderHeaderPath, true);
					PrintLine("Successfully cross-compiled %.*s!", StrPrint(shaderFileName));
				}
			}
		}
	}
}
